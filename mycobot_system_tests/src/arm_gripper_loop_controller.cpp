/**
 * @file arm_gripper_loop_controller.cpp
 * @brief Control MyCobot 280 arm and gripper in a repetitive pick-and-place loop.
 *
 * This node:
 *   1. Moves the arm to the target position
 *   2. Closes the gripper
 *   3. Returns the arm to the home position
 *   4. Opens the gripper
 *   5. Repeats continuously
 *
 * ROS 2 Action Servers:
 *   Arm:
 *     /arm_controller/follow_joint_trajectory
 *
 *   Gripper:
 *     /gripper_action_controller/gripper_cmd
 *
 * Gripper Action Type:
 *   control_msgs/action/ParallelGripperCommand
 *
 * Author: Yash Surve
 */

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "control_msgs/action/parallel_gripper_command.hpp"

#include "trajectory_msgs/msg/joint_trajectory_point.hpp"

using namespace std::chrono_literals;

class ArmGripperLoopController : public rclcpp::Node
{
public:

  using ArmAction = control_msgs::action::FollowJointTrajectory;
  using GripperAction = control_msgs::action::ParallelGripperCommand;

  using ArmGoalHandle =
    rclcpp_action::ClientGoalHandle<ArmAction>;

  using GripperGoalHandle =
    rclcpp_action::ClientGoalHandle<GripperAction>;

  ArmGripperLoopController()
  : Node("arm_gripper_loop_controller")
  {
    RCLCPP_INFO(
      this->get_logger(),
      "Starting Arm Gripper Loop Controller..."
    );

    /*
     * ============================================================
     * ACTION CLIENTS
     * ============================================================
     */

    // Arm action client
    arm_client_ =
      rclcpp_action::create_client<ArmAction>(
        this,
        "/arm_controller/follow_joint_trajectory"
      );

    // Gripper action client
    gripper_client_ =
      rclcpp_action::create_client<GripperAction>(
        this,
        "/gripper_action_controller/gripper_cmd"
      );

    /*
     * ============================================================
     * WAIT FOR ACTION SERVERS
     * ============================================================
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Waiting for arm action server..."
    );

    if (!arm_client_->wait_for_action_server(20s))
    {
      throw std::runtime_error(
        "Arm action server not available"
      );
    }

    RCLCPP_INFO(
      this->get_logger(),
      "Arm action server connected."
    );

    RCLCPP_INFO(
      this->get_logger(),
      "Waiting for gripper action server..."
    );

    if (!gripper_client_->wait_for_action_server(20s))
    {
      throw std::runtime_error(
        "Gripper action server not available"
      );
    }

    RCLCPP_INFO(
      this->get_logger(),
      "Gripper action server connected."
    );

    RCLCPP_INFO(
      this->get_logger(),
      "Both action servers connected!"
    );

    /*
     * ============================================================
     * MYCOBOT 280 JOINTS
     * ============================================================
     */

    joint_names_ =
    {
      "link1_to_link2",
      "link2_to_link3",
      "link3_to_link4",
      "link4_to_link5",
      "link5_to_link6",
      "link6_to_link6_flange"
    };

    /*
     * ============================================================
     * ARM POSITIONS
     * ============================================================
     */

    // Target position from your tutorial
    target_position_ =
    {
      1.345,
      -1.23,
      0.264,
      -0.296,
      0.389,
      -1.5
    };

    // Home position
    home_position_ =
    {
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0
    };

    /*
     * ============================================================
     * LOOP TIMER
     * ============================================================
     *
     * Start the first cycle after 1 second.
     */

    timer_ =
      this->create_wall_timer(
        1s,
        std::bind(
          &ArmGripperLoopController::controlLoopCallback,
          this
        )
      );
  }

private:

  /*
   * ==============================================================
   * SEND ARM COMMAND
   * ==============================================================
   */

  void sendArmCommand(
    const std::vector<double> & positions)
  {
    auto goal_msg = ArmAction::Goal();

    // Joint names
    goal_msg.trajectory.joint_names =
      joint_names_;

    // Create trajectory point
    trajectory_msgs::msg::JointTrajectoryPoint point;

    point.positions = positions;

    // Move in 2 seconds
    point.time_from_start.sec = 2;
    point.time_from_start.nanosec = 0;

    goal_msg.trajectory.points.push_back(point);

    RCLCPP_INFO(
      this->get_logger(),
      "Sending arm trajectory..."
    );

    auto send_goal_options =
      rclcpp_action::Client<ArmAction>::SendGoalOptions();

    send_goal_options.goal_response_callback =
      [this](const ArmGoalHandle::SharedPtr & goal_handle)
      {
        if (!goal_handle)
        {
          RCLCPP_ERROR(
            this->get_logger(),
            "Arm goal was rejected."
          );
        }
        else
        {
          RCLCPP_INFO(
            this->get_logger(),
            "Arm goal accepted."
          );
        }
      };

    arm_client_->async_send_goal(
      goal_msg,
      send_goal_options
    );
  }

