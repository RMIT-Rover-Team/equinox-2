#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>

int main()
{
    wiringPiSetupGpio();      // Use BCM numbering

    const int pin = 11;       // BCM GPIO11

    if (softPwmCreate(pin, 15, 200) != 0)
    {
        std::cerr << "Failed to create SoftPWM\n";
        return 1;
    }

    softPwmWrite(pin, 20);
    delay(100);

    softPwmWrite(pin, 15);
    return 0;
}
