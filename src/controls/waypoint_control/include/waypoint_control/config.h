#ifndef WAYPOINT_CONTROL_WAYPOINT_CONTROL_STATES_H
#define WAYPOINT_CONTROL_WAYPOINT_CONTROL_STATES_H

#include <string>
#include <boost/math/constants/constants.hpp>
#include <ros/ros.h>
#include <waypoint_control/WaypointControlAction.h>
#include <utilities/config.h>

namespace waypoint_control
{
  typedef std::vector<Waypoint> Waypoints;
  using boost::math::double_constants::pi;

  enum class ControlState
  {
    error = 0,
    ready,
    new_goal,
    in_progress,
    finished,
    cancel,
    manual
  };

  enum class WaypointState
  {
    error = 0,
    ready,
    initial_angle_correction,
    driving,
    angle_correction,
    final_angle_correction,
    finished,
  };

  WaypointControlResult toResult(ControlState state);
  ControlState toControlState(WaypointControlGoal goal);
  std::string to_string(ControlState state);
  std::string to_string(WaypointState state);

  class Config : public utilities::Config
  {
  public:
    Config(ros::NodeHandle *nh);

    const double &dt();
    const double &rate();
    const double &maxAcceleration();
    const double &maxDuty();
    const double &maxInPlaceDuty();
    const double &minInPlaceDuty();
    const double &maxDrivingDuty();
    const double &minDrivingDuty();
    const double &maxManualDuty();
    const double &minManualDuty();
    const double &inPlaceK();
    const double &drivingKx();
    const double &drivingKy();
    const bool   &voltageCompensation();
    const double &minVoltage();
    const double &startVoltage();
    const double &maxCompensatedDuty();
    const double &batteryFilterK();

  private:
    double dt_;
    double rate_;
    double max_acceleration_;
    double max_duty_;
    double max_in_place_duty_;
    double min_in_place_duty_;
    double max_driving_duty_;
    double min_driving_duty_;
    double max_manual_duty_;
    double min_manual_duty_;
    double in_place_k_;
    double driving_kx_;
    double driving_ky_;
    bool voltage_compensation_;
    double min_voltage_;
    double start_voltage_;
    double max_compensated_duty_;
    double battery_filter_k_;
  };
}

#endif //WAYPOINT_CONTROL_WAYPOINT_CONTROL_STATES_H