  /*
   * ==============================================================
   * SEND GRIPPER COMMAND
   * ==============================================================
   *
   * Your controller uses:
   *
   *   control_msgs/action/ParallelGripperCommand
   *
   * Its command field is sensor_msgs/JointState.
   */

  void sendGripperCommand(
    double position)
  {
    auto goal_msg = GripperAction::Goal();

    /*
     * The joint configured in:
     *
     * ros2_controllers.yaml
     *
     * is:
     *
     * joint: gripper_controller
     */

    goal_msg.command.name =
    {
      "gripper_controller"
    };

    goal_msg.command.position =
    {
      position
    };

    goal_msg.command.effort =
    {
      5.0
    };

    RCLCPP_INFO(
      this->get_logger(),
      "Sending gripper command: %.3f",
      position
    );

    auto send_goal_options =
      rclcpp_action::Client<GripperAction>::SendGoalOptions();

    send_goal_options.goal_response_callback =
      [this](const GripperGoalHandle::SharedPtr & goal_handle)
      {
        if (!goal_handle)
        {
          RCLCPP_ERROR(
            this->get_logger(),
            "Gripper goal was rejected."
          );
        }
        else
        {
          RCLCPP_INFO(
            this->get_logger(),
            "Gripper goal accepted."
          );
        }
      };

    gripper_client_->async_send_goal(
      goal_msg,
      send_goal_options
    );
  }

  /*
   * ==============================================================
   * CONTROL LOOP
   * ==============================================================
   */

  void controlLoopCallback()
  {
    // Stop timer while one complete cycle is running
    timer_->cancel();

    /*
     * ------------------------------------------------------------
     * STEP 1: MOVE TO TARGET
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "======================================"
    );

    RCLCPP_INFO(
      this->get_logger(),
      "Moving to target position..."
    );

    sendArmCommand(
      target_position_
    );

    std::this_thread::sleep_for(
      2500ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 2: PAUSE AT TARGET
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Reached target position - Pausing..."
    );

    std::this_thread::sleep_for(
      1000ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 3: CLOSE GRIPPER
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Closing gripper..."
    );

    sendGripperCommand(
      -0.7
    );

    std::this_thread::sleep_for(
      1000ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 4: RETURN HOME
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Moving to home position..."
    );

    sendArmCommand(
      home_position_
    );

    std::this_thread::sleep_for(
      2500ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 5: PAUSE AT HOME
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Reached home position - Pausing..."
    );

    std::this_thread::sleep_for(
      1000ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 6: OPEN GRIPPER
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Opening gripper..."
    );

    sendGripperCommand(
      0.0
    );

    std::this_thread::sleep_for(
      1000ms
    );

    /*
     * ------------------------------------------------------------
     * STEP 7: COMPLETE
     * ------------------------------------------------------------
     */

    RCLCPP_INFO(
      this->get_logger(),
      "Cycle complete."
    );

    RCLCPP_INFO(
      this->get_logger(),
      "Starting next cycle..."
    );

    RCLCPP_INFO(
      this->get_logger(),
      "======================================"
    );

    // Start another cycle after 1 second
    timer_ =
      this->create_wall_timer(
        1s,
        std::bind(
          &ArmGripperLoopController::controlLoopCallback,
          this
        )
      );
  }

  /*
   * ==============================================================
   * MEMBER VARIABLES
   * ==============================================================
   */

  // Action clients
  rclcpp_action::Client<ArmAction>::SharedPtr
    arm_client_;

  rclcpp_action::Client<GripperAction>::SharedPtr
    gripper_client_;

  // Timer
  rclcpp::TimerBase::SharedPtr
    timer_;

  // Joint names
  std::vector<std::string>
    joint_names_;

  // Arm positions
  std::vector<double>
    target_position_;

  std::vector<double>
    home_position_;
};


/*
 * ================================================================
 * MAIN
 * ================================================================
 */

int main(
  int argc,
  char ** argv)
{
  rclcpp::init(
    argc,
    argv
  );

  try
  {
    auto node =
      std::make_shared<
        ArmGripperLoopController
      >();

    rclcpp::spin(
      node
    );
  }
  catch (
    const std::exception & e)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger(
        "arm_gripper_loop_controller"
      ),
      "Exception caught: %s",
      e.what()
    );
  }

  rclcpp::shutdown();

  return 0;
}
