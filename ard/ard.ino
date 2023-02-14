////////////////////////////////////////////////////////////////
// Task 1
// Arduino Nano Resource Manager
// Name / Date
////////////////////////////////////////////////////////////////

#include <Wire.h>
#include "hal.h"
#include "Concurrent.h"

// State definitions
#define NoTriggerNoDemand    0   // idle state
#define TriggerIamMaster     1   // we are master after a successful claim
#define IamSlave             2   // we are slave after other device has made a successful claim

// Global variable for storing the Resource Manager state.
int rmstate;

// Global variable for storing the trigger value
bool trigger;

// Global variable for storing the orientation
char orientation;

int *X;
int *Y;
int *Z;

Concurrent mainTask;


//==============================================================
// resourceManager()
//
// This function implements the resource manager FSM based on
//    a) the current state (rmstate)
//    b) the 'trigger' variable
//    c) the state of DEMAND determined using the
//       HAL_demandRequest() function
// The LED should be illuminated in the IamMaster state and
// should be extinguished for all other states.
//==============================================================
void resourceManager()
{
  // TO-DO: Implement your optimized FSM
  switch (rmstate)
  {
    case NoTriggerNoDemand:
      if (!trigger & !HAL_demandRequest()) {
        rmstate = NoTriggerNoDemand;
      }
      else if (!trigger) {
        rmstate = IamSlave;
      }
      else {
        rmstate = TriggerIamMaster;
      }
      break;

    case TriggerIamMaster:
      if (!trigger) {
        rmstate = NoTriggerNoDemand;
      }
      else {
        rmstate = TriggerIamMaster;
      }
      break;

    case IamSlave:
      if (HAL_demandRequest()) {
        rmstate = IamSlave;
      }
      else {
        rmstate = NoTriggerNoDemand;
      }
      break;
  }
}

void setup() {
  // For debugging purposes
  Serial.begin(115200);

  // Initialize the GPIO
  // TO-DO
  HAL_gpioInit();

  // Initialize rmstate and trigger to sensible values.
  // TO-DO
  rmstate = NoTriggerNoDemand;
  trigger = false;

  // Initialize the orientation variable to a sensible value
  orientation = 'F';


  // Initialize the I2C Wire object. This should be configured
  // as a slave. It should have event-driven function(s). As the
  // Arduino needs to access the I2C bus to turn-on the accelerometer
  // consideration needs to be given to where that can be safely
  // carried out.
  Wire.begin(8);
  Wire.onRequest(requestEvent);


  // Start the main task code module
  mainTask.setModuleDelay(1);
  mainTask.setRunning(true);
}


void loop() {

  // The Resource Manager has the highest priority and must
  // run every time the loop() function is executed.
  // TO-DO
  resourceManager();

  // The main task requires some form of concurrent code module
  // capable of implementing an FSM containing 4 states.
  // This will require a module delay of 1ms.
  // State 0: Set a delay time of 11 ms, then move immediately to state 1
  // State 1: Stay here until the delay time has elapsed,
  //          activate the trigger, then move to state 2
  // State 2: Stay here until the Resource Manager reaches the IamMaster
  //          state. Then move to state 3.
  // State 3: Read the accelerometer data, disable the trigger, process the
  //          data, and return back to state 0.

  if (mainTask.actionTask())
  {
    static int state = 0;
    static unsigned long reftime;

    switch (state)
    {
      case 0:
        reftime = millis();
        state = 1;
        break;
      case 1:
        if ((millis() - reftime) >= 11) {
          trigger = true;
          state = 2;
        }
        break;
      case 2:
        if (rmstate == TriggerIamMaster) state = 3;
        else state = 2;
        break;
      case 3:
        HAL_readAccelerometer( *X, *Y, *Z);
        orientation = HAL_determineOrientation(orientation, X, Y, Z);
        trigger = false;
        state = 0;
        break;
    }
  }
}

// Place any event-driven functions here....
// TO-DO
void requestEvent() {
  Wire.write("Accelerometer = ");
}
