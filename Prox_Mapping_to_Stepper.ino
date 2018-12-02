

#include <MPR121.h>
#include <Wire.h>
#include "Compiler_Errors.h"

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper = AccelStepper(4,9,10,11,12,true); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5


// mapping and filter definitions
#define LOW_DIFF 0
#define HIGH_DIFF 50
#define filterWeight 0.3f // 0.0f to 1.0f - higher value = more smoothing
float lastProx = 0;

// the electrode to monitor
#define ELECTRODE 1


  
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  analogWrite(LED_BUILTIN, 0); //LED is inverted, this turns on LED throughout whole program

  Serial.begin(9600);
  //while(!Serial);  
  if(!MPR121.begin(0x5A)){ 
    Serial.println("error setting up MPR121");  
    switch(MPR121.getError()){
      case NO_ERROR:
        Serial.println("no error");
        break;  
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;      
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;      
    }
    while(1);
  }

  stepper.setMaxSpeed(400);
  
  // slow down some of the MPR121 baseline filtering to avoid 
  // filtering out slow hand movements
  MPR121.setRegister(MPR121_NHDF, 0x01); //noise half delta (falling)
  MPR121.setRegister(MPR121_FDLF, 0x3F); //filter delay limit (falling)
}

void loop() {

  // update all of the data from the MPR121
  MPR121.updateAll();

  // read the difference between the measured baseline and the measured continuous data
  int reading = MPR121.getBaselineData(ELECTRODE)-MPR121.getFilteredData(ELECTRODE);
  
  // print out the reading value for debug
  

  // constrain the reading between our low and high mapping values
  unsigned int prox = constrain(reading, LOW_DIFF, HIGH_DIFF);
  
//  // implement a simple (IIR lowpass) smoothing filter
  lastProx = (filterWeight*lastProx) + ((1-filterWeight)*(float)prox);
//
//  // map the LOW_DIFF..HIGH_DIFF range to 0..255 (8-bit resolution for analogWrite)
  int thisOutput =map(lastProx,LOW_DIFF,HIGH_DIFF,120,400);
  thisOutput = constrain(thisOutput, 120, 400); //constrains determined by testing apropriate motor speeds
  Serial.println(thisOutput); 

  stepper.setSpeed(-thisOutput); //reversed to spin gears in correct direction 
  stepper.runSpeed();
    // step 1/100 of a revolution:
  

}
