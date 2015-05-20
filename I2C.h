#ifndef _I2C_H
#define _I2C_H 1

/// Utility class for interacting with I2C devices.
class I2CHandler
{
public:
                 /// Boring constructor.
                 I2CHandler (void);
                 
                 /// Boring destructor.
                ~I2CHandler (void);
                
                 /// Sets up I2C. Uses a 400KHz clock.
    void         begin (void);
    
                 /// Resets the I2C bus.
    void         resetBus (void);
    
                 /// Write a byte to the I2C bus.
                 /// \param dev The I2C device
                 /// \param addr The register address to set.
                 /// \param val The value to write.
    void         set (uint8_t dev, uint8_t addr, uint8_t val);
    
                 /// Read a byte from the I2C bus.
                 /// \param dev The I2C device
                 /// \param addr The register address to read.
    uint8_t      get (uint8_t dev, uint8_t addr);
    
                 /// Read a word from the I2C bus.
                 /// \param dev The I2C device
                 /// \param addr The register address to read.
    uint16_t     getWord (uint8_t dev, uint8_t addr);

    bool         error;
    
protected:
    bool         begun;
};

extern I2CHandler I2C;

#endif
