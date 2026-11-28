#include <ExcavatorNode.h>
#include <ChannelLossycast.hpp>

ExcavatorNode::ExcavatorNode(RoverCanMaster &m)
  : excavator_(m) {}

void ExcavatorNode::run() {
  LossyCastSubscriber control("COMFILES/Excavator.defcom");

  while (true) {
    MessageStructure msg = control.subscribe();

    if (msg.getChar("EStop")){
      std::cout << "EStop!" << std::endl;
      excavator_.estop();
    }
    const bool magnet = msg.getChar("Magnet") == 1;
    if (magnet != last_magnet_) {
        std::cout << (magnet ? "Magnet on" : "Magnet off") << std::endl;
        excavator_.paver_magnet.set_status(magnet);
        last_magnet_ = magnet;
    }

    int16_t boom_velocity = msg.getInt("BoomVelocity");
    int16_t rock_bucket_velocity = msg.getInt("RockBucketVelocity");
    int16_t sand_bucket_velocity = msg.getInt("SandBucketVelocity");

    excavator_.boom_tilt.set_velocity(boom_velocity);
    excavator_.rock_bucket_tilt.set_velocity(rock_bucket_velocity);
    excavator_.sand_bucket_tilt.set_velocity(sand_bucket_velocity);
  }
}
