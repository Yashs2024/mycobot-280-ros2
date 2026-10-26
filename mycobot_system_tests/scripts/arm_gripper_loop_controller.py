#!/usr/bin/env python3

"""
Control the MyCobot 280 arm and gripper in a repetitive loop.

The arm is moved to a target position, the gripper closes,
the arm returns home, and the gripper opens.

Author: Yash Surve
"""

import time

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient

from control_msgs.action import FollowJointTrajectory
from control_msgs.action import ParallelGripperCommand

from trajectory_msgs.msg import JointTrajectoryPoint
from builtin_interfaces.msg import Duration


class ArmGripperLoopController(Node):

    def __init__(self):
        super().__init__('arm_gripper_loop_controller')

        # Arm action client
        self.arm_client = ActionClient(
            self,
            FollowJointTrajectory,
            '/arm_controller/follow_joint_trajectory'
        )

        # Gripper action client
        self.gripper_client = ActionClient(
            self,
            ParallelGripperCommand,
            '/gripper_action_controller/gripper_cmd'
        )

        self.get_logger().info('Waiting for action servers...')

        if not self.arm_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error(
                'Arm action server not available!'
            )
            raise RuntimeError('Arm action server unavailable')

        if not self.gripper_client.wait_for_server(timeout_sec=10.0):
            self.get_logger().error(
                'Gripper action server not available!'
            )
            raise RuntimeError('Gripper action server unavailable')

        self.get_logger().info('Action servers connected!')

        # MyCobot 280 arm joints
        self.joint_names = [
            'link1_to_link2',
            'link2_to_link3',
            'link3_to_link4',
            'link4_to_link5',
            'link5_to_link6',
            'link6_to_link6_flange'
        ]

        # Target and home positions
        self.target_pos = [
            1.345,
            -1.23,
            0.264,
            -0.296,
            0.389,
            -1.5
        ]

        self.home_pos = [
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0
        ]

        # Run the sequence once after startup
        self.create_timer(1.0, self.control_loop_callback)

        self.running = False

    def send_arm_command(self, positions):
        """Send a trajectory goal to the arm."""

        goal_msg = FollowJointTrajectory.Goal()

        goal_msg.trajectory.joint_names = self.joint_names

        point = JointTrajectoryPoint()
        point.positions = positions
        point.time_from_start = Duration(sec=2, nanosec=0)

        goal_msg.trajectory.points = [point]

        self.get_logger().info(
            f'Sending arm positions: {positions}'
        )

        return self.arm_client.send_goal_async(goal_msg)

    def send_gripper_command(self, position):
        """Send a ParallelGripperCommand goal."""

        goal_msg = ParallelGripperCommand.Goal()

        # ParallelGripperCommand uses sensor_msgs/JointState
        goal_msg.command.name = ['gripper_controller']
        goal_msg.command.position = [position]
        goal_msg.command.effort = [5.0]

        self.get_logger().info(
            f'Sending gripper position: {position}'
        )

        return self.gripper_client.send_goal_async(goal_msg)

    def control_loop_callback(self):
        """Execute one arm/gripper cycle."""

        if self.running:
            return

        self.running = True

        self.get_logger().info('Moving to target position')
        self.send_arm_command(self.target_pos)

        time.sleep(2.5)

        self.get_logger().info(
            'Reached target position - Pausing'
        )
        time.sleep(1.0)

        self.get_logger().info('Closing gripper')
        self.send_gripper_command(-0.7)

        time.sleep(0.5)

        self.get_logger().info('Moving to home position')
        self.send_arm_command(self.home_pos)

        time.sleep(2.5)

        self.get_logger().info(
            'Reached home position - Pausing'
        )
        time.sleep(1.0)

        self.get_logger().info('Opening gripper')
        self.send_gripper_command(0.0)

        time.sleep(0.5)

        self.get_logger().info('Cycle complete')

        self.running = False


def main(args=None):

    rclpy.init(args=args)

    controller = None

    try:
        controller = ArmGripperLoopController()
        rclpy.spin(controller)

    except KeyboardInterrupt:
        pass

    except Exception as e:
        print(f'Controller error: {e}')

    finally:
        if controller is not None:
            controller.destroy_node()

        rclpy.shutdown()


if __name__ == '__main__':
    main()
