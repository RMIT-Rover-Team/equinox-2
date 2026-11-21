#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast            #check with Nick 
from DEFCOM import ChannelLossyCast
import pygame             #code captures events rather than state so using pygame instead of inputs 
import time

if __name__ == "__main__":
    #print("Hello World")s
    publisher = ChannelLossyCast.LossyCastPublisher("COMFILES/Drive.defcom")
    
    #pygame working 
    pygame.init()
    pygame.joystick.init()
    joystick = pygame.joystick.Joystick(0) #taking inputs from plugged in controller , assuming only 1 
    joystick.init()

    Enable = 1
    Mode = 0
    while True:
        #getting iputs from controller
        pygame.event.pump() #update events
        left_raw=joystick.get_axis(1) #axis numbers would only be clear during testing , please update as observed. Please fill 'n'
        right_raw=joystick.get_axis(4)

        if abs(left_raw) < 0.1:
            left_raw = 0
        if abs(right_raw) < 0.1:
            right_raw = 0

        # A Button
        AButton = joystick.get_button(0)
        # B Button
        BButton = joystick.get_button(1)
        # X Button
        XButton = joystick.get_button(2)
        # Y Button
        YButton = joystick.get_button(3)

        hat = joystick.get_hat(0)

        dpad_up    = 1 if hat[1] == 1 else 0
        dpad_down  = 1 if hat[1] == -1 else 0
        dpad_left  = 1 if hat[0] == -1 else 0
        dpad_right = 1 if hat[0] == 1 else 0


        #Float converstions to int as per defcom file , please remove if dtype is changed to float in defcom file
        left_stick=int(left_raw*-100) #assuming axis values are between -1.0 and 1.0
        right_stick=int(right_raw*-100)


        if AButton:
            if Enable:
                print("Disabling controller")
                Enable = 0
            else:
                print("Enabling controller")
                Enable = 1

        if YButton:
            if Mode == 0:
                print("Torque Mode")
                Mode = 2
            else:
                print("Regular Mode")
                Mode = 0

        

        
    
        message= publisher.getNewMessageObject()
        message.setFloat("LeftSpeed", left_stick)
        message.setFloat("RightSpeed", right_stick)
        message.setChar("Enable", Enable.to_bytes(1))
        message.setChar("EStop", XButton.to_bytes(1))
        message.setChar("DriveMode", Mode.to_bytes(1))

        # Zero the rest
        message.setChar("Calibrate", b'\x00')



        #print("Sending controller state: left_stick={}, right_stick={}, right_bumper={}, dpad_up={}, dpad_down={}, dpad_left={}, dpad_right={}".format(left_stick, right_stick, right_bumper, dpad_up, dpad_down, dpad_left, dpad_right))
        publisher.publish(message)

        time.sleep(0.2)
