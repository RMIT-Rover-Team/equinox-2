#include <SDL2/SDL.h>
#include <iostream>
#include "SimpleController.h"
#include <stdbool.h>

SimpleController::SimpleController(int buttonCount, int axisCount){
    this->buttonCount = buttonCount;
    this->axisCount = axisCount;

    //Set up the arrays
    buttonStates = (bool*)malloc(sizeof(bool) * buttonCount);
    buttonReadStates = (bool*)malloc(sizeof(bool) * buttonCount);

    axisStates = (int*)malloc(sizeof(int) * axisCount);
    axisReadStates = (bool*)malloc(sizeof(bool) * axisCount);

    //Initialise them
    for (int index = 0; index < buttonCount; index++){
        buttonStates[index] = BUTTON_RELEASE;
        buttonReadStates[index] = UNSET;
    }

    for (int index = 0; index < axisCount; index++){
        axisStates[index] = 0;
        axisReadStates[index] = UNSET;
    }


    //Configure the controller
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    }

    //Open the first available game controller
    controller = nullptr;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                std::cout << "Opened controller " << i << std::endl;
                break;
            } else {
                std::cerr << "Could not open gamecontroller " << i << ": " << SDL_GetError() << std::endl;
            }
        }
    }

    //Make sure it opens
    if (!controller) {
        std::cerr << "No game controller found." << std::endl;
        SDL_Quit();
    }
    std::cout << "Controller Ready" << std::endl;

}
SimpleController::~SimpleController(){
    delete buttonStates;
    delete buttonReadStates;

    delete axisStates;
    delete axisReadStates;

    SDL_GameControllerClose(controller);
    SDL_Quit();
}

bool* SimpleController::getButtonArray(){
    return this->buttonStates;
}
int* SimpleController::getAxisArray(){
    return this->axisStates;
}

bool SimpleController::getButton(int b){
    this->buttonReadStates[b] = UNSET;
    return this->buttonStates[b];
}
int SimpleController::getAxis(int a){
    this->axisReadStates[a] = UNSET;
    return this->axisStates[a];
}

bool SimpleController::buttonAvailable(int b){
    return this->buttonReadStates[b];
}
bool SimpleController::axisAvailable(int a){
    return this->axisReadStates[a];
}

void SimpleController::tick(){
    //Get any events
    SDL_Event event;

    int index;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_CONTROLLERBUTTONDOWN:
                index = (int)event.cbutton.button;
                if (index < this->buttonCount){
                    this->buttonStates[index] = BUTTON_PRESSED;
                    this->buttonReadStates[index] = AWAIT_READ;
                }
                break;

            case SDL_CONTROLLERBUTTONUP:
                index = (int)event.cbutton.button;
                if (index < this->buttonCount){
                    this->buttonStates[index] = BUTTON_RELEASE;
                    this->buttonReadStates[index] = AWAIT_READ;
                }
                break;
        

            case (SDL_JOYAXISMOTION):
                index = (int)event.caxis.axis;
                if (index < this->axisCount){
                    this->axisStates[index] = (int)event.caxis.value;
                    this->axisReadStates[index] = AWAIT_READ;
                }
                break;
        }
    }
}