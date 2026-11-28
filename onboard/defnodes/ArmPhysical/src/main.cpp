#include <iostream>

//Include your channel type(s)
#include <ChannelLossycast.hpp> 
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>

#include "GenericMotor.h"
#include "MyActuatorMotor.h"

#include "SocketCanWrapper.h"

#define SANITY_MOVE_ANGLE 15
#define SANITY_DECAY_TIME 10

#include <thread>
#include <chrono>
#include <math.h>
//#include <ChannelMulticast.hpp>
//#include <ChannelTransactional.hpp>
using std::cout;
using std::endl;

const double jointScales[6] = {
    -1,
    1,
    -1,
    -1,
    1,
    -1
};

const double jointOffsets[6] = {
    0,
    180,
    0,
    0,
    0,
    0
};

const double jointMinimums[6] = {
    -90,
    -180,
    -150,
    -15,
    -90,
    -90
};

const double jointMaximums[6] = {
    90,
    0,
    165,
    90,
    90,
    90
};

double monotonic_ms()
{
    using namespace std::chrono;

    return duration<double, std::milli>(
        steady_clock::now().time_since_epoch()
    ).count();
}

class ArmNode {
    private:
        //Create a canbus
        WrappedCANBus* myCan = nullptr;
        GenericMotor* myMotors[6];

        double lastEstimates[6];
        double lastEstimateTimes[6];

    public:
        ArmNode(char* interfaceName){
            std::cout << "Arm Node Created" << std::endl;

            //Create a canbus
            myCan = new WrappedCANBus(interfaceName);

            //The Motors 
            myMotors[0] = new MyActuatorMotor(1, myCan);
            myMotors[1] = new MyActuatorMotor(2, myCan);
            myMotors[2] = new MyActuatorMotor(3, myCan);
            myMotors[3] = new MyActuatorMotor(4, myCan);
            myMotors[4] = new MyActuatorMotor(5, myCan);
            myMotors[5] = new MyActuatorMotor(6, myCan);

            for (int i = 0; i < 6; i++){
                myMotors[i]->calibrate();
                lastEstimateTimes[i] = monotonic_ms();
            }

            for (int i = 0; i < 6; i++){
                this->lastEstimates[i] = (myMotors[i]->getPosition() - jointOffsets[i])/jointScales[i];
                printf("Initial Estimate: %lf %i\n",this->lastEstimates[i],i);
            }

            printf("Ready!\n");

        }

        ~ArmNode(){
            for (int i = 0; i < 6; i++){
                delete myMotors[i];
            }
            delete this->myCan;
        }

        void telemThread(){
            LossyCastPublisher telem("COMFILES/ArmState.defcom");
            std:cout << "Arm Telem Thread Started" << std::endl;

            while (true){
                // Get each of the joint values
                double readings[6] = {0};
    
                MessageStructure msg = telem.getNewMessageObject();
                printf("Reading!\n");
                //for (int i = 0; i < 6; i++){
                for (int i = 0; i < 6; i++){
                    readings[i] = (myMotors[i]->getPosition() - jointOffsets[i])/jointScales[i];

                    // Check if the decay time has passed
                    if (monotonic_ms() - lastEstimateTimes[i] > SANITY_DECAY_TIME){
                        printf("Decay %i %lf\n",i,readings[i]);
                        this->lastEstimates[i] = readings[i];
                        this->lastEstimateTimes[i] = monotonic_ms();
                    }
                    // otherwise check if it is sane
                    else if (fabs(readings[i] - this->lastEstimates[i]) > SANITY_MOVE_ANGLE){
                        printf("Reject %i %lf\n",i,readings[i]);
                        readings[i] = this->lastEstimates[i];
                        
                    }
                    else {
                        this->lastEstimates[i] = readings[i];
                        printf("Got %i %lf\n",i,readings[i]);
                        this->lastEstimateTimes[i] = monotonic_ms();
                    }
                    msg.setFloat("jointPositions", (readings[i]/180)*3.141592, i);
                }
                

                telem.publish(msg);

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            
        }

        void run(){
            //Connect to the channel
            LossyCastSubscriber subscriber("COMFILES/ArmControl.defcom");
            std::cout << "Arm Control Thread Started" << std::endl;


            // Start Telem
            std::thread telemThread = std::thread(&ArmNode::telemThread, this);

            
            
            while (true){
                MessageStructure myMessage = subscriber.subscribe();

                //Get all the joint angles
                for (int i = 0; i < 6; i++) {
                    double angle = 180.0*(myMessage.getFloat("jointSetValues",i)/3.141592);
                    printf("Set J%i to %lf Raw %lf\n", i, angle,myMessage.getFloat("jointSetValues",i));

                    // Get the movement type
                    char movementType = myMessage.getChar("movementType");
                    //printf("Movement type %i\n",movementType);
                    if (movementType == 'V'){
                        myMotors[i]->setVelocity(angle);
                    }
                    else if (movementType == 'P'){
                        if (angle < jointMinimums[i]){
                            angle = jointMinimums[i];
                        }
                        else if (angle > jointMaximums[i]){
                            angle = jointMaximums[i];
                        }
                        myMotors[i]->setPosition((angle * jointScales[i]) + jointOffsets[i]);
                    }
                }

            }
        }

};


int main(int argc, char *argv[]) {
    if (argc < 2){
        std::cout << "Usage:" << argv[0] << " <CanbusInterface>" << std::endl;
        return 1;
    }

    std::cout << "Starting Arm on " << argv[1] << std::endl;
    ArmNode node(argv[1]);
    node.run();
    
    return 0;
}

