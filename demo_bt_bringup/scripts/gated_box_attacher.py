#!/usr/bin/env python3

import math
import random

import rclpy
from rclpy.duration import Duration
from rclpy.node import Node
from rviz_2d_overlay_msgs.msg import OverlayText
from std_msgs.msg import ColorRGBA
from std_msgs.msg import Empty
from std_msgs.msg import String
from std_srvs.srv import Trigger
from tf2_ros import Buffer
from tf2_ros import TransformException
from tf2_ros import TransformListener

SUCTION = "suction_cup"
MAGNET = "magnet"
ACTUATORS = (SUCTION, MAGNET)

BOX_NAMES = tuple(f"box_{index}" for index in range(1, 6))


class SuctionCupController(Node):
    def __init__(self):
        super().__init__("suction_cup_controller")
        self.declare_parameter("parent_frame", "wrist_3_link")
        self.declare_parameter("max_distance", 0.15)
        self.declare_parameter("initial_detach_period", 0.5)
        self.declare_parameter("initial_detach_timeout", 30.0)

        self.declare_parameter("random_seed", -1)

        self._parent_frame = self.get_parameter("parent_frame").value
        seed = int(self.get_parameter("random_seed").value)
        rng = random.Random(seed if seed >= 0 else None)
        # Which actuator can pick up each box, randomised once per launch.
        self._box_actuator = {box: rng.choice(ACTUATORS) for box in BOX_NAMES}
        self.get_logger().info(f"Box actuator assignment: {self._box_actuator}")
        self._max_distance = float(self.get_parameter("max_distance").value)
        if self._max_distance <= 0.0:
            raise ValueError("max_distance must be greater than zero")

        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, self)
        self._attach_publishers = {
            box_name: self.create_publisher(Empty, f"/{box_name}/attach_gated", 10)
            for box_name in BOX_NAMES
        }
        self._detach_publishers = {
            box_name: self.create_publisher(Empty, f"/{box_name}/detach", 10)
            for box_name in self._attach_publishers
        }
        self._joint_states = {}
        self._joint_state_subscriptions = [
            self.create_subscription(
                String,
                f"/{box_name}/joint_state",
                lambda msg, name=box_name: self._on_joint_state(name, msg),
                10,
            )
            for box_name in self._attach_publishers
        ]
        self._initial_detach_deadline = self.get_clock().now() + Duration(
            seconds=float(self.get_parameter("initial_detach_timeout").value)
        )
        self._initial_detach_timer = self.create_timer(
            float(self.get_parameter("initial_detach_period").value),
            self._initial_detach,
        )
        # Each actuator is simulated without feedback: it is either on or off,
        # and remembers which box (if any) it attached.
        self._active = {actuator: False for actuator in ACTUATORS}
        self._held_box = {actuator: None for actuator in ACTUATORS}
        self._overlay_publishers = {
            actuator: self.create_publisher(OverlayText, f"/{actuator}/overlay", 1)
            for actuator in ACTUATORS
        }
        self.create_timer(0.5, self._publish_overlays)
        for actuator in ACTUATORS:
            self.create_service(
                Trigger,
                f"/{actuator}/attach",
                lambda _request, response, name=actuator: self._attach(name, response),
            )
            self.create_service(
                Trigger,
                f"/{actuator}/detach",
                lambda _request, response, name=actuator: self._detach(name, response),
            )

    def _publish_overlays(self):
        for index, actuator in enumerate(ACTUATORS):
            active = self._active[actuator]
            state = "ON" if active else "OFF"
            msg = OverlayText()
            msg.action = OverlayText.ADD
            msg.width = 340
            msg.height = 40
            msg.horizontal_distance = 10
            msg.vertical_distance = 10 + index * 50
            msg.horizontal_alignment = OverlayText.RIGHT
            msg.vertical_alignment = OverlayText.TOP
            msg.bg_color = ColorRGBA(r=0.0, g=0.0, b=0.0, a=0.6)
            msg.fg_color = (
                ColorRGBA(r=0.2, g=1.0, b=0.2, a=1.0)
                if active
                else ColorRGBA(r=1.0, g=0.3, b=0.3, a=1.0)
            )
            msg.line_width = 2
            msg.text_size = 14.0
            msg.font = "DejaVu Sans Mono"
            msg.text = f"{actuator.replace('_', ' ').title()}: {state}"
            self._overlay_publishers[actuator].publish(msg)

    def _on_joint_state(self, box_name, msg):
        if self._joint_states.get(box_name) != msg.data:
            self.get_logger().info(f"{box_name} joint is {msg.data}")
        self._joint_states[box_name] = msg.data

    def _initial_detach(self):
        # The plugin only reports state on change, so boxes with no report yet
        # are treated as still attached and detached until "detached" is seen.
        pending = [
            box_name
            for box_name in self._detach_publishers
            if self._joint_states.get(box_name) != "detached"
        ]
        if not pending:
            self.get_logger().info("All boxes detached")
            self._initial_detach_timer.cancel()
            return
        if self.get_clock().now() > self._initial_detach_deadline:
            self.get_logger().warning(
                f"Initial detach timed out; no detached feedback for {pending}"
            )
            self._initial_detach_timer.cancel()
            return
        for box_name in pending:
            self._detach_publishers[box_name].publish(Empty())

    def _attach(self, actuator, response):
        other = next(name for name in ACTUATORS if name != actuator)
        if self._active[other]:
            response.success = False
            response.message = f"Cannot switch on {actuator}: {other} is already on"
            self.get_logger().warning(response.message)
            return response

        self._active[actuator] = True
        if self._held_box[actuator] is not None:
            response.success = True
            response.message = (
                f"{actuator} is on, already holding {self._held_box[actuator]}"
            )
            return response

        box_name, distance = self._find_nearby_box(actuator)
        if box_name is None:
            response.success = True
            response.message = f"{actuator} on; no compatible box in range"
        else:
            self._attach_publishers[box_name].publish(Empty())
            self._held_box[actuator] = box_name
            response.success = True
            response.message = (
                f"{actuator} on; requested attachment of {box_name} at "
                f"distance {distance:.3f} m"
            )
        self.get_logger().info(response.message)
        return response

    def _find_nearby_box(self, actuator):
        try:
            parent = self._tf_buffer.lookup_transform(
                "world", self._parent_frame, rclpy.time.Time()
            )
        except TransformException as exception:
            self.get_logger().warning(
                f"TF for {self._parent_frame} is unavailable ({exception})"
            )
            return None, None

        held = set(self._held_box.values())
        nearest_box = None
        nearest_distance = None
        for box_name in self._attach_publishers:
            if self._box_actuator.get(box_name) != actuator or box_name in held:
                continue
            try:
                box = self._tf_buffer.lookup_transform(
                    "world", box_name, rclpy.time.Time()
                )
            except TransformException as exception:
                self.get_logger().warning(
                    f"Skipping {box_name}: TF unavailable ({exception})"
                )
                continue

            distance = self._distance(parent, box)
            if nearest_distance is None or distance < nearest_distance:
                nearest_box = box_name
                nearest_distance = distance

        if nearest_box is None or nearest_distance > self._max_distance:
            return None, None
        return nearest_box, nearest_distance

    def _detach(self, actuator, response):
        box_name = self._held_box[actuator]
        if box_name is not None:
            self._detach_publishers[box_name].publish(Empty())
        self._held_box[actuator] = None
        self._active[actuator] = False

        response.success = True
        response.message = f"{actuator} off" + (
            f"; requested detachment of {box_name}" if box_name else ""
        )
        self.get_logger().info(response.message)
        return response

    @staticmethod
    def _distance(first_transform, second_transform):
        first = first_transform.transform.translation
        second = second_transform.transform.translation
        return math.sqrt(
            (first.x - second.x) ** 2
            + (first.y - second.y) ** 2
            + (first.z - second.z) ** 2
        )


def main():
    rclpy.init()
    node = SuctionCupController()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
