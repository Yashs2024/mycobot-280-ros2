# mycobot_ros2 #
![OS] (https://img.shields.io/ubuntu/v/ubuntu-wallpapers/noble)
[ROS2] (https://img.shields.io/ros/v/jazzy/rclcpp)
MyCobot 280 — ROS 2 Simulation and Control
A ROS 2 project for exploring the MyCobot 280 robotic arm using robot descriptions, visualization, simulation, and controller-based command interfaces. The project is being developed and tested in a ROS 2 Jazzy environment.
Project scope: Simulation and software integration. Do not assume physical-robot validation unless it is documented separately.

Overview
This project brings together the robot's URDF/Xacro description and ROS 2 tools so the arm can be inspected in RViz and used with the configured controllers in simulation.
Current capabilities
- Robot model description using URDF/Xacro.
- Robot visualization with RViz 2 and robot_state_publisher.
- Gazebo simulation integration and ROS 2 control configuration.
- Arm trajectory control through a FollowJointTrajectory action interface.
- Gripper command testing through the configured parallel-gripper action interface.
Technology stack
- ROS 2 Jazzy
- URDF and Xacro
- RViz 2
- Gazebo / Gazebo Sim (depending on the launch configuration in the repository)
- ros2_control and controller interfaces
- control_msgs
Repository structure
The repository contains the MyCobot-related ROS 2 packages, robot descriptions, controller configuration, and supporting simulation assets. Package names and launch files may vary as the project evolves; use colcon list to inspect what is available in your checkout.
Requirements
- A ROS 2 Jazzy installation
- The ROS 2 development tools, including colcon
- Required ROS dependencies for the packages in this repository
- RViz 2 and the simulator required by the launch files you intend to run
Build the project
Clone the repository into the src directory of a ROS 2 workspace:
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/Yashs2024/mycobot-280-ros2.git mycobot_ros2
Build from the workspace root:
cd ~/ros2_ws
source /opt/ros/jazzy/setup.bash
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source install/setup.bash
If rosdep reports a dependency it cannot resolve, check that the relevant package is installed or documented by its upstream project before continuing.
Launch the robot description
From a terminal where the workspace has been sourced:
source ~/ros2_ws/install/setup.bash
ros2 launch mycobot_description robot_state_publisher.launch.py
This launch file was used in the development workspace to start the robot-description and visualization workflow. Other simulation or controller launch files may be available in the repository; inspect them before use.
Controller interfaces
Arm
The configured arm trajectory action endpoint is:
/arm_controller/follow_joint_trajectory
Action type:
control_msgs/action/FollowJointTrajectory
The six arm joints in the current configuration are:
link1_to_link2
link2_to_link3
link3_to_link4
link4_to_link5
link5_to_link6
link6_to_link6_flange
Check the active action servers and controller state in your running system before sending goals:
ros2 action list -t
ros2 control list_controllers
Gripper — important interface detail
The current setup uses parallel_gripper_action_controller/GripperActionController, with this action endpoint and type:
/gripper_action_controller/gripper_cmd
control_msgs/action/ParallelGripperCommand
This differs from tutorials that use the older control_msgs/action/GripperCommand interface. Use the interface reported by your running controller rather than copying an older tutorial command unchanged.

Example goal for the current configured interface (only run while the corresponding controller is active):
ros2 action send_goal /gripper_action_controller/gripper_cmd control_msgs/action/ParallelGripperCommand "{command: {name: ['gripper_controller'], position: [-0.7], effort: [5.0]}}"
The acceptable position range depends on the robot/controller configuration. Verify the joint limits and test with a conservative goal before using other values.
Useful debugging commands
# List packages found in the workspace
cd ~/ros2_ws && colcon list

# See active ROS 2 nodes and topics
ros2 node list
ros2 topic list

# Inspect available actions
ros2 action list -t

# Inspect controller state (when ros2_control is running)
ros2 control list_controllers
If a controller or action is missing, confirm that the correct launch file is running and that the workspace overlay has been sourced in the same terminal.
Development status
Area	Status
URDF/Xacro description	Present in the development workspace
RViz / robot-state-publisher workflow	Launched during development
ROS 2 control and arm trajectory interface	Configured; validate against the current launch setup
Parallel gripper action	Command interface tested in the development setup
Full end-to-end arm trajectory validation	Continue testing and document results
Physical robot validation	Not claimed here


This table describes the development status known at the time this README was written; update it when additional tests are completed.
Roadmap
- Document a reproducible simulation launch sequence from a clean workspace.
- Add arm trajectory examples and verified joint-limit notes.
- Document RViz/Gazebo launch options and screenshots.
- Add a short demo video and a system architecture diagram.
- Record test results and any differences between simulation and hardware.
Notes on third-party assets
This repository may contain meshes, models, or simulation assets from upstream projects. Review and retain their license and attribution notices before redistributing the repository publicly.
Author
Yash Surve
- GitHub: @Yashs2024
- Project repository: mycobot-280-ros2
If you find an issue or have a suggestion, open a GitHub issue with the ROS 2 distribution, launch command, and relevant terminal output.
