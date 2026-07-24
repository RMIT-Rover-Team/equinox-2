#include <iostream>
#include <unistd.h>
#include <termios.h>
#include "CommsThread.h"


char getch();


int main() {
    // what tui library are we using
    // otherwise were using ncurses

    CommsThread worker;
    worker.start();

    while (true) {
        // blocks until char
        char c = getch();

        int16_t v;
        switch (c) {
            // esc key - estop
            case 27:
                worker.estop();
                break;

            // space - non-emergency stop
            case ' ':
                worker.set_excavator_velocity(0);
                worker.set_bucket_velocity(0);
                break;
            
            case 'w':
                v = worker.get_excavator_velocity();
                worker.set_excavator_velocity(v + 10);
                break;

            case 's':
                v = worker.get_excavator_velocity();
                worker.set_excavator_velocity(v - 10);
                break;

            case 'a':
                v = worker.get_bucket_velocity();
                worker.set_bucket_velocity(v + 10);
                break;

            case 'd':
                v = worker.get_bucket_velocity();
                worker.set_bucket_velocity(v - 10);
                break;

            default:
                break;
        }

        // reprint status
        std::cout << "\rExcavator velocity: " << worker.get_excavator_velocity() << " Bucket velocity: " << worker.get_bucket_velocity() << std::flush;
    }

    return 0;
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