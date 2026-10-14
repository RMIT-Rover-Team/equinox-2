#pragma once
#include <SDL2/SDL.h>

#include <stdbool.h>

#define BUTTON_PRESSED 1
#define BUTTON_RELEASE 0

#define AXIS_MAX 32767
#define AXIS_MIN -32768

#define AWAIT_READ 1
#define UNSET 0

class SimpleController {
    private:
        int buttonCount, axisCount;
        
        bool* buttonStates; // An array of button states (On, off)
        bool* buttonReadStates; // Contains flags for whether buttons have been read

        int* axisStates; // An array of 16 bit integers for the axis values
        bool* axisReadStates; // Contains flags for whether axis have been read


        SDL_GameController* controller;
    public:
        //Axis Count = Joystick Count x2
        SimpleController(int buttonCount, int axisCount);
        ~SimpleController();

        //Returns the state array for all buttons
        bool* getButtonArray();
        int* getAxisArray();

        //Gets the value of a specific button / axis. This also resets the unread flag
        bool getButton(int b);
        int getAxis(int a);

        //When a change is detected, the unread flag (True) is asserted
        bool buttonAvailable(int b);
        bool axisAvailable(int a);

        //Needed to check for new events, call every 10ms or so
        void tick();
};