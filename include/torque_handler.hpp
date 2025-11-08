#pragma once
#include "motor_wrapper.hpp"
#include <thread>

/* 
RMIT Rover

Authors:
* [Electrical] Kaelan Grainger

*/


//Function Flags
// IN TestMode, ODrive motors are replaced with FakeMotor objects that print the output of each operation. No CANBUs communications occur
//#define TorqueHandler_TestMode 1
//When 1, A linear math model is used, when undefined a PID model is used
//#define TorqueHandler_LinearMathOrPID 1

//Config for the math files
#define TorqueHandler_PID_Kp 0.0001
#define TorqueHandler_PID_Kd 0.00000
#define TorqueHandler_PID_Ki 0.01

#define TorqueHandler_MinTorque -2
#define TorqueHandler_MaxTorque 2

#define TorqueHandler_LinearResponse 0.01

//Motor Definitions
#define MotorCount 6
#define MotorsPerSide 3
const int MotorIDs[MotorCount] = {1,2,3,4,5,6};
const int LeftMotorIndexes[MotorsPerSide] = {0,1,2}; //These are indexes of the above motor IDs
const int RightMotorIndexes[MotorsPerSide] = {3,4,5}; //These are indexes of the above motor IDs

const int MotorDirs[MotorCount] = {-1,-1,-1,1,1,1};

#define ControlLoopTimestep 0.1


enum TorqueDriveMode : int
{
    UNLOCKED_VELOCITY = 0x0, // Independent wheels (Regular driving) - Just like BOB
    UNLOCKED_TORQUE   = 0x1, // Independent wheels with torque modulation (Offroad)
    LOCKED_VELOCITY   = 0x2, // Traction control in velocity mode, wheels are matched to slowest, best for sand / uneven terrain
};

//Semophore for the Motor wrappers
enum MotorAction: int
{
    CONTROL_LOOP = 0x0,
    CALIBRATE    = 0x1,
    ENABLE       = 0x2,
    DISABLE      = 0x3,
    ESTOP        = 0x4
};

//The return type from Left and Right odom
typedef struct 
{
    float leftPos;
    float rightPos;
    float leftSpeed;
    float rightSpeed;
} OdomReading;


#ifdef TorqueHandler_TestMode
    #include "fake_wrapper.hpp"
#else
    #include "odrive_wrapper.hpp"
    #include "odrive_control.hpp"
#endif

#ifdef TorqueHandler_EnableTorqueMode

#endif

#include "control_calc.hpp"
#ifdef TorqueHandler_LinearMathOrPID
#include "linear_calc.hpp"
#else
#include "pid_calc.hpp"
#endif

class TorqueHandler{
    private:
        #ifndef TorqueHandler_TestMode
            ODriveController* myController;
        #endif
        volatile float leftSpeed, rightSpeed;

        //Async handler for PID
        std::thread controlThread;

        //The motors on the rover and their Mathematical Models
        MotorWrapper* myMotors[MotorCount];
        Control_Calc* myModels[MotorCount]; 

        //Flag for Async instructions
        volatile int actionFlag;

        //The drive mode
        TorqueDriveMode mode;

        //The private control loop stuff
        void controlLoop();
        void handlePID();
        int loopControlFlag;

    public:
        TorqueHandler(const char* canInterface);

        void setSpeed(float leftSpeed, float rightSpeed);
        void calibrate();
        void enable();
        void disable();
        void estop();

        void setMode(TorqueDriveMode mode);

        OdomReading getOdom();

        ~TorqueHandler();
};
