#!/usr/bin/env python3
"""Virtual sensor: true while any box frame sits at z=detection_z in the sensor frame (within tolerances)."""

import rclpy
from rclpy.node import Node
from rviz_2d_overlay_msgs.msg import OverlayText
from std_msgs.msg import Bool, ColorRGBA, String
from tf2_ros import Buffer
from tf2_ros import TransformException
from tf2_ros import TransformListener


class BoxProximitySensor(Node):
    def __init__(self):
        super().__init__("box_proximity_sensor")
        self.declare_parameter("sensor_frame", "sensor_1")
        self.declare_parameter("box_count", 6)
        self.declare_parameter("box_colors", ["blue", "orange", "blue", "blue", "orange", "blue"])
        self.declare_parameter("detection_z", 0.1)
        self.declare_parameter("z_tolerance", 0.02)
        self.declare_parameter("lateral_tolerance", 0.05)
        self.declare_parameter("publish_rate", 20.0)
        self.declare_parameter("topic", "/sensor_1/detected")
        self.declare_parameter("color_topic", "/sensor_1/color")

        self._sensor_frame = self.get_parameter("sensor_frame").value
        self._box_frames = [
            f"box_{index}"
            for index in range(1, int(self.get_parameter("box_count").value) + 1)
        ]
        self._box_colors = self.get_parameter("box_colors").value
        if len(self._box_colors) != len(self._box_frames):
            raise ValueError("box_colors must have box_count elements")
        self._detection_z = float(self.get_parameter("detection_z").value)
        self._z_tolerance = float(self.get_parameter("z_tolerance").value)
        self._lateral_tolerance = float(self.get_parameter("lateral_tolerance").value)
        if self._z_tolerance <= 0.0 or self._lateral_tolerance <= 0.0:
            raise ValueError("z_tolerance and lateral_tolerance must be greater than zero")

        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, self)
        self._publisher = self.create_publisher(
            Bool, self.get_parameter("topic").value, 10
        )
        self._color_publisher = self.create_publisher(
            String, self.get_parameter("color_topic").value, 10
        )
        self._presence_overlay = self.create_publisher(
            OverlayText, "/sensor_1/presence_overlay", 1
        )
        self._color_overlay = self.create_publisher(
            OverlayText, "/sensor_1/color_overlay", 1
        )
        self._last_state = None
        self._timer = self.create_timer(
            1.0 / float(self.get_parameter("publish_rate").value), self._update
        )

    def _update(self):
        detected = False
        color = ""
        for idx, box_frame in enumerate(self._box_frames):
            try:
                # Box position expressed in the sensor frame.
                box = self._tf_buffer.lookup_transform(
                    self._sensor_frame, box_frame, rclpy.time.Time()
                ).transform.translation
            except TransformException:
                continue
            if (
                abs(box.z - self._detection_z) <= self._z_tolerance
                and abs(box.x) <= self._lateral_tolerance
                and abs(box.y) <= self._lateral_tolerance
            ):
                detected = True
                try:
                    color = self._box_colors[idx]  # type: ignore
                except:
                    self.get_logger().info(f"No color information for box {idx}")
                    pass
                break

        if detected != self._last_state:
            self.get_logger().info(f"{self._sensor_frame} detected: {detected} ({color})")
            self._last_state = detected
        self._publisher.publish(Bool(data=detected))
        self._color_publisher.publish(String(data=color))
        self._publish_overlay(
            self._presence_overlay,
            110,
            f"Sensor presence: {'DETECTED' if detected else 'none'}",
            ColorRGBA(r=0.2, g=1.0, b=0.2, a=1.0)
            if detected
            else ColorRGBA(r=1.0, g=0.3, b=0.3, a=1.0),
        )
        self._publish_overlay(
            self._color_overlay,
            160,
            f"Sensor colour: {color if detected else '-'}",
            self._COLORS.get(
                color if detected else "", ColorRGBA(r=0.8, g=0.8, b=0.8, a=1.0)
            ),
        )

    _COLORS = {
        "blue": ColorRGBA(r=0.3, g=0.6, b=1.0, a=1.0),
        "orange": ColorRGBA(r=1.0, g=0.6, b=0.1, a=1.0),
    }

    @staticmethod
    def _publish_overlay(publisher, vertical_distance, text, fg_color):
        msg = OverlayText()
        msg.action = OverlayText.ADD
        msg.width = 340
        msg.height = 40
        msg.horizontal_distance = 10
        msg.vertical_distance = vertical_distance
        msg.horizontal_alignment = OverlayText.RIGHT
        msg.vertical_alignment = OverlayText.TOP
        msg.bg_color = ColorRGBA(r=0.0, g=0.0, b=0.0, a=0.6)
        msg.fg_color = fg_color
        msg.line_width = 2
        msg.text_size = 14.0
        msg.font = "DejaVu Sans Mono"
        msg.text = text
        publisher.publish(msg)


def main():
    rclpy.init()
    node = BoxProximitySensor()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
