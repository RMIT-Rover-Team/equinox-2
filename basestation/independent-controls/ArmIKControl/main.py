#!/usr/bin/env python3
from DEFCOM import ChannelLossyCast
import tkinter as tk
import math

class slider_app:
    def send_values(self,v=None):
            pos = [s.get() for s in self.posSliders]
            ori = [s.get() for s in self.oriSliders]

            print("Current pos:", pos, " Ori: ",ori)
            
            #Send a message
            message = self.channel.getNewMessageObject()
            for i in range(3):
                 message.setFloat("endPosition",pos[i]/100.0,i)
                 message.setFloat("endOrientation", math.radians(ori[i]), i)
            self.channel.publish(message)



    def __init__(self):
        #Start the multicast channel
        self.channel = ChannelLossyCast.LossyCastPublisher("COMFILES/ArmIKControl.defcom")


        #Set up tkinter
        self.root = tk.Tk()
        self.root.title("Ik input Controller")

        self.posSliders = []
        self.oriSliders = []

   

        for i in range(3):
            frame = tk.Frame(self.root)
            frame.pack(pady=5)

            label = tk.Label(frame, text=["X","Y","Z"][i])
            label.pack()

            slider = tk.Scale(
                frame,
                from_=-100,
                to=100,
                orient=tk.HORIZONTAL,
                length=300,
                command=self.send_values
            )
            slider.pack()
            self.posSliders.append(slider)

        for i in range(3):
            frame = tk.Frame(self.root)
            frame.pack(pady=5)

            label = tk.Label(frame, text=["Roll","Pitch","Yaw"][i])
            label.pack()

            slider = tk.Scale(
                frame,
                from_=0,
                to=360,
                orient=tk.HORIZONTAL,
                length=300,
                command=self.send_values
            )
            slider.pack()
            self.oriSliders.append(slider)

    

    def run(self):
        self.root.mainloop()





if __name__ == "__main__":
    a = slider_app()
    a.run()
