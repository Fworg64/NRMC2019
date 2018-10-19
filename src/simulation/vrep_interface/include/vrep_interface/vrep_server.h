#ifndef VREP_INTERFACE_VREP_SERVER_H
#define VREP_INTERFACE_VREP_SERVER_H


#include <ros/ros.h>
#include <ros/package.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>
#include <std_srvs/Trigger.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <dynamic_reconfigure/server.h>
#include <tf/transform_broadcaster.h>

#include <vrep_library/v_repLib.h>

#include <vrep_robot/vrep_robot.h>
#include <vrep_msgs/SpawnRobot.h>

namespace vrep_interface
{

class VREPServer
{
  public:
    explicit VREPServer();
    ~VREPServer();

  private:
    ros::NodeHandle *nh;
    //const std::string description_path;
    /*static ros::ServiceServer spawn_robot_server;
    static ros::ServiceServer spawn_robot_random_server;
    static ros::ServiceServer shutdown_vrep_server;
    static ros::Publisher *clock_publisher;
    static ros::Subscriber add_status_bar_message_subscriber;
    static tf::TransformBroadcaster* tf_broadcaster;

    static VREPRobot *robot;

    static bool spawnRobotService(vrep_msgs::SpawnRobot::Request &req, vrep_msgs::SpawnRobot::Response &res);
    static bool spawnRobotRandomService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
    static bool shutdownService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res);
    static void addStatusBarMessageCallback(const std_msgs::String::ConstPtr &msg);
    static void spinOnce();*/
};

}

#endif //VREP_INTERFACE_VREP_SERVER_H
