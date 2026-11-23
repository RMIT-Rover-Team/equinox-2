#pragma once

//Abstract class
class GenericMotor {
  private:

  public:
    virtual void calibrate() = 0;
    virtual double getPosition() = 0;
    virtual void setPosition(double pos) = 0;
    virtual void estop() = 0;
    virtual void stop() = 0;
    virtual void tick() = 0;
    virtual void setVelocity(double vel) = 0;
};