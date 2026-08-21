#include "torque_handler.hpp"
#include <iostream>

#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>

void setNonCanonicalMode(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);           // Save old settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);         // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore old settings
    }
}


int main(int argc, char** argv){
    if (argc < 2){
        printf("Usage: %s <canInterface>\n",argv[0]);
        exit(0);
    }

    //In test mode the can interface is ignored
    TorqueHandler* myHandler = new TorqueHandler(argv[1]);
    printf("Init handler\n");

    printf("Calibrate\n");
    //myHandler->calibrate();
    //std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("Enable\n");
    myHandler->enable();
    myHandler->setMode(TorqueDriveMode::UNLOCKED_VELOCITY);

    int run = 1;
    float leftsp = 0;
    float rightsp = 0;
    char c;
    setNonCanonicalMode(1);
    
    while (run){
        c = getchar();
        if (c == 27) { // ESC
            if (getchar() == '[') {
                switch(getchar()) {
                    case 'A': 
                        std::cout << "Faster\n"; 
                        leftsp += 5;
                        rightsp += 5;
                        break;
                    case 'B': 
                        std::cout << "Slower\n";
                        leftsp -= 5;
                        rightsp -= 5;
                        break;

                    case 'C': 
                        std::cout << "Right\n"; 
                        leftsp += 5;
                        rightsp -= 5;
                        break;
                    case 'D': 
                        std::cout << "Left\n";
                        leftsp -= 5;
                        rightsp += 5;
                        break;
                }
                myHandler->setSpeed(leftsp,rightsp);
            }
        }
        else if (c == 'q'){
            run = 0;
        }
        else if (c == ' '){
            leftsp = 0;
            rightsp = 0;
            myHandler->setSpeed(leftsp,rightsp);

        }

        //Show current speeds
        OdomReading myReading = myHandler->getOdom();
        printf("Current Odom - Speed: %f %f Pos: %f %f\n",myReading.leftSpeed, myReading.rightSpeed, myReading.leftPos, myReading.rightPos);
    }
    setNonCanonicalMode(0);

    printf("Clean Up\n");
    delete myHandler;
    printf("Done\n");
}
