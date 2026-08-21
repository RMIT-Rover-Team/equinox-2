#include "ControllerState.h"

ControllerState::ControllerState() {
    // Drive movement: Sticks correspond to respective rover side
    right_stick = 0.0;
    left_stick  = 0.0;

    /* Drive mode:
     *      RB pressed      = LOCKED_VELOCITY
     *      RB not pressed  = UNLOCKED_VELOCITY
     */
    right_bumper = false;

    // Camera movement
    dpad_up = false;
    dpad_down = false;
    dpad_left = false;
    dpad_right = false;
};

ControllerState::ControllerState(MessageStructure& message) {
    this->right_stick   = message.getFloat("right_stick");
    this->left_stick    = message.getFloat("left_stick");

    // Boolean values: check if != 0
    this->right_bumper  = message.getInt("right_bumper") != 0;
    this->dpad_up       = message.getInt("dpad_up") != 0;
    this->dpad_down     = message.getInt("dpad_down") != 0;
    this->dpad_left     = message.getInt("dpad_left") != 0;
    this->dpad_right    = message.getInt("dpad_right") != 0;

    std::cout << "Left Speed: " << this->left_stick << " Right Speed: " << this->right_stick << std::endl;
}

void ControllerState::printLabel(std::string label) {
    std::cout << std::setw(15) << label;
}

void ControllerState::dump() {
    std::cout << "Xbox controller state:\n\n";
    std::cout << std::left << std::setfill(' ') << std::boolalpha;

    printLabel("Right stick:");
    std::cout << right_stick << "\n";;

    printLabel("Left Stick:");
    std::cout << left_stick << "\n";

    printLabel("Right bumper:");
    std::cout << right_bumper << "\n";

    printLabel("DPAD:");
    if (dpad_up)
        std::cout << "UP ";
    if (dpad_down)
        std::cout << "DOWN ";
    if (dpad_left)
        std::cout << "LEFT";
    if (dpad_right)
        std::cout << "RIGHT";
    std::cout << "\n";
}
