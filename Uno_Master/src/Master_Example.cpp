/*************************************************************************
 * Master_Example.cpp ----- Sample sketch for retrieving data from Slave_Example.pde
 * 
 * A sample sketch that shows of the master side for retrieving data
 * from the Slave_Example.pde sketch
 *
 * Original Copyright (c) 2011, DSS Circuits, Inc.  http://www.dsscircuits.com
 * Modified by Peter Provost (c) 2018, http://www.github.com/PProvost
 * 
 *************************************************************************/

// For Uno, SDA=A4 & SCL=A5
// Remember to pull-up both lines!

#include <Arduino.h>
#include <Wire.h>

byte i2cData[14];
byte test = 0;

// Struct to define the register map exposed by the Nano-Slave
struct tag_registerMap
{
  byte status;
  unsigned long latitude;
  long longitude;
  unsigned int speed;
  byte mode;
  byte config;
  byte id;
} __attribute__((packed)) registerMapStruct;

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#define SWAP_UINT32(x) (((x) >> 24) | (((x)&0x00FF0000) >> 8) | (((x)&0x0000FF00) << 8) | ((x) << 24))

void setup()
{
  Wire.begin();
  Serial.begin(115200);
}

void loop()
{
  // Tell the slave we want to start at reg 0x00
  Wire.beginTransmission(0x29);
  Wire.write(0x00);
  Wire.endTransmission();

  // Read the whole register set and write them out
  Wire.requestFrom(0x29, 14);

  Wire.readBytes((byte *)&registerMapStruct, sizeof(registerMapStruct));
  registerMapStruct.latitude = SWAP_UINT32(registerMapStruct.latitude);
  registerMapStruct.longitude = SWAP_UINT32(registerMapStruct.longitude);
  registerMapStruct.speed = SWAP_UINT16(registerMapStruct.speed);

  Serial.print(" status: ");
  Serial.print(registerMapStruct.status);

  Serial.print(" lat: ");
  Serial.print(registerMapStruct.latitude);

  Serial.print(" lon: ");
  Serial.print(registerMapStruct.longitude);

  Serial.print(" speed: ");
  Serial.print(registerMapStruct.speed);

  Serial.print(" mode: ");
  Serial.print(registerMapStruct.mode);

  Serial.print(" config: ");
  Serial.print(registerMapStruct.config);

  Serial.print(" id: ");
  Serial.print(registerMapStruct.id);

  Serial.println();

  delay(1000);

  // Write to the 0x0B and 0x0C register as one transmission
  Wire.beginTransmission(0x29);
  Wire.write(0x0B);
  Wire.write(test);
  Wire.write(test + 1);
  Wire.endTransmission();
  test++;
}
