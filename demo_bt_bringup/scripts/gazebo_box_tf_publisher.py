#!/usr/bin/env python3

import rclpy
from geometry_msgs.msg import TransformStamped
from rclpy.node import Node
from tf2_ros import TransformBroadcaster
from tf2_msgs.msg import TFMessage


class GazeboBoxTfPublisher(Node):
    def __init__(self):
        super().__init__("gazebo_box_tf_publisher")
        self._broadcaster = TransformBroadcaster(self)
        self._box_names = {f"box_{index}" for index in range(1, 6)}
        self._subscription = self.create_subscription(
            TFMessage,
            "/world/default/dynamic_pose/info",
            self._publish_box_transforms,
            10,
        )

    def _publish_box_transforms(self, message):
        transforms = []
        for source_transform in message.transforms:
            if source_transform.child_frame_id not in self._box_names:
                continue

            transform = TransformStamped()
            transform.header.stamp = self.get_clock().now().to_msg()
            transform.header.frame_id = "world"
            transform.child_frame_id = source_transform.child_frame_id
            transform.transform = source_transform.transform
            transforms.append(transform)

        if transforms:
            self._broadcaster.sendTransform(transforms)


def main():
    rclpy.init()
    node = GazeboBoxTfPublisher()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
