#ifndef _PORT_H
#define _PORT_H 1

#define SVC_PORT 0x02
#define EV_PORT_IRQ 0x01
#define EV_PORT_TIMER 0x02

#define SVC_INPORT 0x03
#define EV_INPORT_CHANGE 0x01

#define EV_INPUT_ENCODER_LEFT 0x30
#define EV_INPUT_ENCODER_RIGHT 0x31
#define EV_INPUT_BUTTON_DOWN 0x32
#define EV_INPUT_BUTTON_UP 0x33
#define EV_INPUT_ABUTTON_DOWN 0x34
#define EV_INPUT_ABUTTON_UP 0x35

#include "EventQueue.h"

/// Class representing an MCP23017 port extender hanging off the
/// I2C bus.
class PortBus
{
public:
                 /// Constructor.
                 /// \param id The i2c bus address of the extender.
                 PortBus (uint8_t id, uint8_t irq0, uint8_t irq1);
                 
                 /// Dummy destructor.
                ~PortBus (void);

                 /// Set up a pin's in/out mode.
                 /// \param pin The id of the pin (0-15).
                 /// \param mode The pin mode (INPUT or OUTPUT).
    void         pinMode (uint8_t pin, uint8_t mode);

                 /// Initializes the extender to the desired
                 /// configuration.
    void         begin (void);
    
                 /// Dump state.
    void         dump (void);
    
                 /// Current value of a pin. Valid for both input
                 /// and output ports.
    uint8_t      pinValue (uint8_t pin);
    
                 /// Set the output state of an output pin.
                 /// \param pin The pin number (0-15).
                 /// \param time Time to keep the pin HIGH. If 0,
                 ///             pin is set to LOW. If 255,
                 ///             it is kept HIGH indefinitely.
                 ///             Other values keep the pin HIGH
                 ///             for the provided number of ticks.
    void         pinOut (uint8_t pin, uint8_t time);
    
                 /// Handles a timer tick.
    void         decreaseTimers (void);
    
                 /// Handles an interrupt.
    void         handleInterrupt (uint8_t bank, bool isirq=true);

    uint8_t      i2cid; /// i2c bus address of the extender.
    uint8_t      irq0pin;
    uint8_t      irq1pin;

protected:
    uint8_t      pinvalues[16]; /// Current values
    uint16_t     pinmodes; /// Current in/out modes.
    uint8_t      activeTimers; /// Number of active timers
};

#define ABUTTON_DOWN_LOW 0x0
#define ABUTTON_DOWN_HIGH 0x1

struct AButton
{
    uint8_t      pin;
    uint8_t      type:1;
    uint8_t      state:7;
    uint16_t     threshold;
};

/// A service combining several Port Extenders and physical arduino
/// pins, handling inbound interrupts, turning them into events for the
/// InPortService.
class PortService : public EventReceiver
{
public:
                 PortService (void);
                ~PortService (void);
    
    void         addBus (uint8_t i2cid, uint8_t irq0, uint8_t irq1);
    void         addOutput (uint16_t);
    void         addInput (uint16_t);
    void         addAnalogButton (uint8_t pin, uint8_t type, 
                                  uint16_t threshold);
    void         begin (void);
    void         dump (void);
    
    void         pinOut (uint16_t id, uint8_t timeval);
    void         handleEvent (eventtype tp, eventid id, uint16_t X,
                              uint8_t Y, uint8_t Z);

protected:
    PortBus     *findPortBus (uint8_t i2cid);
    PortBus     *ports[8];
    uint8_t      numports;
    AButton      abuttons[8];
    uint8_t      numbuttons;
};

extern PortService Port;

struct Button {
    uint16_t     id;
    uint8_t      state:1;
    uint8_t      prevstate:1;
};

struct Encoder {
    uint16_t     id1;
    uint16_t     id2;
    uint8_t      state:2;
    uint8_t      prevstate:2;
    uint8_t      prevprevstate:2;
};

class InPortService : public EventReceiver
{
public:
                 InPortService (void);
                ~InPortService (void);
                
    void         setReceiver (serviceid);
    void         addButton (uint16_t);
    void         addEncoder (uint16_t, uint16_t);
    void         begin (void);

    void         handleEncoder (uint8_t);
    void         handleEvent (eventtype tp, eventid id, uint16_t X,
                              uint8_t Y, uint8_t Z);

    uint8_t      outsvc;
    
protected:
    Button       buttons[24];
    Encoder      encoders[16];
    uint8_t      numbuttons;
    uint8_t      numencoders;
};

extern InPortService InPort;

class OutPortHandler
{
public:
                 OutPortHandler (void);
                ~OutPortHandler (void);
    
    void         add (uint16_t);
    void         on (uint8_t);
    void         off (uint8_t);
    void         flash (uint8_t, uint8_t);

protected:
    uint16_t     outputs[32];
    uint8_t      count;
};

extern OutPortHandler OutPort;

#endif
