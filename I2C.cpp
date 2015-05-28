#include <Arduino.h>
#include <Wire.h>
#include "I2C.h"

#define PIN_I2CRST 53

// ==========================================================================
// CLASS I2CHandler
// ==========================================================================
I2CHandler::I2CHandler (void) {
    begun = false;
    error = 0;
}

// --------------------------------------------------------------------------
I2CHandler::~I2CHandler (void) {
}

// --------------------------------------------------------------------------
void I2CHandler::resetBus() {
    Serial.write ("I2C reset\r\n");
    digitalWrite (PIN_I2CRST, LOW);
    digitalWrite (20, LOW);
    digitalWrite (21, LOW);
    delayMicroseconds (50);
    digitalWrite (PIN_I2CRST, HIGH);    
    Wire.begin();
}

// --------------------------------------------------------------------------
void I2CHandler::begin (void) {
    if (! begun) {
        pinMode (PIN_I2CRST, OUTPUT);
        digitalWrite (PIN_I2CRST, HIGH);
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
                Serial.print (F("I2C device found at address 0x"));
                Serial.println (dev, HEX);
            }
        }
    }
}

// --------------------------------------------------------------------------
void I2CHandler::beginWrite (uint8_t dev, uint16_t addr) {
    if (! begun) begin();
    noInterrupts();
    Wire.beginTransmission (dev);
    Wire.write ((addr & 0xef00) >> 8);
    Wire.write (addr & 0xff);
}

// --------------------------------------------------------------------------
void I2CHandler::write (uint8_t byte) {
    Wire.write (byte);
}

// --------------------------------------------------------------------------
void I2CHandler::endWrite (void) {
    error = Wire.endTransmission();
}

// --------------------------------------------------------------------------
void I2CHandler::set (uint8_t dev, uint8_t addr, uint8_t val) {
    if (! begun) begin();
    noInterrupts();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    Wire.write (val);
    error = Wire.endTransmission();
    interrupts();
}

// --------------------------------------------------------------------------
uint8_t I2CHandler::get (uint8_t dev, uint8_t addr) {
    digitalWrite (20, LOW);
    digitalWrite (21, LOW);
    delayMicroseconds (3);
    uint8_t res = 0;
    if (! begun) begin();
    noInterrupts();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    error = Wire.endTransmission();
    if (! error) {
        Wire.requestFrom (dev, (uint8_t) 1);
        res = Wire.read();
    }
    interrupts();
    return res;
}

// --------------------------------------------------------------------------
uint16_t I2CHandler::getWord (uint8_t dev, uint8_t addr) {
    digitalWrite (20, LOW);
    digitalWrite (21, LOW);
    delayMicroseconds (3);
    uint8_t lo = 0, hi = 0;
    if (! begun) begin();
    Wire.beginTransmission (dev);
    Wire.write (addr);
    error = Wire.endTransmission();
    if (! error) {
        Wire.requestFrom (dev, (uint8_t) 2);
        lo = Wire.read();
        hi = Wire.read();
    }
    return lo | (hi << 8);
}

// --------------------------------------------------------------------------
bool I2CHandler::getBytes (uint8_t dev, uint16_t addr,
                           uint8_t *into, uint8_t sz) {
    uint8_t bytes[4];
    if (! begun) begin();
    Wire.beginTransmission (dev);
    Wire.write ((addr & 0xfe) >> 8);
    Wire.write (addr & 0xff);
    error = Wire.endTransmission();
    if (error) return false;
    Wire.requestFrom (dev, sz);
    for (uint8_t i=0; i<sz; ++i) {
        into[i] = Wire.read();
    }
    return true;
}

I2CHandler I2C;
