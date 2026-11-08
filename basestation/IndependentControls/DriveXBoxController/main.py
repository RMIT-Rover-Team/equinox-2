#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast            #check with Nick 
from DEFCOM import ChannelLossyCast
import pygame             #code captures events rather than state so using pygame instead of inputs 
import time

if __name__ == "__main__":
    #print("Hello World")s
    publisher = ChannelLossyCast.LossyCastPublisher("COMFILES/DriveController.defcom")
    
    #pygame working 
    pygame.init()
    pygame.joystick.init()
    joystick = pygame.joystick.Joystick(0) #taking inputs from plugged in controller , assuming only 1 
    joystick.init()
    while True:
        #getting iputs from controller
        pygame.event.pump() #update events
        left_raw=joystick.get_axis(1) #axis numbers would only be clear during testing , please update as observed. Please fill 'n'
        right_raw=joystick.get_axis(4)
        bumper_raw=joystick.get_button(10)

        hat = joystick.get_hat(0)

        dpad_up    = 1 if hat[1] == 1 else 0
        dpad_down  = 1 if hat[1] == -1 else 0
        dpad_left  = 1 if hat[0] == -1 else 0
        dpad_right = 1 if hat[0] == 1 else 0


        #Float converstions to int as per defcom file , please remove if dtype is changed to float in defcom file
        left_stick=int(left_raw*-500) #assuming axis values are between -1.0 and 1.0
        right_stick=int(right_raw*-500)
        right_bumper=int(bumper_raw)
        dpad_up=int(dpad_up)
        dpad_down=int(dpad_down)
        dpad_left=int(dpad_left) 
        dpad_right=int(dpad_right)
    



        

        message= publisher.getNewMessageObject()
        message.setFloat("left_stick", left_stick)
        message.setFloat("right_stick", right_stick)
        message.setInt("right_bumper", right_bumper)
        message.setInt("dpad_up", dpad_up)
        message.setInt("dpad_down", dpad_down)
        message.setInt("dpad_left", dpad_left)
        message.setInt("dpad_right", dpad_right)

        print("Sending controller state: left_stick={}, right_stick={}, right_bumper={}, dpad_up={}, dpad_down={}, dpad_left={}, dpad_right={}".format(left_stick, right_stick, right_bumper, dpad_up, dpad_down, dpad_left, dpad_right))
        publisher.publish(message)

        time.sleep(0.2)
