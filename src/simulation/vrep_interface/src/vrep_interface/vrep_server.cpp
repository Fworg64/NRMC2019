#include "vrep_interface/vrep_server.h"
#include <rosgraph_msgs/Clock.h>

using namespace vrep_interface;

VREPServer::VREPServer()
{
  int argc = 0; char **argv = NULL;
  ros::init(argc, argv, "vrep");
  if (!ros::master::check())
  {
    simAddStatusbarMessage("[WARN]: Unable to start, ros master isn't running");
    throw std::runtime_error("Unable to start, ros master isn't running");
  }
  nh = new ros::NodeHandle("~");
  std::cout << "Starting server..." << std::endl;

  std::string description_path = ros::package::getPath("description");
  if (description_path.empty())
  {
    error("Unable to find the description package path, have you sourced your workspace?");
    throw std::runtime_error("Unable to find the description package path, have you sourced your workspace?");
  }

  std::cout << "Initialized" << std::endl;

  spawn_robot_server = nh->advertiseService("spawn_robot", &VREPServer::spawnRobotService, this);
  spawn_robot_random_server = nh->advertiseService("spawn_robot_random", &VREPServer::spawnRobotRandomService, this);
  shutdown_vrep_server = nh->advertiseService("shutdown", &VREPServer::shutdownService, this);
  tf_broadcaster = new tf::TransformBroadcaster();
  clock_publisher = new ros::Publisher;
  (*clock_publisher) = nh->advertise<rosgraph_msgs::Clock>("/clock", 10, true);

  simInt status = simLoadScene((description_path + "/vrep_models/arena.ttt").c_str());
  if (status == -1)
  {
    error("Unable to load scene");
  }

  robot = new VREPRobot;
  robot->initialize(description_path + "/vrep_models/robot.ttm");
  robot->spawnRobot();
  info("Server started");
}

VREPServer::~VREPServer()
{
  nh->shutdown();
  ros::shutdown();
}

bool VREPServer::spawnRobotRandomService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res)
{
  try
  {
    robot->spawnRobot();
    return true;
  }
  catch (const std::runtime_error &e)
  {
    error(e.what());
    return false;
  }
}

bool VREPServer::spawnRobotService(vrep_msgs::SpawnRobot::Request &req,
                                   vrep_msgs::SpawnRobot::Response &res)
{
  try
  {
    robot->spawnRobot(req.x, req.y, req.omega);
  }
  catch (const std::runtime_error &e)
  {
    error(e.what());
    return false;
  }
}

bool VREPServer::shutdownService(std_srvs::Trigger::Request &req, std_srvs::Trigger::Response &res)
{
  simAddStatusbarMessage("[Service shutdownService] Trying to shutdown");
  res.success = 1;
  res.message = "Trying to shutdown...";
  simQuitSimulator(1);
  return true;
}

void VREPServer::info(const std::string &message)
{
  simAddStatusbarMessage(("[INFO]: " + message).c_str());
  std::cout << message.c_str() << std::endl;
}

void VREPServer::warn(const std::string &message)
{
  simAddStatusbarMessage(("[WARN]: " + message).c_str());
  std::cout << message.c_str() << std::endl;
}

void VREPServer::error(const std::string &message)
{
  simAddStatusbarMessage(("[ERROR]: " + message).c_str());
  std::cout << message.c_str() << std::endl;
}
