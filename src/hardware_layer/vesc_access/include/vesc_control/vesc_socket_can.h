#include <iostream>
// socketcan includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/bcm.h>

#include <sys/time.h>
#include "vesc_control/ivesc.h"
#include <string>
#include "sensor_msgs/JointState.h"
#include "std_msgs/Float32.h"
#include "ros/ros.h"
class Vesc : public iVesc
{
public:
  typedef enum
  {
    MC_STATE_OFF = 0,
    MC_STATE_DETECTING,
    MC_STATE_RUNNING,
    MC_STATE_FULL_BRAKE,
  } mc_state;
  typedef enum
  {
    FAULT_CODE_NONE = 0,
    FAULT_CODE_OVER_VOLTAGE,
    FAULT_CODE_UNDER_VOLTAGE,
    FAULT_CODE_DRV8302,
    FAULT_CODE_ABS_OVER_CURRENT,
    FAULT_CODE_OVER_TEMP_FET,
    FAULT_CODE_OVER_TEMP_MOTOR
  } mc_fault_code;

private:
  struct ifreq ifr_;
  struct sockaddr_can addr_;
  int s_;
  int sbcm_;

  uint32_t quirks_ = 0;
  uint8_t enable_ = 1;
  uint8_t controller_id_;

  int32_t rpm_;
  float current_;
  float duty_cycle_;
  float position_;
  int32_t tachometer_;
  float watt_hours_;
  float in_current_;
  float vin_;
  float motor_temp_;
  float pcb_temp_;
  bool encoder_index_;
  int32_t adc_;
  bool flimit_;
  bool rlimit_;
  mc_fault_code fault_code_;
  mc_state state_;

  struct timeval previous_message_time_;

  typedef struct VESC_set
  {
    int set : 32;
  } VESC_set;
  typedef struct VESC_status
  {
    int32_t rpm : 32;
    int16_t current : 16;
    int16_t duty_cycle : 16;
  } VESC_status;

  // most updated 100hz
  typedef struct VESC_status1
  {  // 64bits
    int rpm : 32;
    int position : 16;
    int motorCurrent : 16;
  } VESC_status1;

  // slightly less updated 50hz
  typedef struct VESC_status2
  {  // 32bits
    int tachometer : 32;
    unsigned adc : 12;
    unsigned flimit : 1;
    unsigned rlimit : 1;
  } __attribute__((packed)) VESC_status2;

  // even less updated 10hz
  typedef struct VESC_status3
  {  // 60bits
    float wattHours;
    int inCurrent : 16;
    unsigned voltage : 12;
  } VESC_status3;
  typedef struct VESC_status4
  {  // 29bits
    unsigned tempMotor : 12;
    unsigned tempPCB : 12;
    unsigned faultCode : 3;
    unsigned state : 2;
    unsigned encoderIndex : 1;
  } VESC_status4;

  void init_socketCAN(char *ifname);
  void processMessages();

public:
  typedef enum
  {
    CAN_PACKET_SET_DUTY = 0,
    CAN_PACKET_SET_CURRENT,
    CAN_PACKET_SET_CURRENT_BRAKE,
    CAN_PACKET_SET_RPM,
    CAN_PACKET_SET_POS,
    CAN_PACKET_FILL_RX_BUFFER,
    CAN_PACKET_FILL_RX_BUFFER_LONG,
    CAN_PACKET_PROCESS_RX_BUFFER,
    CAN_PACKET_PROCESS_SHORT_BUFFER,
    CAN_PACKET_STATUS,  // 9

    CAN_PACKET_STATUS1,
    CAN_PACKET_STATUS2,
    CAN_PACKET_STATUS3,
    CAN_PACKET_STATUS4,

    CAN_PACKET_CONFIG,
    CAN_PACKET_CONTROL
  } CAN_PACKET_ID;

  typedef enum
  {
    CONTROL_MODE_DUTY = 0,
    CONTROL_MODE_SPEED,
    CONTROL_MODE_CURRENT,
    CONTROL_MODE_CURRENT_BRAKE,
    CONTROL_MODE_POS,
    CONTROL_MODE_NONE,
    CONTROL_MODE_CUSTOM,
  } mc_control_mode;

  typedef enum
  {
    RESET_WATT_HOURS = 0,
  } CONFIG_FIELD;

  typedef struct custom_config_data
  {
    union
    {
      float setpointf;
      int setpointi;
    };
    uint8_t config_enum;
  } custom_config_data;

  typedef struct custom_control
  {
    union
    {
      float setpointf;
      int setpointi;
    };
    unsigned control_mode : 4;
  } custom_control;

  Vesc(char *interface, uint8_t controllerID, std::string name);
  Vesc(char *interface, uint8_t controllerID, uint32_t quirks, std::string name);
  void setPoint(mc_control_mode mode, float setpoint, uint index = 0);
  void setDuty(float dutyCycle) override;
  void setCurrent(float current) override;
  void setCurrentBrake(float current);
  void setRpm(float rpm) override;
  void setPos(float pos);
  void setCustom(float setpoint) override;
  void setCustom(float setpoint, uint index) override;

  void enable();
  void disable();

  int getRpm() override;
  float getCurrent() override;
  float getDutyCycle();
  float getPosition();
  int getTachometer() override;
  float getWattHours();
  float getInCurrent() override;
  float getVin() override;
  float getTempMotor();
  float getTempPCB();
  mc_fault_code getFaultCode();
  mc_state getState();

  bool getForLimit() override;
  bool getRevLimit() override;
  int getADC() override;
  std::string name;
  void resetWattHours();
  bool encoderIndexFound() override;
  bool isAlive() override;
};
