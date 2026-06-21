//This is a template for using the library as a slave on arduino with RATCAN

#include "RoverCanSlave.h"
#include "RATCAN.h"
#include <SPI.h>

const int myID = 0xB;

//Global objects to be initialised
RATCAN* mycan = nullptr;
RoverCanSlave* myslave = nullptr;


//Function Handlers to be hooked into, Make sure to match the templates exactly
void my_estop_handler(){
  Serial.println("ESTOP!\n");
}

double my_pos_getter(int targetChannel){
  Serial.println("Get Pos!\n");
  return 0.12345;
}

//Init stuff
void setup() {
  Serial.begin(9600);
  
  //If you need to configure SPI pins, do it here (Pico Specific)
  SPI.begin();

  //Start the RATCAN
  Serial.println("Init RATCAN\n");
  mycan = new RATCAN();

  //Start the Slave
  Serial.println("Prepare Slave\n");
  myslave = new RoverCanSlave(myID, mycan);

  //Add the hooks where needed
  myslave->handleEStop = &my_estop_handler;
  myslave->handleGetMotorPosition = &my_pos_getter;

  Serial.println("Ready!\n");
}

void loop() {
  
  //Need to regularly call the listen function
  myslave->noBlockListenTick();
  delay(100);
  Serial.println("Tick!");

}
