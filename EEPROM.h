#ifndef _EEPROM_H
#define _EEPROM_H 1

#include <stdint.h>
#include <stddef.h>
#include "I2C.h"

/// Storage for a 64-byte page of EEPROM data
class EEPROMPage
{
public:
    uint8_t      data[64];
};

/// EEPROM has not been properly formatted
#define EEPROM_INVALID 0x00

/// EEPROM is formatted for a different version of the application
#define EEPROM_DIFFVERSION 0x01

/// EEPROM is formatted and valid
#define EEPROM_VALID 0x02

/// Handler for 24AAXXX I2C EEPROM
class EEPROMHandler
{
public:
                 /// Constructor
                 EEPROMHandler (void);
                 
                 /// Destructor
                ~EEPROMHandler (void);
                
                 /// Set the I2C address for the chip
    void         setAddress (uint8_t i2caddr);
    
                 /// Check whether the data on the EEPROM is currently
                 /// formatted as valid for the application.
                 /// \param appid 32-bit unique id for the application.
                 /// \param vmajor Major version number
                 /// \param vminor Minor version number
    uint8_t      valid (uint32_t appid, uint8_t vmajor, uint8_t vminor);
    
                 /// Initialize the EEPROM for the application.
                 /// \param appid 32-bit unique id for the application.
                 /// \param vmajor Major version number
                 /// \param vminor Minor version number
                 /// \param numpages Number of pages on the chip
    void         init (uint32_t appid, uint8_t vmajor, uint8_t vminor,
                       uint16_t numpages);
                       
                 /// Load a 64-byte page from the EEPROM
    EEPROMPage  *load (uint16_t page);
    
                 /// Write data (up to 64 bytes) to the EEPROM
    void         write (uint16_t page, void *data, uint8_t sz);
    
protected:
    uint8_t      i2cid;
    uint16_t     maxpage;
};

/// Global instance
extern EEPROMHandler EEPROM;

#endif
