^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for samxl_bt_ros_utils
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
------------------
- Added `apply_transform_to_pose` behavior to apply a transform to an input pose and return the transformed output pose.
- Added `pose_stamped_to_transform_stamped` behavior to convert a `geometry_msgs/PoseStamped` to a `geometry_msgs/TransformStamped`.
- Add ``DateString`` behavior to format date/time as string
- Add ``WriteTextFile`` behavior node from writing to a text file
- Add ``TimerDecorator`` for timing node execution duration
- Add `timeout` duration port (e.g. `15s`) to the `Trigger` behavior, for services that take longer than `ros_plugins_timeout` to respond
- Add templated ROS message print behavior with the following instances (Pre-existing behaviors have been replaced):

  - ``builtin_interfaces``:  ``PrintDuration``, ``PrintTime``
  - ``geometry_msgs``: ``PrintPoint``, ``PrintPose``, ``PrintPoseArray``, ``PrintPoseStamped``, ``PrintQuaternion``, ``PrintVector3``, ``PrintWrench``, ``PrintWrenchStamped``
  - ``sensor_msgs``:  ``PrintJointStates``
  - ``std_msgs``: ``PrintHeader``

- create a ``visualization_msgs/Marker`` publisher

- Add templated ROS message publishing behaviour with the following instances:
  
  - ``PublishPose``, ``PublishPoseArray``, ``PublishPoseStamped``, ``PublishWrenchStamped``

0.3.0 (2026-09-09)
------------------
- convert `geometry_msgs/PoseStamped` from YAML
- Improved conversions for `std_msgs/Header` and `builtin_interfaces/Duration`
- convert `std::shared_ptr<std::deque<geometry_msgs::msg::PoseStamped>>` from YAML
- Add ``joint_name_prefix`` parameter for ``ExecuteFJT`` behavior

0.2.0 (2026-07-29)
------------------
- Add `LoadTextFile` behavior
- Add integration tests for `LoadTextFile`
- Implement `convertFromString()` for various ros message types
- Add `PrintPoseArray` behaviour for testing
- Remove `BT` namespace in favor of a `samxl` namespace for each of the behaviors
- Improve logging
- Always return failure instead of throwing errors
- Add various rosbag2 interface behaviors
- convert `sensor_msgs/JointState` from YAML
- move all behaviors code to headers
- create a `SetBool` service client
- Add `PublishWrenchStamped` behavior
- Use payload field to select the workpiece to scan
- Move behaviours/conversions into individual packages

0.1.2 (2026-05-08)
------------------
- create a `triggerService` behavior

0.1.1 (2026-04-30)
------------------
- create a custom geometry_msgs/Pose type

0.1.0 (2026-03-17)
------------------
- create `nxtgen_automated_ndt_orchestration` package
