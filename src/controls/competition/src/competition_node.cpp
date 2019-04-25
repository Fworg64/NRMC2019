#include <ros/ros.h>
#include <competition/competition_controller.h>

int main(int argc, char **argv)
{
  ros::init(argc, argv, "drive_control_client");
  ros::NodeHandle nh;
  ros::Rate rate(50);

  waypoint_control::Waypoints waypoints;

  competition::Controller controller(&nh, &rate, waypoints);

  while(ros::ok())
  {
    ros::spinOnce();
    controller.update();
    rate.sleep();
  }
}