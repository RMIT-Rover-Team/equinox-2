# Excavator Hardware Interface

## Usage
The main `ExcavatorPayload` class takes in a `&RoverCANMaster`.
 - `excavator_tilt` is for moving the arm of the excavator up and down via velocity commands
 - `bucket_tilt` is for moving the bucket up and down via velocity commands
 - `teeth` is for if actuated teeth are added to the excavator. It can set via position (unfinished) as well as velocity, and has encoders (unfinished)
 - `paver_magnet` is for if a magnet is added to pick up the pavers (isn't that against the rules?).

All components can be individually stopped with the `estop()` method, or all at once from the main class

## TODO
 - Change libuniversalcan from a git subtree to a git submodule or subrepo, so that we can get the latest commits instead of needing to re-download it each time
 - Finish creating tests (do we need them?)
 - Separate out comms thread (see https://discord.com/channels/891173197951696957/891175415090130984/1522519913988620339)
 - Send heartbeat / ping repeatedly using comms thread

Waiting on RoverCanMaster full implementation (recieve packets from CANbus):

 - Recieve heartbeat / ping back from devices
 - Read teeth position from encoder