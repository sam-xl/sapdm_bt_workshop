#!/usr/bin/env python3
"""Publishes the current conveyor belt speed as an RViz overlay text."""

import rclpy
from conveyorbelt_msgs.msg import ConveyorBeltState
from rclpy.node import Node
from rviz_2d_overlay_msgs.msg import OverlayText
from std_msgs.msg import ColorRGBA


class ConveyorSpeedOverlay(Node):
    def __init__(self):
        super().__init__("conveyor_speed_overlay")
        # Must match <max_velocity> of the conveyor plugin in the model SDF.
        self.declare_parameter("max_velocity", 1.0)
        self.declare_parameter("state_topic", "/CONVEYORSTATE")
        self.declare_parameter("overlay_topic", "/conveyor/speed_overlay")
        self.declare_parameter("vertical_distance", 210)
        self.declare_parameter("publish_rate", 5.0)

        self._max_velocity = float(self.get_parameter("max_velocity").value)
        self._vertical_distance = int(self.get_parameter("vertical_distance").value)
        self._power = None
        self.create_subscription(
            ConveyorBeltState,
            self.get_parameter("state_topic").value,
            self._on_state,
            10,
        )
        self._publisher = self.create_publisher(
            OverlayText, self.get_parameter("overlay_topic").value, 1
        )
        self.create_timer(
            1.0 / float(self.get_parameter("publish_rate").value), self._publish
        )

    def _on_state(self, msg):
        self._power = msg.power

    def _publish(self):
        msg = OverlayText()
        msg.action = OverlayText.ADD
        msg.width = 340
        msg.height = 40
        msg.horizontal_distance = 10
        msg.vertical_distance = self._vertical_distance
        msg.horizontal_alignment = OverlayText.RIGHT
        msg.vertical_alignment = OverlayText.TOP
        msg.bg_color = ColorRGBA(r=0.0, g=0.0, b=0.0, a=0.6)
        msg.line_width = 2
        msg.text_size = 14.0
        msg.font = "DejaVu Sans Mono"
        if self._power is None:
            msg.fg_color = ColorRGBA(r=0.8, g=0.8, b=0.8, a=1.0)
            msg.text = "Conveyor speed: -"
        else:
            speed = self._max_velocity * self._power / 100.0
            running = self._power > 0.0
            msg.fg_color = (
                ColorRGBA(r=0.2, g=1.0, b=0.2, a=1.0)
                if running
                else ColorRGBA(r=1.0, g=0.3, b=0.3, a=1.0)
            )
            msg.text = f"Conveyor speed: {speed:.2f} m/s"
        self._publisher.publish(msg)


def main():
    rclpy.init()
    node = ConveyorSpeedOverlay()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
