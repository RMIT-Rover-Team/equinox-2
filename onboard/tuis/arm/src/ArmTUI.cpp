#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <termios.h>
#include "CommsThread.h"

void print_controls();
void print_status(ArmPayloadState &state, std::array<double, 6> encoded_positions, int selected_device);
char getch();
void move_terminal_cursor_up(int lines);


int main() {
    // what tui library are we using
    // otherwise were using ncurses

    // spdlog::set_level(spdlog::level::off);
    // std::cout << "-----  Controls -----\n  W: Excavator up\n  S: Excavator down\n  A: Bucket up\n  D: Bucket down\n  Space: Stop\n  Esc: Estop\n" << std::endl;

    print_controls();
    WrappedCANBus can_bus("vcan0");
    CommsThread worker(can_bus);
    worker.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // wait for myactuators to start up

    int selected_device = 0;

    while (true) {
        // std::cout << "\rExcavator velocity: " << worker.get_excavator_velocity() << " Bucket velocity: " << worker.get_bucket_velocity() << std::flush;
        {
            std::lock_guard<std::mutex> lock(worker.m_target_state);
            print_status(worker.target_state, worker.encoded_motor_positions, selected_device);
        }
        // non blocking
        char c = getch();
        if (c == 0) std::this_thread::sleep_for(std::chrono::milliseconds(50));
        else if (c == 27) worker.estop();
        else if (c == 'w') {
            --selected_device;
            if (selected_device < 0) selected_device = 7;
        } else if (c == 's') {
            ++selected_device;
            if (selected_device > 7) selected_device = 0;
        } else if (c == 'a' || c == 'd') {
            std::lock_guard<std::mutex> lock(worker.m_target_state);
    
            if (selected_device == 6) worker.target_state.grip_velocity += (c == 'a' ? -1 : 1);
            else if (selected_device == 7) worker.target_state.poke_velocity += (c == 'a' ? -1 : 1);
            else {
                // device 0-5
                worker.target_state.motor_positions.at(selected_device) += (c == 'a' ? -0.1 : 0.1);
            }
            
            worker.cv.notify_one();
        }
    }

    return 0;
}


void print_controls() {
    // 5 newlines at the end so there is room for print_status to overwrite
    std::cout << "----- Controls -----\n  W/S: Select device\n A/D: Control device\n  Space: Stop\n  Esc: Estop\n\n\n\n\n\n\n\n\n" << std::endl;
}

void print_status(ArmPayloadState &state, std::array<double, 6> encoded_positions, int selected_device) {
    move_terminal_cursor_up(8);
    // std::cout << "\033[2J\033[1;1H";
    for (int i = 0; i < 6; ++i) {
        std::cout << "Motor " << i+1
                  << (selected_device == i ? "  << " : "     ")
                  << (state.motor_positions.at(i) >= 0.0 ? " " : "")
                  << std::fixed << std::setprecision(2) << state.motor_positions.at(i)
                  << (selected_device == i ? "° >> " : "°    ")
                  << "(" << encoded_positions.at(i) << "°)" << "        \n";
    }

    std::cout << "Grip vel"
              << (selected_device == 6 ? " << " : "    ")
              << (state.grip_velocity >= 0 ? " " : "")
              << state.grip_velocity
              << (selected_device == 6 ? " >> " : "    ") << "        \n";

    std::cout << "Poke vel"
              << (selected_device == 7 ? " << " : "    ")
              << (state.poke_velocity >= 0 ? " " : "")
              << state.poke_velocity
              << (selected_device == 7 ? " >> " : "    ") << "        " << std::endl;
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

    // old.c_cc[VMIN] = 1;     // block until char to read
    old.c_cc[VMIN] = 0;     // dont block if no char to read
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