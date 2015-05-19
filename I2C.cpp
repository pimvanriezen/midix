#include <Arduino.h>
#include <Wire.h>
#include "I2C.h"

// ==========================================================================
// CLASS I2CHandler
// ==========================================================================
I2CHandler::I2CHandler (void) {
    begun = false;
}

// --------------------------------------------------------------------------
I2CHandler::~I2CHandler (void) {
}

// --------------------------------------------------------------------------
void I2CHandler::begin (void) {
    if (! begun) {
        Wire.begin();
        begun = true;

        #ifdef BOARD_MEGA    
          #ifndef CPU_FREQ
            #define CPU_FREQ 16000000L
          #endif
          #define I2C_FREQ 400000L
          TWBR = ((CPU_FREQ / I2C_FREQ) - 16) / 2;
        #else
          Wire.setClock(400000);
        #endif
        
        for (uint8_t dev=1; dev<128; ++dev) {
            Wire.beginTransmission (dev);
            if (Wire.endTransmission() == 0) {
                Serial.print ("I2C device found at address 0x");
                Serial.println (dev, HEX);
            }
        }
    }
}

// --------------------------------------------------------------------------
void I2CHandler::set (uint8_t dev, uint8_t addr, uint8_t val) {
    if (! begun) begin();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    Wire.write (val);
    Wire.endTransmission();
}

// --------------------------------------------------------------------------
uint8_t I2CHandler::get (uint8_t dev, uint8_t addr) {
    if (! begun) begin();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    Wire.endTransmission();
    Wire.requestFrom (dev, (uint8_t) 1);
    uint8_t res = Wire.read();
    return res;
}

// --------------------------------------------------------------------------
uint16_t I2CHandler::getWord (uint8_t dev, uint8_t addr) {
    if (! begun) begin();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    Wire.endTransmission();
    Wire.requestFrom (dev, (uint8_t) 2);
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    return lo | (hi << 8);
}

I2CHandler I2C;
