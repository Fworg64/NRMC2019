#include <backhoe_controller/backhoe_controller.h>
#include <bucket_controller/bucket_controller.h>
#include <std_msgs/Empty.h>
#include <ros/ros.h>
#include <vesc_access/vesc_access.h>
// TODO backhoe params
#include <visualization_msgs/Marker.h>
#include <sim_robot/sim_outriggers.h>
#include <sim_robot/sim_bucket.h>
#include <sim_robot/sim_backhoe.h>

#include <sensor_msgs/JointState.h>
#include <cmath>

#include "dig_dump_action/dig_dump_action.h"
#include "wheel_params/wheel_params.h"

#include "safety_vesc/backhoe_safety_controller.h"
#include "safety_vesc/linear_safety_controller.h"

#define DIGGING_CONTROL_RATE_HZ 50.0

#define FLAP_START_ANGLE
#define FLAP_END_ANGLE
#define FLAP_RANGE

#define POLYFIT_RANK 5
double monoboom_c[] = {-.0808, -0.0073,  0.0462,  0.9498,  -0.0029};
double flap_c[]     = {85.0010, -376.8576, 620.7329, -453.8172,  126.0475};


bool should_initialize = false;

void callback (const std_msgs::Empty::ConstPtr &msg)
{
  should_initialize = true;
}

/* It shall be known that the angles are defined as such:
Backhoe.Shoulder angle is the Central Drive angle.
Central Drive Angle =0 When monoboom angle =0


*/

