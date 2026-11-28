#pragma once
#include <Excavator.h>
#include <universalcan/RoverCanMaster.h>

class ExcavatorNode {
private:
  Excavator excavator_;
  bool last_magnet_;
public:
  ExcavatorNode(RoverCanMaster &m);
  ~ExcavatorNode() = default;
  void run();


};
