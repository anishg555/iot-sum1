/************************************************************/
/* hal.cpp                                                  */
/*                                                          */
/* Hardware Abstraction Layer (HAL) implementation          */
/* Name / P-number / Date                                   */
/************************************************************/

#include "hal.h"


//==============================================================
// HAL_gpioInit()
//
// This function initializes the GPIO lines.
//==============================================================
void HAL_gpioInit()
{
  pinMode(HAL_LED, OUTPUT);
  pinMode(HAL_GRANTED, OUTPUT);

  DDRB &= ~DATA;
  DDRB &= ~CLOCK;
  DDRB &= ~LATCH;
}

//==============================================================
// HAL_demandRequest()
//
// This function measures the voltage on analogue pin A6
// If this exceeds 1.25V the line is considered to be a
// logic 1, there is no demand, and the function returns false.
// Otherwise the line is considered a logic '0', there is
// demand and the function returns true.
//==============================================================
bool HAL_demandRequest()
{
  return (!(analogRead(HAL_DEMAND_INPUT) >> 8));
}

//==============================================================
// HAL_grantedHigh()
//
// This function implements a safe method of asserting a
// logic 1 on the GRANTED line
//==============================================================
void HAL_grantedHigh()
{
  pinMode(HAL_GRANTED, INPUT);
}

//==============================================================
// HAL_grantedLow()
//
// This function implements a safe method of asserting a
// logic 0 on the GRANTED line
//==============================================================
void HAL_grantedLow()
{
  digitalWrite(HAL_GRANTED, LOW);
  pinMode(HAL_GRANTED, OUTPUT);
}

//==============================================================
// HAL_mpu6050Init()
//
// This function contains the initialization code to turn on
// the MPU6050.
//==============================================================
void HAL_mpu6050Init()
{
  // TO-DO
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission(true);
}

//==============================================================
// HAL_readAccelerometer()
//
// This function reads the X, Y, Z acceleration data from the
// MPU6050 and returns the values via pointers to accx, accy
// and accz respectively.
//==============================================================
void HAL_readAccelerometer(int *accX, int *accY, int *accZ)
{
  /*Sets up the accelerometer*/  
  Wire.beginTransmission(MPU_addr);
  Wire.write(XOUT_High);
  Wire.write(YOUT_High);
  Wire.write(ZOUT_High);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 8, true);

  accX = (Wire.read() << 8) | Wire.read();
  accY = (Wire.read() << 8) | Wire.read();
  accZ = (Wire.read() << 8) | Wire.read();
}

//==============================================================
// HAL_determineOrientation()
//
// This function uses the X, Y and Z components of acceleration
// via parameters: accX, accY and accZ respectively and
// determines the orientation.
// This function also takes another parameter: prevOrientation
// containing the previous orientation value. If an orientation
// value cannot be determined, the previous value will be used.
// This function returns a single character representing the
// orientation: 'F' (flat), 'b' (base-up), 'l' (left-portrait)
// 'r' (right portrait), 'L' (Landscape), 'U' (upside-down
// landscape).
//==============================================================
char HAL_determineOrientation(char prevOrientation, int accX, int accY, int accZ)
{
  char orientation = prevOrientation;
  float k = 2.0;

  if (abs(accZ) > abs(accX) && abs(accZ) >  abs(accY)) {
    if ( accX > 0) {
      orientation = 'L';     // Orientation is landscape facing the right way round
    }
    else {
      orientation = 'U';     // Board is upside down landscape
    }
  }

  /*Assigns flat/base up*/
  if (abs(accY) > abs(accX) && abs(accY) > abs(accZ)) {
    if (accX > 0) {
      orientation = 'F';     // Board is face up
    }
    else {
      orientation = 'B';     // Board is base up
    }
  }

  /*Assigns portrait left/right*/
  if (abs(accX) > abs(accY) && abs(accX) > abs(accZ)) {
    if (accX > 0) {
      orientation = 'R';     // Board is portrait right
    }
    else {
      orientation = 'l';      // Board is portrait left
    }
  }
  Serial.println(orientation);
  return orientation;
}

/*Bring the latch high and then low again*/
void HAL_toggleLatch()
{
  PORTB |= LATCH;
  PORTB &= ~LATCH;
}

/*Brings the clock high and then low again*/
void HAL_toggleClock()
{
  PORTB |= CLOCK;
  PORTB &= ~CLOCK;
}

/*Display characters on the 7 segment display*/
void writeToShiftRegister(byte value)
{
  /*Mask to compare and bit bang*/
  byte mask = B10000000;

  /*For each bit, if the mask is the same as the value, pull the data high, otherwise keep low*/
  for (int i = 7; i >= 0; i--)
  {
    if (value & mask)
    {
      PORTB |= DATA;
    }
    else
    {
      PORTB &= ~DATA;
    }
    HAL_toggleClock();
    mask = mask >> 1;             // Move to the next bit
  }
  HAL_toggleLatch();
}
