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
    
                 /// Start a write operation that is going to span
                 /// multiple bytes (using write()).
    void         beginWrite (uint8_t dev, uint16_t addr);
    
                 /// Write a byte to the stream opened with beginWrite().
    void         write (uint8_t byte);
    
                 /// End the write stream started with beginWrite().
    void         endWrite (void);
    
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
    
                 /// Read a stream of bytes from the I2C bus using
                 /// 16 bit addressing.
    bool         getBytes (uint8_t dev, uint16_t addr, 
                           uint8_t *into, uint8_t sz);

    uint8_t      error;
    
protected:
    bool         begun;
};

extern I2CHandler I2C;

#endif
