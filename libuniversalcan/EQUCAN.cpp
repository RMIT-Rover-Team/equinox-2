#include <stdint.h>
#include "EQUCAN.h"
#include "CANBus-SOLDERED.h"
#include <SPI.h>

volatile bool RECV_FLAG = false;
void CAN_RECV_ISR(){
  RECV_FLAG = true;
}

void CAN_CLR_RECV(){
  RECV_FLAG = false;
}

CANBus CAN(RATCAN_SPI_CS_PIN);


EQUCAN::EQUCAN(){
    while (0 != CAN.begin(RATCAN_Speed)){
      //Serial.println("CAN init fail, retry...");
      delay(100);
    }

    memset(this->myFrame.data,0,RATCAN_Datalen);


    #ifdef RATCAN_USEISR
    attachInterrupt(digitalPinToInterrupt(RATCAN_SPI_INT_PIN),CAN_RECV_ISR, FALLING);
    #endif

    #ifdef RATCAN_DEBUG
    //Serial.println("CANBus Queue Size: " + String(sizeof(msgQueue)));
    #endif

}

CANFrame EQUCAN::readMSG(){
    if (frameQueued){
        frameQueued = 0;
        return this->myFrame;
    }
    
    //Otherwise we wait for a message  
    #ifdef RATCAN_USEISR
    while (!RECV_FLAG){
      #ifdef RATCAN_DEBUG
      //Serial.println("Await msg int"); 
      #endif
      //delay(100);
    }
    CAN_CLR_RECV();
    #else
    while (CAN_MSGAVAIL != CAN.checkReceive()){
      #ifdef RATCAN_DEBUG
      //Serial.println("Await msg poll"); 
      #endif
      //delay(100);
    }
    #endif

    //Always call readMsgBuf before getCanId
    CAN.readMsgBuf(&(this->myFrame.can_dlc), (byte*)this->myFrame.data);
    this->myFrame.can_id = CAN.getCanId();

    return this->myFrame;
}

CANFrame EQUCAN::readMSGFrom(uint32_t Id, uint32_t Mask){
  return this->readMSGFrom(Id,Mask,0);
}

CANFrame EQUCAN::readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms){
  if (frameQueued){
    if ((this->myFrame.can_id & Mask) == (Id & Mask)){
      frameQueued = 0;
      return this->myFrame;
    }
  }

  CANFrame newFrame;
  uint32_t start = millis();
  uint32_t now;
  bool done = false; 
  while (true){
    Serial.println("I am stuck here");
    #ifdef RATCAN_USEISR
    while (!RECV_FLAG){
      #ifdef RATCAN_DEBUG
      //Serial.println("Await msg int"); 
      #endif
      //delay(100);
    }
    CAN_CLR_RECV();
    #else
    while (CAN_MSGAVAIL != CAN.checkReceive()){
      #ifdef RATCAN_DEBUG
      Serial.println("Await msg Poll");
      #endif
      // This is if no messages are being sent on the bus and we are stuck in this loop 
      now = millis();
      if (((now - start) > timeout_ms) && (timeout_ms != 0)){
        Serial.println("Timed out");
        newFrame.can_id = 0;
        memset(newFrame.data,0,8);
        done = true; 
        break;
      }
    }
    #endif

    if(done){
      break; 
    }

    //Always call readMsgBuf before getCanId
    CAN.readMsgBuf(&(newFrame.can_dlc), (byte*)newFrame.data);
    newFrame.can_id = CAN.getCanId();

    #ifdef RATCAN_DEBUG
      Serial.print("Got Response from "); //Serial.print(newFrame.can_id);
      Serial.print("Awaiting ID: "); //Serial.println(Id);
    #endif
    //delay(100);

    //If it is what we want, we break
    if ((newFrame.can_id & Mask)== Id){
      break;
    }

    // There also needs to be a time out here. It reaches this when messages are being received on the bus but it is not from whow e want
    now = millis();
    if (((now - start) > timeout_ms) && (timeout_ms != 0)){
        Serial.println("Timed out");
        newFrame.can_id = 0;
        memset(newFrame.data,0,8);
        done = true; 
        break;
    }

  }
  return newFrame;
}

int EQUCAN::writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length){
    //Serial.println("Write to " + String(IdAndFlags));
    CAN.sendMsgBuf(IdAndFlags, 0, 0, CANFD::len2dlc(length), (const uint8_t*)data, true);
    //CAN_CLR_RECV();
    return 1;
}

int EQUCAN::writeMSG_RTR(uint32_t IdAndFlags, const char* data, uint8_t length){
    //Serial.println("Write to " + String(IdAndFlags));
    CAN.sendMsgBuf(IdAndFlags, 0, 1, CANFD::len2dlc(length), (const uint8_t*)data, true);
    //CAN_CLR_RECV();
    return 1;
}

bool EQUCAN::available(){
  //Check if a message exists

  if (frameQueued){
    return true;
  }

  #ifdef RATCAN_USEISR
  return RECV_FLAG;
  #else
  return (CAN_MSGAVAIL == CAN.checkReceive()) ? 1: 0;
  #endif
}

bool EQUCAN::availableFrom(uint32_t Id, uint32_t Mask){
  //Otherwise read one into the buffer
  //CANFrame myFFrame;
  
  #ifdef RATCAN_USEISR
  if (RECV_FLAG){
    CAN.readMsgBuf(&(myFrame.can_dlc), myFrame.data);
    myFrame.can_id = CAN.getCanId();
    RECV_FLAG = false;

    if ((myFrame.can_id & Mask) == (Id & Mask)){
      frameQueued = 1;
      return true;
    }
  }
  
  #else

  if (CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&(myFrame.can_dlc), (byte*)myFrame.data);
    myFrame.can_id = CAN.getCanId();

    

    if ((myFrame.can_id & Mask) == (Id & Mask)){
      //Serial.println("Avail check");
      frameQueued = 1;
      return true;
    }

  }
  
  #endif


  return false;
}

void EQUCAN::clearBuffer(){
  frameQueued = 0;
}

EQUCAN::~EQUCAN(){
    // This is never called in an embedded environment
}