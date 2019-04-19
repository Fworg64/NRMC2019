#include <dig_control/dig_control_server.h>


using namespace dig_control;


DigControlServer::DigControlServer(ros::NodeHandle *nh, DigController *controller) :
  dig_safety(false), backhoe_duty(0.0f), bucket_duty(0.0f), central_duty(0.0f), vibrator_duty(0.0f),
  nh(nh), controller(controller),
  server(*nh, "dig_control_server", false)
{
  joy_subscriber = nh->subscribe("joy", 1, &DigControlServer::joyCallback, this);
  controller->setControlState(ControlState::manual);
  server.registerGoalCallback(boost::bind(&DigControlServer::goalCallback, this));
  server.registerPreemptCallback(boost::bind(&DigControlServer::preemptCallback, this));
  server.start();
}

void DigControlServer::goalCallback()
{
  auto goal = server.acceptNewGoal();
  ControlState request = toControlState(*goal);
  ControlState current_state = controller->getControlState();

  ROS_INFO("[DigControlServer::goalCallback] Request for %s to %s",
            to_string(current_state).c_str(), to_string(request).c_str());
  switch (request)
  {
    case ControlState::manual:
    {
      // Set to manual regardless of current state
      controller->setControlState(request);
      controller->stop();
      server.setSucceeded(toResult(request));
      break;
    }
    case ControlState::dig:
    case ControlState::dump:
    {
      // Make sure controller is ready for new command
      if (current_state == ControlState::ready || current_state == ControlState::manual)
      {
        controller->setControlState(request);
      }
      else
      {
        server.setAborted(toResult(current_state));
        controller->setControlState(ControlState::error);
        controller->stop();
        ROS_ERROR("[DigControlServer::goalCallback] Unable to set control state from %s to %s",
                  to_string(current_state).c_str(), to_string(request).c_str());
      }
      break;
    }
    case ControlState::finish_dig:
    {
      if (current_state == ControlState::dig)
      {
        controller->setControlState(request);
        server.acceptNewGoal();
      }
      else
      {
        server.setAborted(toResult(current_state));
        controller->setControlState(ControlState::error);
        controller->stop();
        ROS_ERROR("[DigControlServer::goalCallback] Unable to set control state from %s to %s",
                  to_string(current_state).c_str(), to_string(request).c_str());
      }
      break;
    }
    case ControlState::finish_dump:
    {
      if (current_state == ControlState::dump)
      {
        controller->setControlState(request);
        server.acceptNewGoal();
      }
      else
      {
        server.setAborted(toResult(current_state));
        controller->setControlState(ControlState::error);
        controller->stop();
        ROS_ERROR("[DigControlServer::goalCallback] Unable to set control state from %s to %s",
                  to_string(current_state).c_str(), to_string(request).c_str());
      }
      break;
    }
    default:
    {
      server.setAborted(toResult(current_state));
      controller->setControlState(ControlState::error);
      controller->stop();
      ROS_ERROR("[DigControlServer::goalCallback] Unable to set control state from %s to %s",
                to_string(current_state).c_str(), to_string(request).c_str());
    }
  }
}

void DigControlServer::preemptCallback()
{
  ControlState current_state = controller->getControlState();
  server.setPreempted(toResult(current_state));
  ROS_INFO("[DigControlServer::preemptCallback] Preempting from %s", to_string(current_state).c_str());
}

