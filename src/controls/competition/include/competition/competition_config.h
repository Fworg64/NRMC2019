#ifndef COMPETITION_COMPETITION_CONFIG_H
#define COMPETITION_COMPETITION_CONFIG_H

#include <string>

namespace competition
{
  enum class ControlState
  {
    manual = 0,
    assisted_autonomy,
    wait_for_start,
    start,
    check_for_apriltag,
    wait_for_localization,
    navigate_to_dig_zone_1,
    dig_1,
    finish_dig_1,
    navigate_to_hopper_1,
    dump_1,
    navigate_to_dig_zone_2,
    dig_2,
    finish_dig_2,
    navigate_to_hopper_2,
    dump_2,
    finish
  };

  std::string to_string(ControlState state);


}

#endif //COMPETITION_COMPETITION_CONFIG_H