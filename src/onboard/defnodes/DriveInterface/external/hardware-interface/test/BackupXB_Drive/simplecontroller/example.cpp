#include <iostream>
#include "SimpleController.h"

//For delay
#include <chrono>
#include <thread>  


int main(){
    SimpleController myController(16, 6);
    printf("Ready\n");

    while (true){
        myController.tick();

        //Scan for new keys
        for (int index = 0; index < 16; index++){
            if (myController.buttonAvailable(index)){
                std::cout << "Button " << index << " New State " << myController.getButton(index) << std::endl;
            }
        }

        for (int index = 0; index < 6; index++){
            if (myController.axisAvailable(index)){
                std::cout << "Axis " << index << " New State " << myController.getAxis(index) << std::endl;
            }
        }


        //Put a small delay between reads (~ 10ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}