void DigControlServer::joyCallback(const sensor_msgs::Joy::ConstPtr &joy)
{
  bool x  = joy->buttons[0] == 1; // Bucket down
  bool a  = joy->buttons[1] == 1; // Linear actuator in
  bool b  = joy->buttons[2] == 1; // Linear actuator out
  bool y  = joy->buttons[3] == 1; // Bucket up
  bool rb = joy->buttons[5] == 1; // Safety
  bool lt = joy->buttons[6] == 1; // Vibrator off
  bool rt = joy->buttons[7] == 1; // Vibrator on
  bool st = joy->buttons[9] == 1; // Start automatic dig
  bool up = joy->axes[5] >  0.5;  // Central drive up (Up on left pad)
  bool dp = joy->axes[5] < -0.5;  // Central drive down (Down on left pad)

  dig_safety = rb;

  if (dig_safety)
  {
    // Update bucket (Maintain state)
    if (x && y)
    {
      ROS_WARN("[teleop] Conflicting commands, x and y are both pressed, stopping motion");
      bucket_duty = 0.0;
    }
    else if (x)
    {
      bucket_duty = -BucketDuty::fast;
    }
    else if (y)
    {
      bucket_duty = BucketDuty::fast;
    }

    // Update backhoe (Maintain state)
    if (a && b)
    {
      ROS_WARN("[teleop] Conflicting commands, a and b are both pressed, stopping motion");
      backhoe_duty = 0.0f;
    }
    else if (a)
    {
      backhoe_duty = -BackhoeDuty::normal;
    }
    else if (b)
    {
      backhoe_duty = BackhoeDuty::fast;
    }

    // Update central drive
    if (up && dp)
    {
      ROS_WARN("[teleop] Conflicting commands, up and dp are both pressed, stopping motion");
      central_duty = 0.0f;
    }
    else if (up)
    {
      central_duty = CentralDriveDuty::fast;
    }
    else if (dp)
    {
      central_duty = -CentralDriveDuty::normal;
    }
    else
    {
      central_duty = 0.0f;
    }

    // Update vibrator (Maintain state)
    if (lt && rt)
    {
      ROS_WARN("[teleop] Conflicting commands, lt and rt are both pressed, stopping motion");
      vibrator_duty = 0.0f;
    }
    else if (lt)
    {
      vibrator_duty = 0.0f;
    }
    else if (rt)
    {
      vibrator_duty = 0.75;
    }
  }
  else
  {
    central_duty = 0.0f;
    backhoe_duty = 0.0f;
    bucket_duty = 0.0f;
    vibrator_duty = 0.0f;
  }
}

DigControlResult DigControlServer::toResult(ControlState state)
{
  DigControlResult result;
  result.control_state = (DigControlResult::_control_state_type)state;
  return result;
}

ControlState DigControlServer::toControlState(DigControlGoal goal)
{
  return (ControlState)goal.control_state;
}

void DigControlServer::update()
{
  controller->update();

  ControlState dig_state = controller->getControlState();
  if (dig_safety)
  {
    if (dig_state == ControlState::error)
    {
      ROS_ERROR("Dig controller is in an error state");
      controller->stop();
    }
    else if (dig_state == ControlState::manual)
    {
      controller->setCentralDriveDuty(central_duty);
      controller->setBackhoeDuty(backhoe_duty);
      controller->setVibratorDuty(vibrator_duty);
      controller->setBucketDuty(bucket_duty);
    }
  }
  else
  {
    controller->setControlState(ControlState::manual);
    controller->stop();
  }

  if (server.isActive())
  {
    DigControlFeedback feedback;
    feedback.control_state       = (DigControlFeedback::_control_state_type)       controller->getControlState();
    feedback.dig_state           = (DigControlFeedback::_dig_state_type)           controller->getDigState();
    feedback.backhoe_state       = (DigControlFeedback::_backhoe_state_type)       controller->getBackhoeState();
    feedback.central_drive_state = (DigControlFeedback::_central_drive_state_type) controller->getCentralDriveState();
    feedback.bucket_state        = (DigControlFeedback::_bucket_state_type)        controller->getBucketState();
    server.publishFeedback(feedback);
  }
}

int main(int argc, char* argv[])
{
  ros::init(argc, argv, "dig_control_server");
  ros::NodeHandle nh;
  DigController controller;
  DigControlServer server(&nh, &controller);
  ros::Rate rate(50);
  while (ros::ok())
  {
    server.update();
    ros::spinOnce();
    rate.sleep();
  }
  controller.stop();
}


