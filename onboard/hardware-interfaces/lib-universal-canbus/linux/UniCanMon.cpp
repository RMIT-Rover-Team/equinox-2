#include <iostream>
#include "../libuniversalcan/RoverCanMaster.h"
#include "../libuniversalcan/SocketCanWrapper.h"

void printHelp(){
    std::cout << "Commands:" << std::endl;
    std::cout << "(p)ing - Tests if device is alive" << std::endl;
    std::cout << "(e)stop - Stops a device" << std::endl;
    std::cout << "(c)alibrate - Calibrates a device" << std::endl;
    std::cout << "(sp) setpos - Sets a position" << std::endl;
    std::cout << "(ss) setspeed - Sets a speed" << std::endl;
    std::cout << "(t)oggle - Toggles switch to value" << std::endl;
    std::cout << "(gp) getpos - Gets position" << std::endl;
    std::cout << "(gs) getspeed - Gets speed" << std::endl;
    std::cout << "(h)elp - Prints this help" << std::endl;
    std::cout << "(q)uit - Exits the program" << std::endl;
}


int main(int argc, char** argv){
    if (argc != 2){
        std::cout << "Usage: " << argv[0] << " <can interface>" << std::endl;
        return 1;
    }

    WrappedCANBus myCan(argv[1]);
    printf("Init Ratcan on %s\n", argv[1]);

    RoverCanMaster myMaster(myCan, MASTER_CAN_ID);
    printf("Init Master\n");

    printHelp();

    std::string command;
    int target, motorID;
    double value;
    bool togVal;
    std::pair<ReceivedState, float> response;
    while (1){
        command = "";
        std::cout << "Enter Command: ";
        std::cin >> command;

        if (command == "h"){
            printHelp();
        }
        else if (command == "q"){
            break;
        }

        std::cout << "Enter Target (in decimal): ";
        std::cin >> target;
        

        if (command == "p"){
            printf(" -> Ping First!\n");
            printf(" -> Result: %i\n",myMaster.ping(target));
        }
        else if (command == "e"){
            printf(" -> Test ESTOP\n");
            myMaster.EStop(target);
        }
       
        else {
            //These commands need motor ID as well
            std::cout << "Enter Motor ID: ";
            std::cin >> motorID;

            if (command == "c"){
                printf(" -> Test Calibrate\n");
                myMaster.Calibrate(target, motorID);
            }

            else if (command == "gp"){
                printf(" -> Test Get Position\n");
                response = myMaster.GetMotorPosition(target, motorID);
                printf(" -> Position: %lf\n", response.second);
            }
            else if (command == "gs"){
                printf(" -> Test Get Speed\n");
                response = myMaster.GetMotorSpeed(target, motorID);
                printf(" -> Speed: %lf\n", response.second);
            }
            else if (command == "t"){
                std::cout << "Enter Toggle Value [0/1]: ";
                std::cin >> togVal;
                printf(" -> Test Toggle\n");
                myMaster.ToggleState(target, motorID, togVal);
            }
            else {
                //Get the value to set
                std::cout << "Enter Value: ";
                std::cin >> value;

                if (command == "sp"){
                    printf(" -> Test Set Position\n");
                    myMaster.SetMotorPosition(target, motorID, value);
                }
                else if (command == "ss"){
                    printf(" -> Test Set Speed\n");
                    myMaster.SetMotorSpeed(target, motorID, value);
                }
            }
            
            
        }
        
    }

}