double nrmc_polyfit(double* c, double x)
{
    double ret = c[0]*x*x*x*x + c[1]*x*x*x + c[2]*x*x + c[3] *x + c[4];
    return ret;
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "the_backhoe_master");
  
  if( ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Debug) ) 
  {
   ros::console::notifyLoggerLevelsChanged();
  }

  ros::NodeHandle node("~");
  ros::NodeHandle globalNode;
  ros::WallRate rate(DIGGING_CONTROL_RATE_HZ);  // should be 50 Hz

  ros::Subscriber initializeSub = globalNode.subscribe("init_digging",100,callback);

  bool simulating;
  if (node.hasParam("simulating_digging"))
  {
    node.getParam("simulating_digging", simulating);
    if (simulating)
    {
      ROS_WARN("\nRUNNING DIGGING AS SIMULATION\n");
    }
    else
    {
      ROS_WARN("\nRUNNING DIGGING AS PHYSICAL\n");
    }
  }
  else
  {
    ROS_ERROR("\n\nsimulating_digging param not defined! aborting.\n\n");
    return -1;
  }

  double backhoeInitialShoulderTheta;
  double backhoeInitialWristTheta;

  iVescAccess *bucketBigConveyorVesc;
  iVescAccess *bucketLittleConveyorVesc;
  iVescAccess *bucketSifterVesc;
  iVescAccess *backhoeShoulderVesc;
  iVescAccess *backhoeWristVesc;
  ros::Time lastTime=ros::Time::now();
  ros::WallRate vesc_init_rate (10);
  // these should not be initialized if we are not simulating
  SimBucket *bucketSimulation;

  SimBackhoe *backhoeSimulation;
  if (simulating)
  {
    // SimBucket
    bucketSimulation = new SimBucket();
    bucketBigConveyorVesc = bucketSimulation->getBigConveyorVesc();
    bucketLittleConveyorVesc = bucketSimulation->getLittleConveyorVesc();
    bucketSifterVesc = bucketSimulation->getSifterVesc();
    // SimBackhoe
    backhoeSimulation = new SimBackhoe(0.3, 0.1, central_joint_params.lower_limit_position, central_joint_params.upper_limit_position,
                                       linear_joint_params.lower_limit_position, linear_joint_params.upper_limit_position);  // shoulder and wrist angle, limits
    backhoeShoulderVesc = backhoeSimulation->getShoulderVesc();
    backhoeWristVesc = backhoeSimulation->getWristVesc();
    // populate inital backhoe position
    backhoeInitialShoulderTheta = 0;
    backhoeInitialWristTheta = 0;
  }
  else
  {
    // Don't use these pointers
    // This is a physical run.
    // You'll cause exceptions.
    bucketSimulation = NULL;
    backhoeSimulation = NULL;
    bool no_except = false;
    while (!no_except  && ros::ok()) {
      try {
        backhoeShoulderVesc = new VescAccess(shoulder_param, true);
        backhoeWristVesc = new VescAccess(linear_param, true);
        bucketLittleConveyorVesc = new VescAccess(small_conveyor_param);
        bucketBigConveyorVesc = new VescAccess(large_conveyor_param);
        bucketSifterVesc = new VescAccess(sifter_param);
        no_except = true;
      } catch (VescException e) {
        ROS_WARN("%s",e.what ());
        no_except = false;
      }
      if (!no_except && (ros::Time::now() - lastTime).toSec() > 10){
        ROS_ERROR ("Vesc exception thrown for more than 10 seconds");
        ros::shutdown ();
      }
        vesc_init_rate.sleep();
    }

    // initialize real vescs here
  }
 LinearSafetyController linearSafety(linear_joint_params, backhoeWristVesc);
  BackhoeSafetyController backhoeSafety(central_joint_params, backhoeShoulderVesc);
  ROS_INFO ("WAITING for init");
  /*while (ros::ok() && !should_initialize)
  {
    ros::spinOnce();
    rate.sleep();
  }*/
  
  ROS_INFO("Going to INIT backhoeSafety");
  ROS_DEBUG("Debug info shown");
    
  backhoeSafety.init();

    ROS_INFO ("INITING");

  bool isLinearInit = false;
  while (ros::ok() && !isLinearInit)
  {
    if (simulating)
    {
      backhoeSimulation->update(1.0 / DIGGING_CONTROL_RATE_HZ);
      ROS_DEBUG("Linear at %.4f", backhoeSimulation->getWrTheta());
      ROS_DEBUG("Linear from vesc at %.4f", backhoeSimulation->getWristVesc()->getPotPosition());
      ROS_DEBUG("Linear Limit state %d", backhoeSimulation->getWristVesc()->getLimitSwitchState());
      ROS_DEBUG("Linear vel set to  %.4f", backhoeSimulation->getWristVesc()->getLinearVelocity());
      //if (((SimVesc *)backhoeSimulation->getWristVesc())->ableToHitGround())
      //  ROS_INFO("Linear able to hit ground");
    }
    isLinearInit = linearSafety.init();
    //ROS_DEBUG("Called linearSafetyinit");
    rate.sleep();
    //ROS_DEBUG("slept");
    ros::spinOnce();
    //ROS_DEBUG("spun");
  }
  ROS_INFO("Going to move central monoboom");
  backhoeSafety.setPositionSetpoint(CENTRAL_TRANSPORT_ANGLE);
  while (ros::ok () && !backhoeSafety.isAtSetpoint())
  {
    if (simulating)
    {
        bucketSimulation->update(1.0/DIGGING_CONTROL_RATE_HZ);
        backhoeSimulation->update(1.0/DIGGING_CONTROL_RATE_HZ);
    }
    backhoeSafety.update(1.0/DIGGING_CONTROL_RATE_HZ);
    rate.sleep();
    ros::spinOnce();
  }
  ROS_INFO("Going init controllers");

  BucketController bucketC(bucketBigConveyorVesc, bucketLittleConveyorVesc, bucketSifterVesc);
  BackhoeController backhoeC(&backhoeSafety, &linearSafety);

  bool bucket_init = false;
  while(ros::ok() && bucket_init)
  {
    bucket_init = bucketC.init();
    rate.sleep();
    ros::spinOnce();
  }


  // pass vescs (sim or physical) to controllers

  ROS_INFO("Init'd");

  DigDumpAction ddAct(&backhoeC, &bucketC, argc, argv);

  ros::Publisher JsPub = globalNode.advertise<sensor_msgs::JointState>("joint_states", 100);

  while (ros::ok())
  {
    if (simulating)  // update simulations if neccesary
    {
      bucketSimulation->update(1.0 / DIGGING_CONTROL_RATE_HZ);
      backhoeSimulation->update(1.0 / DIGGING_CONTROL_RATE_HZ);
    }
    // update controllers

    backhoeC.update(1.0 / DIGGING_CONTROL_RATE_HZ);

    // display output if simulating
    sensor_msgs::JointState robotAngles;
    if (simulating)
    {
      robotAngles.header.stamp = ros::Time::now();

      robotAngles.name.push_back("frame_to_monoboom");
      double urdf_monoboom = -(-M_PI_4 + nrmc_polyfit(monoboom_c,backhoeSimulation->getShTheta()));
      robotAngles.position.push_back(urdf_monoboom);
      
      robotAngles.name.push_back("frame_to_gravel_bucket");
      double urdf_bucket = (backhoeSimulation->getShTheta() - 1.484) > 0 ? 3.0613 * (backhoeSimulation->getShTheta() - 1.484) : 0;
      robotAngles.position.push_back(urdf_bucket);

      robotAngles.name.push_back("monoboom_to_bucket"); //digging bucket
      robotAngles.position.push_back(backhoeSimulation->getWrTheta());
      
      robotAngles.name.push_back("left_flap_joint"); 
      double urdf_flap = (backhoeSimulation->getShTheta() < 1.275 && backhoeSimulation->getShTheta() > 0.785 ) ? 
                         nrmc_polyfit(flap_c, backhoeSimulation->getShTheta()) : (backhoeSimulation->getShTheta() >= 1.275 ? 0 : 2.424);
      robotAngles.position.push_back(urdf_flap);
      robotAngles.name.push_back("right_flap_joint"); 
      robotAngles.position.push_back(urdf_flap);

      JsPub.publish(robotAngles);
      //ROS_DEBUG("wrist joint state published with angle %f \n", backhoeSimulation->getWrTheta());
      //ROS_DEBUG("monoboom angle     : %f \n", nrmc_polyfit(monoboom_c,backhoeSimulation->getShTheta()));
      //ROS_DEBUG("URDF monoboom angle: %f \n", urdf_monoboom);
      //ROS_DEBUG("shoulder joint stat: %f \n", backhoeSimulation->getShTheta());
      //ROS_DEBUG("urdf flap angle    : %f \n", urdf_flap);

      
    }
    else  // display output for physical
    {
    }
    //ROS_INFO("backhoe controller says CD at %.4f", backhoeSafety.getPositionEstimate());
    //ROS_INFO("backhoe controller says LA at %.4f", linearSafety.getPositionEstimate());
    //ROS_INFO("Digdump AS states: %d, %d", ddAct.digging_state, ddAct.dumping_state);

    ros::spinOnce();
    rate.sleep();
  }
}
