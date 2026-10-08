#!/usr/bin/env python3
"""Service node that resets the boxes to their start slots in a random order.

Call: ros2 service call /reset_boxes std_srvs/srv/Trigger

Releases anything held by the suction cup or magnet, then teleports every box
to a randomly chosen slot of the initial layout in world.sdf via Gazebo's
set_pose service.
"""

import random
import subprocess
import threading
from concurrent.futures import ThreadPoolExecutor

import rclpy
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from std_srvs.srv import Trigger

WORLD = "default"
BOX_NAMES = [f"box_{index}" for index in range(1, 6)]
# Start slots (x, y, z) as in world.sdf; the boxes drop onto the belt from here.
SLOTS = [(0.5, -0.3 + 0.15 * index, 0.75) for index in range(len(BOX_NAMES))]


class ResetBoxes(Node):
    def __init__(self):
        super().__init__("reset_boxes")
        self.declare_parameter("random_seed", -1)
        self.declare_parameter("service_timeout", 2.0)
        self._timeout = float(self.get_parameter("service_timeout").value)
        seed = int(self.get_parameter("random_seed").value)
        self._rng = random.Random(seed if seed >= 0 else None)
        self._group = ReentrantCallbackGroup()
        self._detach_clients = {
            actuator: self.create_client(
                Trigger, f"/{actuator}/detach", callback_group=self._group
            )
            for actuator in ("suction_cup", "magnet")
        }
        self._busy = threading.Lock()
        self.create_service(
            Trigger, "/reset_boxes", self._reset, callback_group=self._group
        )

    def _release_actuators(self):
        for actuator, client in self._detach_clients.items():
            if not client.wait_for_service(timeout_sec=self._timeout):
                self.get_logger().warning(f"/{actuator}/detach unavailable; skipping")
                continue
            done = threading.Event()
            client.call_async(Trigger.Request()).add_done_callback(
                lambda _future: done.set()
            )
            done.wait(self._timeout)

    def _reset(self, _request, response):
        if not self._busy.acquire(blocking=False):
            response.success = False
            response.message = "A reset is already in progress"
            return response
        try:
            order = list(BOX_NAMES)
            self._rng.shuffle(order)
            self._release_actuators()

            placements = list(zip(order, SLOTS))
            with ThreadPoolExecutor(max_workers=len(placements)) as pool:
                results = list(pool.map(lambda item: set_pose(*item), placements))

            failed = [name for (name, _), ok in zip(placements, results) if not ok]
            response.success = not failed
            response.message = (
                f"Failed to reset {failed}"
                if failed
                else "Box order: " + ", ".join(order)
            )
            self.get_logger().info(response.message)
        finally:
            self._busy.release()
        return response


def set_pose(name, position):
    x, y, z = position
    request = (
        f'name: "{name}", position: {{x: {x}, y: {y}, z: {z}}}, '
        "orientation: {x: 0, y: 0, z: 0, w: 1}"
    )
    result = subprocess.run(
        [
            "ign", "service",
            "-s", f"/world/{WORLD}/set_pose",
            "--reqtype", "ignition.msgs.Pose",
            "--reptype", "ignition.msgs.Boolean",
            "--timeout", "3000",
            "--req", request,
        ],
        capture_output=True,
        text=True,
    )
    return result.returncode == 0 and "true" in result.stdout


def main():
    rclpy.init()
    node = ResetBoxes()
    executor = MultiThreadedExecutor()
    executor.add_node(node)
    try:
        executor.spin()
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
