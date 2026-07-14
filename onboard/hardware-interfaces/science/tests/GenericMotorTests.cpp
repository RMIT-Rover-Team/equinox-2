#include "GenericMotor.h"
#include <gtest/gtest.h>
#include <SocketCanWrapper.h>

TEST(GenericMotorTest, SetRPM) {
    class MockGenericMotor : public GenericMotor {
        void can_monitor_loop() override {
            // Override to prevent actual CAN communication during tests
            current_rpm = target_rpm; // Simulate immediate RPM change
        }
    }
    
    #define CAN_BUS_NAME "can0"
    WrappedCANBus can_bus(CAN_BUS_NAME);
    RoverCanMaster can_master(can_bus, GroupId::PAYLOAD);
    MockGenericMotor motor(0x01, can_master);

    motor.setRpm(100);
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Allow time for the CAN worker to process
    ASSERT_EQ(motor.getRpm(), 100);

    motor.setRpm(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Allow time for the CAN worker to process
    ASSERT_EQ(motor.getRpm(), 0);

    motor.setRpm(-50); // Negative RPM should be treated as 0
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Allow time for the CAN worker to process
    ASSERT_EQ(motor.getRpm(), 0);
}