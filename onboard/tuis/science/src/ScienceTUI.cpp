#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <termios.h>
#include "CommsThread.h"

void print_controls();
void print_status(SciencePayloadState &state, int selected_device);
char getch();
void move_terminal_cursor_up(int lines);


int main() {
    // what tui library are we using
    // otherwise were using ncurses

    // spdlog::set_level(spdlog::level::off);
    // std::cout << "-----  Controls -----\n  W: Excavator up\n  S: Excavator down\n  A: Bucket up\n  D: Bucket down\n  Space: Stop\n  Esc: Estop\n" << std::endl;

    print_controls();
    CommsThread worker;
    worker.start();

    int selected_device = 0;

    while (true) {
        // std::cout << "\rExcavator velocity: " << worker.get_excavator_velocity() << " Bucket velocity: " << worker.get_bucket_velocity() << std::flush;
        {
            std::lock_guard<std::mutex> lock(worker.m_target_state);
            print_status(worker.target_state, selected_device);
        }
        // blocks until char
        char c = getch();

        switch (c) {
            // esc key - estop
            case 27:
                worker.estop();
                break;

            case 'w':
                --selected_device;
                if (selected_device < 0) selected_device = 4;
                break;

            case 's':
                ++selected_device;
                if (selected_device > 4) selected_device = 0;
                break;

            case 'a':
            case 'd':
                {
                    std::lock_guard<std::mutex> lock(worker.m_target_state);

                    if (selected_device == 0) worker.target_state.heater_temperature += (c == 'a' ? -1 : 1);
                    else if (selected_device == 1) worker.target_state.drill_height += (c == 'a' ? -0.1 : 0.1);
                    else if (selected_device == 2) worker.target_state.drill_enabled = !worker.target_state.drill_enabled;
                    else if (selected_device == 3) worker.target_state.microscope_height += (c == 'a' ? -0.1 : 0.1);
                    else if (selected_device == 4) worker.target_state.microscope_swivel += (c == 'a' ? -0.1 : 0.1);
                }
                break;

            default:
                break;
        }
    }

    return 0;
}


void print_controls() {
    // 5 newlines at the end so there is room for print_status to overwrite
    std::cout << "----- Controls -----\n  W/S: Select device\n A/D: Control device\n  Space: Stop\n  Esc: Estop\n\n\n\n\n" << std::endl;
}

void print_status(SciencePayloadState &state, int selected_device) {
    move_terminal_cursor_up(5);
    std::cout << "Heater Temperature " << (selected_device == 0 ? "<< " : "   ") << std::setw(3) << state.heater_temperature << (selected_device == 0 ? " >>" : "   ") << std::endl;
    std::cout << "Drill Height       " << (selected_device == 1 ? "<< " : "   ") << std::setw(3) << state.drill_height << (selected_device == 1 ? " >>" : "   ") << std::endl;
    std::cout << "Drill Enabled      " << (selected_device == 2 ? "<< " : "   ") << std::setw(3) << (state.drill_enabled ? "ON" : "OFF") << (selected_device == 2 ? " >>" : "   ") << std::endl;
    std::cout << "Microscope Height  " << (selected_device == 3 ? "<< " : "   ") << std::setw(3) << state.microscope_height << (selected_device == 3 ? " >>" : "   ") << std::endl;
    std::cout << "Microscope Swivel  " << (selected_device == 4 ? "<< " : "   ") << std::setw(3) << state.microscope_swivel << (selected_device == 4 ? " >>" : "   ") << std::endl;
}


// https://stackoverflow.com/questions/421860/capture-characters-from-standard-input-without-waiting-for-enter-to-be-pressed
// https://www.man7.org/linux/man-pages/man3/termios.3.html
/// Gets a character in non-canonical mode (blocks until char)
char getch() {
    char buf = 0;
    struct termios old = {0};

    if (tcgetattr(0, &old) < 0) perror("Error reading termios attributes (tcsetattr())");

    old.c_lflag &= ~ICANON; // disable canonical mode (input avaliable immediately, dont wait for newline)
    old.c_lflag &= ~ECHO;   // stop echo

    old.c_cc[VMIN] = 1;     // block until char to read
    old.c_cc[VTIME] = 0;    // no timeout

    if (tcsetattr(0, TCSANOW, &old) < 0) perror("Error setting terminal to non-canonical mode (tcsetattr !ICANON)");
    if (read(0, &buf, 1) < 0) perror("Error reading char");

    // reset
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;

    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("Error resetting terminal to canonical mode (tcsetattr ICANON)");

    return buf;
}



void move_terminal_cursor_up(int lines) {
    std::cout << "\033[" << lines << "A";
}