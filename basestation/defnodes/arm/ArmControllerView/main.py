#!/usr/bin/env python3

# Import your channel type(s)
#from DEFCOM import ChannelTransactional
#from DEFCOM import ChannelMulticast
from DEFCOM import ChannelLossyCast
import tkinter as tk
import threading


Buttons = {
    (50,120):'L_Stick_X',
    (50,100):'L_Stick_Y',
    (430,270):'R_Stick_X',
    (430,250):'R_Stick_Y',

    (50,230):'D_Pad_Up',
    (50,270):'D_Pad_Down',
    (50,250):'D_Pad_Left',
    (50,290):'D_Pad_Right',
    
    (430,145):'X_Button',
    (430,95):'Y_Button',
    (430,215):'A_Button',
    (430,125):'B_Button',

    (50,50):'L_Bumper',
    (430,45):'R_Bumper',
    (50,70):'L_Trigger',
    (430,70):'R_Trigger',

    (430,293):'R_Stick_Button',
    (50,145):'L_Stick_Button',
    (50,75):'L_Trigger'
}

Mappings = {
    0:{
        'L_Stick_X':'Base Joint',
        'L_Stick_Y':'Shoulder',
        'R_Stick_X':'Twist',
        'R_Stick_Y':'Elbow',
        'D_Pad_Up':'Wrist Up',
        'D_Pad_Down':'Wrist Down',
        'D_Pad_Left':'Wrist Left',
        'D_Pad_Right':'Wrist Right',
        'X_Button':'To IK Mode',
        'B_Button':'To Preset'
        
    },
    1:{
        'R_Stick_X':'X Value',
        'R_Stick_Y':'Y Value',
        'L_Stick_Y':'Z Value',
        'Y_Button':'To Joint Mode',
        'B_Button':'To Preset',
        'L_Trigger':'Hold (Angle)'
    },
    2:{
        'X_Button':'To IK Mode',
        'Y_Button':'To Joint Mode',
        'D_Pad_Up':'Raised',
        'D_Pad_Down':'Stowed'
    }
}


class app:
    def __init__(self):
        # Set up the channels
        self.XBInputChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/XBox.defcom")
        self.ArmStatusChannel = ChannelLossyCast.LossyCastSubscriber("COMFILES/ArmControlStatus.defcom")

        self.root = tk.Tk()

        self.canvas = tk.Canvas(self.root, width=502, height=342)
        self.canvas.pack()

        # Load background image
        self.bg = tk.PhotoImage(file="Controller.png")
        self.canvas.create_image(0, 0, image=self.bg, anchor="nw")

        # Create labels at predefined coordinates
        self.labels = {}
        for i in Buttons:
            x,y = i
            if Buttons[i] in Mappings[0]:
                l = Mappings[0][Buttons[i]]
            else:
                l = ''
            print('Button',Buttons[i])
            self.labels[Buttons[i]] = self.canvas.create_text(x,y,text=l, fill="white", font=("Arial", 12, "bold"))

        # Add the mode label
        self.mode = 0
        self.modeLabel = self.canvas.create_text(100,330,text='Mode: ', fill="black", font=("Arial", 12, "bold"))

        self.modeThread = threading.Thread(target=self.getArmState)
        self.modeThread.start()

    def getArmState(self):
        while True:
            message = self.ArmStatusChannel.subscribe()
            self.mode = message.getChar('inputState')[0]
        
    def run(self):
        while True:
            message = self.XBInputChannel.subscribe()
            #print(self.labels)

            # For each of the buttons/axis, check if they have been pressed
            for b in ['A_Button','B_Button','X_Button','Y_Button','L_Bumper','R_Bumper','D_Pad_Up','D_Pad_Down','D_Pad_Left','D_Pad_Right','L_Stick_Button','R_Stick_Button']:
                if message.getChar(b)[0]:
                    self.canvas.itemconfig(self.labels[b], fill='red')
                else:
                    self.canvas.itemconfig(self.labels[b], fill='gray')

            for a in ['L_Stick_X','L_Stick_Y','R_Stick_X','R_Stick_Y', 'L_Trigger', 'R_Trigger']:
                if abs(message.getFloat(a)) > 0.05:
                    self.canvas.itemconfig(self.labels[a], fill='red')
                else:
                    self.canvas.itemconfig(self.labels[a], fill='gray')

            if self.mode == 1:
                self.canvas.itemconfig(self.modeLabel, text='Mode: IK Control')
            elif self.mode == 2:
                self.canvas.itemconfig(self.modeLabel, text='Mode: Preset Control')
            else:
                self.canvas.itemconfig(self.modeLabel, text='Mode: Joint Control')

            # Update labels for given mode
            for i in self.labels:
                modemap = Mappings[self.mode]
                if i in modemap:
                    self.canvas.itemconfig(self.labels[i], text=modemap[i])
                else:
                    self.canvas.itemconfig(self.labels[i], text='')

            self.root.update()

if __name__ == "__main__":
    a = app()
    a.run()