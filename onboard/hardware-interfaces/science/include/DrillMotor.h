#include <stdint.h>

#include <common/CanAddress.h>
#include <universalcan/CanTypes.h>
#include <universalcan/RoverCanMaster.h>

class DrillMotor {
private:
  RoverCanMaster &m_;
  CanAddress addr_;
  bool status_;
  int16_t rpm_;

public:
  DrillMotor(CanAddress addr, RoverCanMaster &can_master);

  void set_rpm(int16_t rpm);
  int16_t get_rpm();
};
