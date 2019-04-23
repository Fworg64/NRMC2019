#ifndef WAYPOINT_CONTROL_WAYPOINT_CONTROL_SERVER_H
#define WAYPOINT_CONTROL_WAYPOINT_CONTROL_SERVER_H

#include <ros/ros.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2/transform_datatypes.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Transform.h>
#include <actionlib/server/simple_action_server.h>
#include <sensor_msgs/Joy.h>

#include <navigation_msgs/FollowPathAction.h>
#include <waypoint_control/WaypointControlAction.h>
#include <teleop_interface/teleop_interface.h>
#include <localization/StateVector.h>
#include <waypoint_control/waypoint_control_states.h>
#include <waypoint_control/waypoint_controller.h>

namespace waypoint_control
{
  typedef tf2::Stamped<tf2::Transform> StampedTransform;

  class WaypointControlServer
  {
  public:

    WaypointControlServer(ros::NodeHandle *nh, iVescAccess *fl, iVescAccess *fr, iVescAccess *br, iVescAccess *bl);

    void goalCallback();
    void preemptCallback();
    void joyCallback(const sensor_msgs::Joy::ConstPtr &joy);
    void update(double dt);
    void stop();

  private:
    bool debug, manual_safety, autonomy_safety, direction; // forward = true
    uint32_t seq;
    float teleop_left, teleop_right, max_velocity;
    double x, y, theta;

    ControlState state;
    iVescAccess *fl, *fr, *br, *bl;
    TeleopInterface teleop;
    WaypointController controller;

    ros::NodeHandle *nh;
    ros::Subscriber joy_subscriber;
    ros::Publisher joint_publisher;
    actionlib::SimpleActionServer<WaypointControlAction> server;
    sensor_msgs::JointState joint_angles;

  };
}


#endif //WAYPOINT_CONTROL_WAYPOINT_CONTROL_SERVER_H