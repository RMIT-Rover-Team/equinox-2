#pragma once

#include <stdbool.h>

#define BUTTON_PRESSED 1
#define BUTTON_RELEASE 0

#define AXIS_MAX 32767
#define AXIS_MIN -32768

#define AWAIT_READ 1
#define UNSET 0

class RemoteController {
    private:
        int buttonCount, axisCount;
        
        bool buttonStates[32]; // An array of button states (On, off)
        bool oldButtonStates[32]; // Contains flags for whether buttons have been read
        bool buttonReadStates[32]; // Contains flags for whether buttons have been read

        int axisStates[32]; // An array of 16 bit integers for the axis values
        int oldAxisStates[32]; // Contains flags for whether axis have been read
        bool axisReadStates[32]; // Contains flags for whether axis have been read

        int client;

    public:
        //Axis Count = Joystick Count x2
        RemoteController(int port);
        ~RemoteController();

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