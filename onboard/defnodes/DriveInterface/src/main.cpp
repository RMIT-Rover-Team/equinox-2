#include <iostream>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
#include "torque_handler.hpp"
#include <thread>
#include <chrono>
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>
using std::cout;
using std::endl;

class DriveNode {
    private:
        TorqueHandler* handler = nullptr;
        int lastEnable = 1;
        int lastDriveMode = 0;

    public:
        DriveNode(char* interfaceName){
            std::cout << "Drive Node Created" << std::endl;

            this->handler = new TorqueHandler(interfaceName);
            this->handler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);

            this->handler->enable();
            this->handler->setSpeed(0,0);

        }

        ~DriveNode(){
            delete this->handler;
        }

        void telemThread(){
            LossyCastPublisher telem("COMFILES/DriveStatus.defcom");
            std:cout << "Drive Telem Thread Started" << std::endl;

            while (true){
                OdomReading myReading = this->handler->getOdom();
                
                MessageStructure msg = telem.getNewMessageObject();
                msg.setFloat("LeftSpeed", myReading.leftSpeed);
                msg.setFloat("RightSpeed", myReading.rightSpeed);
                msg.setFloat("LeftPosition", myReading.leftPos);
                msg.setFloat("RightPosition", myReading.rightPos);

                // The flags
                msg.setChar("DriveMode", lastDriveMode);
                msg.setChar("Enabled", lastEnable);

                // Calculate the slip alarm
                // [TODO]

                telem.publish(msg);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            
        }

        void run(){
            LossyCastSubscriber control("COMFILES/Drive.defcom");
            std::cout << "Drive Control Thread Started" << std::endl;


            // Start Telem
            std::thread telemThread = std::thread(&DriveNode::telemThread, this);

            while (true){
                MessageStructure msg = control.subscribe();

                if (msg.getChar("EStop")){
                    std::cout << "EStop!" << std::endl;
                    this->handler->estop();
                }
                else if (msg.getChar("Calibrate")){
                    std::cout << "Calibrate!" << std::endl;
                    this->handler->calibrate();
                }

                // Handle swapping
                if ((msg.getChar("Enable") == 1) && (lastEnable == 0)){
                    std::cout << "Enable!" << std::endl;
                    this->handler->enable();
                    lastEnable = 1;
                }
                else if ((msg.getChar("Enable") == 0) && (lastEnable == 1)){
                    std::cout << "Disable!" << std::endl;
                    this->handler->disable();
                    lastEnable = 0;
                }

                // Set the Drive Mode
                lastDriveMode = msg.getChar("DriveMode");
                this->handler->setMode((TorqueDriveMode)lastDriveMode);

                // Set the Speed
                this->handler->setSpeed(msg.getFloat("LeftSpeed"), msg.getFloat("RightSpeed"));

            }
        }

};


int main(int argc, char *argv[]) {
    if (argc < 2){
        std::cout << "Usage:" << argv[0] << " <CanbusInterface>" << std::endl;
        return 1;
    }

    std::cout << "Starting Drive on " << argv[1] << std::endl;
    DriveNode node(argv[1]);
    node.run();
    
    return 0;
}