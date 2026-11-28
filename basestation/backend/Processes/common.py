
# A helper class to turn button presses into latching 1 or 0
class togglerButton:
    def __init__(self,v=0):
        self._value = v
        self._lastButton = 0

    def update(self,buttonValue):
        if (buttonValue == 1) and (self._lastButton == 0):
            if self._value == 0:
                self._value = 1
            else:
                self._value = 0

        self._lastButton = buttonValue

    def getValue(self):
        return self._value


