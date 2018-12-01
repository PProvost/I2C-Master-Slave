#include <Arduino.h>
#include <TinyWireS.h>

/****************************************************************************************
 * Slave_Example.cpp ----- Sample sketch for making an I2C Slave Device
 *
 * A sample sketch that shows the basic steps necessary to making
 * an I2C slave device using Arduino's Wire library.
 *
 * Original Copyright (c) 2011, DSS Circuits, Inc.  http://www.dsscircuits.com
 * Modified by Peter Provost (c) 2018, http://www.github.com/PProvost
 *
 ***************************************************************************************/

// For Nano, SDA=A4 & SCL=A5
// Remember to pull-up both lines!

/* REGISTER MAP
Address	Register Description
0x00	Status Register
0x01	Latitude - MSB
0x02	Latitude
0x03	Latitude
0x04	Latitude - LSB
0x05	Longitude - MSB
0x06	Longitude
0x07	Longitude
0x08	Longitude - LSB
0x09	Speed - MSB
0x0A	Speed - LSB
0x0B	Mode Register
0x0C	Configuration Register
0x0D	Identification Register
 */


/* These getX DEFINES are only used so the code will compile.  They
** simply return numbers for the function calls.  Instead you would
** use regular functions and place your GPS parsing code inside them */

// Note: These values will produce 0x00 through 0x0A in the corresponding
// registers, so don't be surprised when you get see that in the master!
#define getLatitude() ((unsigned long)16909060)
#define getLongitude() ((long)84281096)
#define getSpeed() ((unsigned int)2314)
#define getSignalStatus() ((byte)0x00)

/********************************************************************/
#define SLAVE_ADDRESS 0x29 //slave address,any number from 0x01 to 0x7F
#define REG_MAP_SIZE 14
#define MAX_SENT_BYTES 3
#define INTERRUPT_PIN 2
#define IDENTIFICATION 0x0D

/********* Global  Variables  ***********/
byte registerMap[REG_MAP_SIZE];
byte registerMapTemp[REG_MAP_SIZE];
byte receivedCommands[MAX_SENT_BYTES];
byte newDataAvailable = 0;
byte useInterrupt = 1;
byte modeRegister = 0;
byte configRegister = 0;
byte zeroB_changed = 0;
byte zeroC_changed = 0;
byte zeroB_data = 0;
byte zeroC_data = 0;
long longitude = 0;
unsigned long latitude = 0;
unsigned int speedKPH = 0;
byte gpsStatus = 0;

// Forward declarations
void requestEvent();
void receiveEvent(int bytesReceived);
void toggleInterrupt();
void changeModeConfig();
void storeData();

void setup()
{
    if (useInterrupt)
    {
        pinMode(INTERRUPT_PIN, OUTPUT);
        digitalWrite(INTERRUPT_PIN, HIGH);
    }
    Wire.begin(SLAVE_ADDRESS);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    registerMap[13] = IDENTIFICATION; // ID register
}

void loop()
{
    if (zeroB_changed || zeroC_changed)
    {
        newDataAvailable = 0; //let the master know we don’t have any post correction data yet
        toggleInterrupt();
        changeModeConfig(); //call the function to make your changes
        return;             //go back to the beginning and start collecting data from scratch
    }

    latitude = getLatitude(); //returns latitude as an unsigned long
    if (zeroB_changed || zeroC_changed)
    {
        newDataAvailable = 0; //let the master know we don’t have any post correction data yet
        toggleInterrupt();
        changeModeConfig(); //call the function to make your changes

        return; //go back to the beginning and start collecting data from scratch
    }

    longitude = getLongitude(); //returns longitude a long

    if (zeroB_changed || zeroC_changed)
    {
        newDataAvailable = 0; //let the master know we don’t have any post correction data yet
        toggleInterrupt();
        changeModeConfig(); //call the function to make your changes
        return;             //go back to the beginning and start collecting data from scratch
    }

    speedKPH = getSpeed(); //returns speed as an unsigned int
    if (zeroB_changed || zeroC_changed)
    {
        newDataAvailable = 0; //let the master know we don’t have any post correction data yet
        toggleInterrupt();
        changeModeConfig(); //call the function to make your changes
        return;             //go back to the beginning and start collecting data from scratch
    }

    gpsStatus = getSignalStatus(); //returns status as a byte
    if (zeroB_changed || zeroC_changed)
    {
        newDataAvailable = 0; //let the master know we don’t have any post correction data yet
        toggleInterrupt();
        changeModeConfig(); //call the function to make your changes

        return; //go back to the beginning and start collecting data from scratch
    }

    storeData();
    newDataAvailable = 1;
    toggleInterrupt();
}

