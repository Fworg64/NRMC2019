#ifndef __MOCK_VESC_H_
#define __MOCK_VESC_H_

#include <vesc_control/ivesc.h>
#include <gmock/gmock.h>

class MockVesc : public iVesc
{
public:
  //    MOCK_METHOD0 (~mockVesc);
  MOCK_METHOD1(setRpm, void(float rpm));
  MOCK_METHOD1(setCurrent, void(float current));
  MOCK_METHOD0(getCurrent, float(void));
  MOCK_METHOD0(getRpm, int(void));
  MOCK_METHOD0(getADC, int(void));
  MOCK_METHOD0(getForLimit, bool(void));
  MOCK_METHOD0(getRevLimit, bool(void));
  MOCK_METHOD1(setDuty, void(float));
  MOCK_METHOD1(setCustom, void(float));
  MOCK_METHOD2(setCustom, void(float, uint));
  MOCK_METHOD0(getInCurrent, float(void));
  MOCK_METHOD0(getTachometer, int(void));
  MOCK_METHOD0(getVin, float(void));
  MOCK_METHOD0(encoderIndexFound, bool(void));
  MOCK_METHOD0(isAlive, bool(void));
};

#endif
