//This is a template for using the library as a slave on arduino with RATCAN

#include "RoverCanSlave.h"
#include "EQUCAN.h"
#include <SPI.h>

#define S0 9
#define S1 10
// #define S2 11
struct CANFrame128;
const int myID = 0x8;
int volatile counter = 0; 
int currentstate = 0;

enum 
{
  idleState,
  estop1,
  estop2,
  estopOn
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Global objects to be initialised
EQUCAN* mycan = nullptr;
RoverCanSlave* myslave = nullptr;


//Function Handlers to be hooked into, Make sure to match the templates exactly
void cut_power(uint8_t estopState, uint8_t position)//function uses setmotor position command 0x02
{
  //set pin 8 high

  cli();//clear interrupts
 
  if(currentstate == idleState)//statemachine
  {
    if(estopState == 1)
    {
      TCNT1  = 0;//initialize counter value to 0
      currentstate = estop1;
    //  Serial.println("estop msg1 received");
    }
  }
  else if(currentstate == estop1)
  {
    if(estopState == 2)
    {
      TCNT1 = 0;//initialize counter value to 0
      currentstate = estop2;
    //  Serial.println("estop msg2 received");
    }
  }
  else if(currentstate == estop2)
  {
    if(estopState == 3)
    {
      TCNT1 = 0;//initialize counter value to 0
      currentstate = estopOn; 
      myslave->broadcastDP(2,1,1);
      digitalWrite(8,HIGH);//shuts power to rover
    //  Serial.println("estop msg3 received shutting power to rover");
    }
  }
  else if(currentstate == estopOn)
  {
    TCNT1  = 0;//initialize counter value to 0
    digitalWrite(8,LOW);//shuts power to rover
    currentstate = idleState;
  }
  else 
  {
    currentstate = idleState;// something has gone wrong
  }

  sei();//enable interrupts

}


double read_cell(int stream_id, int cell_id){

  double cellNew[12] ={};
  double cell[12] ={};
  double cellscaler[12] = {1.0102948191,1.012852625,1.007750158,1.020604082,1.010294819,1.007750158,1.010294819,1.010294819,1.007750158,1.005218026,1.005218026,1.010294819};
  
  if (cell_id >= 0 && cell_id < 8) {
    if (cell_id == 0) {
      PORTB &= ~(1 << PB6);  // low
   //   digitalWrite(S2, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S0, LOW);     
    } else if (cell_id == 1) {
      PORTB &= ~(1 << PB6);  // low
   //  digitalWrite(S2, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S0, HIGH);      
    } else if (cell_id == 2) {
      PORTB &= ~(1 << PB6);  // low
    //  digitalWrite(S2, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S0, LOW);
    } else if (cell_id == 3) {
      PORTB &= ~(1 << PB6);  // low
    //  digitalWrite(S2, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S0, HIGH);
    } else if (cell_id == 4) {
      PORTB |=  (1 << PB6);  // high
    //  digitalWrite(S2, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S0, LOW);
    } else if (cell_id == 5) {
      PORTB |=  (1 << PB6);  // high
    //  digitalWrite(S2, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S0, HIGH);
    } else if (cell_id == 6) {
      PORTB |=  (1 << PB6);  // high
    //  digitalWrite(S2, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S0, LOW);
    } else if (cell_id == 7) {
      PORTB |=  (1 << PB6);  // high
    //  digitalWrite(S2, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S0, HIGH);
      
    } 
    cell[cell_id] = analogRead(A0);
  } 
    
  else if (cell_id == 8) {
    cell[cell_id] = analogRead(A1);
  }
  else if (cell_id == 9) {
    cell[cell_id] = analogRead(A2);
  }
  else if (cell_id == 10){
    cell[cell_id] = analogRead(A3);
  }
  else if (cell_id == 11){
    cell[cell_id] = analogRead(A4);
  }
  else  {
    return -1.0;
  }
      
  cellNew[cell_id] = 2.0 * (cell[cell_id] * 5.0 * cellscaler[cell_id]) / 1024.0;
  
  return cellNew[cell_id];
}


//Init stuff
void setup() {

  DDRB &= ~(1 << PB7);  // input
  //DDRB  |=  (1 << PB7);  // output
  //PORTB |=  (1 << PB7);  // high
  //PORTB &= ~(1 << PB7);  // low

  delay(100); //ensures PB7 is set at input
  pinMode(A0, INPUT);  // cells 0-7
  pinMode(A1, INPUT);  // cell 8
  pinMode(A2, INPUT);  // cell 9
  pinMode(A3, INPUT);  // cell 10
  pinMode(A4, INPUT);  // cell 11

  DDRB  |=  (1 << PB6);  // output s2
  // pinMode(S2, OUTPUT);//s2
  pinMode(S1, OUTPUT);//s1
  pinMode(S0, OUTPUT);//s0
 
  digitalWrite(8, LOW);
  pinMode(8, OUTPUT);// control for PB0 (controls a relay to cut power to rover)
  digitalWrite(8, LOW);

  cli();

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624*2;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();
  
  // Serial.begin(9600);
  
  //If you need to configure SPI pins, do it here (Pico Specific)
  SPI.begin();

  //Start the EQUCAN
  // Serial.println("Init EQUCAN\n");
  mycan = new EQUCAN();

  //Start the Slave
  Serial.println("Prepare Slave\n");
  myslave = new RoverCanSlave(myID, mycan);

  //Add the hooks where needed
  myslave->handleRequestDataPoint = &read_cell;
  myslave->handleSetMotorPosition = &cut_power;

  // Serial.println("Ready!\n");
}


ISR(TIMER1_COMPA_vect){ //4 interrupts every 4 seconds

// Serial.println("estop timed out");
// mutexlock
  currentstate = 0; // clear our counter
//   //Serial.println("message correctly recieved");
  //Serial.println(cell_id);
//  mutexunlock

}

void loop(){
  myslave->noBlockListenTick();
}