void storeData()
{
    byte *bytePointer;               //we declare a pointer as type byte
    byte arrayIndex = 1;             //we need to keep track of where we are storing data in the array
    registerMapTemp[0] = gpsStatus;  //no need to use a pointer for gpsStatus
    bytePointer = (byte *)&latitude; //latitude is 4 bytes
    for (int i = 3; i > -1; i--)
    {
        registerMapTemp[arrayIndex] = bytePointer[i]; //increment pointer to store each byte
        arrayIndex++;
    }
    bytePointer = (byte *)&longitude; //longitude is 4 bytes
    for (int i = 3; i > -1; i--)
    {
        registerMapTemp[arrayIndex] = bytePointer[i]; //increment pointer to store each byte
        arrayIndex++;
    }

    bytePointer = (byte *)&speedKPH; //speedKPH is 2 bytes
    for (int i = 1; i > -1; i--)
    {
        registerMapTemp[arrayIndex] = bytePointer[i]; //increment pointer to store each byte
        arrayIndex++;
    }

    registerMapTemp[arrayIndex] = modeRegister;
    arrayIndex++;
    registerMapTemp[arrayIndex] = configRegister;
}

void changeModeConfig()
{
    /*Put your code here to evaluate which of the registers need changing
   And how to make the changes to the given device.  For our GPS example
   It could be issuing the commands to change the baud rate, update rate,
   Datum, etc…
   We just put some basic code below to copy the data straight to the registers*/
    if (zeroB_changed)
    {
        modeRegister = zeroB_data;
        zeroB_changed = 0; //always make sure to reset the flags before returning from the function
        zeroB_data = 0;
    }
    if (zeroC_changed)
    {
        configRegister = zeroC_data;
        zeroC_changed = 0; //always make sure to reset the flags before returning from the function
        zeroC_data = 0;
    }
}

// In this example, we will pull the interrupt pin LOW until the master reads anything
// This should be called after any time you set newDataAvailable.
void toggleInterrupt()
{
    if (!useInterrupt)
    {
        return;
    } //first let’s make sure we’re using interrupts, if not just return from the function

    if (newDataAvailable) // if new data is available set the interrupt low
    {
        digitalWrite(INTERRUPT_PIN, LOW); //set pin low and return
        return;
    }

    //no new data available or data was just read so set interrupt pin high
    digitalWrite(INTERRUPT_PIN, HIGH);
}

void requestEvent()
{
    // Copy the temp map to the real one
    if (newDataAvailable)
    {
        for (int c = 0; c < (REG_MAP_SIZE - 1); c++)
        {
            registerMap[c] = registerMapTemp[c];
        }
    }

    // Prevent another request clashing with config changes
    newDataAvailable = 0;
    toggleInterrupt();

    // Set the buffer to send all 14 bytes, but offset the start
    // to the register address requested
    Wire.write(registerMap + receivedCommands[0], REG_MAP_SIZE);
}

void receiveEvent(int bytesReceived)
{
    // Copy the bytes on the wire into receivedCommands[]
    for (int a = 0; a < bytesReceived; a++)
    {
        if (a < MAX_SENT_BYTES)
        {
            receivedCommands[a] = Wire.read();
        }
        else
        {
            Wire.read(); // if we receive more data then allowed just throw it away
        }
    }

    // If we got only 1 byte, it is a register address, so we're done
    if (bytesReceived == 1 && (receivedCommands[0] < REG_MAP_SIZE))
    {
        return;
    }

    // If they try to address past the end of the register map, set the register to 0x00
    if (bytesReceived == 1 && (receivedCommands[0] >= REG_MAP_SIZE))
    {
        receivedCommands[0] = 0x00;
        return;
    }

    // Otherwise, we're being sent data to write to a register
    // Since our writable registers are sequential, we may be given
    // a starting address and multiple registers of incoming data
    
    // In this example, we only have two writable registers, so
    // if the address 0x0B they can be writing to either 0x0B by
    // itself, or they can be writing to 0x0B and 0x0C.
    switch (receivedCommands[0])
    {
    case 0x0B:
        zeroB_changed = 1;                       //this variable is a status flag to let us know we have new data in register 0x0B
        zeroB_data = receivedCommands[1]; //save the data to a separate variable
        bytesReceived--;

        if (bytesReceived == 1) //only two bytes total were sent so we’re done
        {
            return;
        }

        zeroC_changed = 1;
        zeroC_data = receivedCommands[2];
        return; //we can return here because the most bytes we can receive is three anyway
        break;

    case 0x0C:
        zeroC_changed = 1;
        zeroC_data = receivedCommands[1];
        return; //we can return here because 0x0C is the last writable register
        break;

    default:
        return; // ignore the commands and return
    }
}