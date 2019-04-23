#ifndef COMPETITION_CONTROLLER_H
#define COMPETITION_CONTROLLER_H

#include <ros/ros.h>
#include <utilities/joy.h>
#include <competition/visuals.h>
#include <drive_controller/drive_control_client.h>
#include <dig_control/dig_control_client.h>

#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2/transform_datatypes.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Transform.h>

namespace competition
{
  using DriveControlState = drive_controller::ControlState;
  using DigControlState = dig_control::ControlState;
  using utilities::Joy;

  class Controller
  {
  public:
    Controller(ros::NodeHandle *nh, ros::Rate *rate, BezierSegment *path);

    void update();
    void joyCallback(const sensor_msgs::Joy::ConstPtr &joy);

  private:
    drive_controller::DriveControlClient drive_client;
    dig_control::DigControlClient dig_client;
    Joy joy;
    Visuals visuals;
    BezierSegment *path;

    double dt;
    ros::NodeHandle *nh;
    ros::Rate *rate;
    ros::Subscriber joy_subscriber;
    tf2_ros::Buffer tf_buffer;
    tf2_ros::TransformListener tf_listener;
    tf2::Stamped<tf2::Transform> transform;
  };
}

#endif //COMPETITION_CONTROLLER_H
