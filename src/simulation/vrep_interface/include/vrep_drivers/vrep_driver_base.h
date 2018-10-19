#ifndef MOTOR_ACCESS_VREP_MOTOR_ACCESS_H
#define MOTOR_ACCESS_VREP_MOTOR_ACCESS_H

#include <driver_access/driver_access.h>
#include <driver_access/limits.h>
#include <vrep_msgs/VREPDriverMessage.h>

#include <vrep_library/v_repLib.h>

#include <ros/ros.h>
#include <ros/callback_queue.h>


namespace vrep_interface
{

class VREPDriverBase : public driver_access::DriverAccess
{
  public:
    VREPDriverBase(driver_access::ID id);

    // This class is kind of a hack, so we override these
    double getPosition() override;
    double getVelocity() override;
    double getEffort() override;
    driver_access::Mode getMode() override;

    double setPoint();
    void updateHeader(std_msgs::Header *header);
    void updateHandle();
    void shutdown();

    virtual void updateState() = 0;

  protected:
    const std::string joint_name;
    vrep_msgs::VREPDriverMessage state;
    simInt handle;
    ros::Publisher *publisher;

  private:
    void callback(const vrep_msgs::VREPDriverMessageConstPtr &message);

    vrep_msgs::VREPDriverMessage command;
    uint32_t seq;

    ros::NodeHandle *nh;
    ros::Subscriber *subscriber;

};
}

#endif //MOTOR_ACCESS_VREP_MOTOR_ACCESS_H