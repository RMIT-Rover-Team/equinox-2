#include <stdint.h>
#include "RATCAN.h"
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

void RATCAN::dumpBuffer(){
  #ifdef RATCAN_DEBUG
  //Serial.println("Queue:");
  for (int index = 0; index < RATCAN_Queue_Size; index++){
    if (msgQueueFlags[index]){
      //Serial.println(((index==msgQueuePointer) ? "-> " : "  ") + String(index) + " " + String(msgQueue[index].can_id));
    }
    else {
      //Serial.println(((index==msgQueuePointer) ? "-> " : "  ") + String(index) + " EMPTY");
    }
    
  }
  #endif
}

RATCAN::RATCAN(){
    while (0 != CAN.begin(RATCAN_Speed)){
      //Serial.println("CAN init fail, retry...");
      delay(100);
    }

    memset(this->myFrame.data,0,RATCAN_Datalen);

    //Clear the queue
    for (int index = 0; index < RATCAN_Queue_Size; index++){
      msgQueueFlags[index] = 0;
    }
    msgQueuePointer = 0;

    #ifdef RATCAN_USEISR
    attachInterrupt(digitalPinToInterrupt(RATCAN_SPI_INT_PIN),CAN_RECV_ISR, FALLING);
    #endif

    #ifdef RATCAN_DEBUG
    //Serial.println("CANBus Queue Size: " + String(sizeof(msgQueue)));
    #endif

    dumpBuffer();
}

CANFrame RATCAN::readMSG(){
    dumpBuffer();
    //First check if there is an existing message in the buffer
    //We do this by scanning from the write pointer until we find something unread
    int foundread = -1;
    for (int index = 0; index < RATCAN_Queue_Size; index++){
      int mindex = (index+msgQueuePointer) % RATCAN_Queue_Size;
      if (msgQueueFlags[mindex]){
        foundread = mindex;
      }
    }

    //If we find something, read and return
    if (foundread >= 0){
      msgQueueFlags[foundread] = 0;
      #ifdef RATCAN_DEBUG
      //Serial.println("Found Existing");
      #endif
      return msgQueue[foundread];
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

CANFrame RATCAN::readMSGFrom(uint32_t Id, uint32_t Mask){
  return this->readMSGFrom(Id,Mask,0);
}

CANFrame RATCAN::readMSGFrom(uint32_t Id, uint32_t Mask, uint32_t timeout_ms){
  dumpBuffer();
  //First check if there is an existing message in the buffer
  //We do this by scanning from the write pointer until we find something unread that matches the ID
  int foundread = -1;
  for (int index = 0; index < RATCAN_Queue_Size; index++){
    int mindex = (index+msgQueuePointer) % RATCAN_Queue_Size;
    if (msgQueueFlags[mindex]){
      if ((msgQueue[mindex].can_id & Mask) == Id){
        foundread = mindex;
      }
      
    }
  }

  //If we find something, read and return
  if (foundread >= 0){
    msgQueueFlags[foundread] = 0;
    #ifdef RATCAN_DEBUG
    //Serial.println("Found Existing");
    #endif
    return msgQueue[foundread];
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
    //Otherwise we add it to the queue
    else {
      #ifdef RATCAN_DEBUG
      //Serial.println("Add unread MSG");
      #endif
      msgQueue[msgQueuePointer] = newFrame;
      msgQueueFlags[msgQueuePointer] = 1;
      msgQueuePointer += 1;

      if (msgQueuePointer >= RATCAN_Queue_Size){
        msgQueuePointer = 0;
      }
      dumpBuffer();
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

int RATCAN::writeMSG(uint32_t IdAndFlags, const char* data, uint8_t length){
    //Serial.println("Write to " + String(IdAndFlags));
    CAN.sendMsgBuf(IdAndFlags, 0, 0, CANFD::len2dlc(length), (const uint8_t*)data, true);
    //CAN_CLR_RECV();
    return 1;
}

int RATCAN::writeMSG_RTR(uint32_t IdAndFlags, const char* data, uint8_t length){
    //Serial.println("Write to " + String(IdAndFlags));
    CAN.sendMsgBuf(IdAndFlags, 0, 1, CANFD::len2dlc(length), (const uint8_t*)data, true);
    //CAN_CLR_RECV();
    return 1;
}

bool RATCAN::available(){
  //Check if a message exists
  for (int index = 0; index < RATCAN_Queue_Size; index++){
    if (msgQueueFlags[index]){
      return true;
    }
  }

  #ifdef RATCAN_USEISR
  return RECV_FLAG;
  #else
  return (CAN_MSGAVAIL == CAN.checkReceive()) ? 1: 0;
  #endif
}

bool RATCAN::availableFrom(uint32_t Id, uint32_t Mask){
  //Check if a message exists
  for (int index = 0; index < RATCAN_Queue_Size; index++){
    if (msgQueueFlags[index]){
      if ((msgQueue[index].can_id & Mask) == Id){
        return true;
      }
    }
  }


  //Otherwise read one into the buffer
  //CANFrame myFFrame;
  
  #ifdef RATCAN_USEISR
  if (RECV_FLAG){

    CAN.readMsgBuf(&(myFrame.can_dlc), myFrame.data);
    myFrame.can_id = CAN.getCanId();
    RECV_FLAG = false;

    msgQueue[msgQueuePointer] = myFrame;
    msgQueueFlags[msgQueuePointer] = 1;
    msgQueuePointer += 1;

    if (msgQueuePointer >= RATCAN_Queue_Size){
      msgQueuePointer = 0;
    }
  }
  
  #else

  if (CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&(myFrame.can_dlc), (byte*)myFrame.data);
    myFrame.can_id = CAN.getCanId();

    msgQueue[msgQueuePointer] = myFrame;
    msgQueueFlags[msgQueuePointer] = 1;
    msgQueuePointer += 1;

    if (msgQueuePointer >= RATCAN_Queue_Size){
      msgQueuePointer = 0;
    }
  }
  
  #endif


  return false;
}

void RATCAN::clearBuffer(){
  //Mark all as available
  for (int index = 0; index < RATCAN_Queue_Size; index++){
    msgQueueFlags[index] = false;
  }
}

RATCAN::~RATCAN(){
    // This is never called in an embedded environment
}