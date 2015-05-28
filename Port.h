#ifndef _PORT_H
#define _PORT_H 1

/// Port service id and event definitions
#define SVC_PORT 0x02
#define EV_PORT_IRQ 0x01
#define EV_PORT_TIMER 0x02

/// InPort service id and event definitions
#define SVC_INPORT 0x03
#define EV_INPORT_CHANGE 0x01

/// Events that will be sent to the UserInterface
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
    uint8_t      irq0pin; /// IRQ0 id
    uint8_t      irq1pin; /// IRQ1 id

protected:
    uint8_t      pinvalues[16]; /// Current values
    uint16_t     pinmodes; /// Current in/out modes.
    uint8_t      activeTimers; /// Number of active timers
};

/// If voltage is low, button is considered pressed.
#define ABUTTON_DOWN_LOW 0x0

/// If voltage is high, buttons is considered pressed.
#define ABUTTON_DOWN_HIGH 0x1

/// Structure defining a pushbutton connected to an analogue pin.
struct AButton
{
    uint8_t      pin; ///< Analogue pin number
    uint8_t      type:1; ///< Detection type
    
                 /// Internal state data.
                 /// Counting down from 5 to 1 to debounce keydown.
                 /// Counting up from 6 to 10 to debounce keyup.
    uint8_t      state:7; 
    uint16_t     threshold; ///< Value threshold
};

/// A service combining several Port Extenders and physical arduino
/// pins, handling inbound interrupts, turning them into events for the
/// InPortService.
class PortService : public EventReceiver
{
public:
                 /// Constructor
                 PortService (void);
                 
                 /// Destructor
                ~PortService (void);
    
                 /// Add an MCP23017 extender.
                 /// \param i2cid The I2C address of the chip
                 /// \param irq0 IRQ id that triggers on bank 0
                 /// \param irq1 IRQ id that triggers on bank 1
    void         addBus (uint8_t i2cid, uint8_t irq0, uint8_t irq1);
    
                 /// Add an output port.
                 /// \param addr The address of the port, consisting
                 ///             of the i2cid of the extender it is on,
                 ///             and the pin number of the port.
    void         addOutput (uint16_t addr);
    
                 /// Add an input port.
                 /// \param addr The address of the port, consisting
                 ///             of the i2cid of the extender it is on,
                 ///             and the pin number of the port.
    void         addInput (uint16_t addr);
    
                 /// Add an analog button connected directly to the
                 /// MCU.
                 /// \param pin Analog pin number
                 /// \param type How to interpret a signal above
                 ///             the threshold.
                 /// \param threshold The threshold value.
    void         addAnalogButton (uint8_t pin, uint8_t type, 
                                  uint16_t threshold);
                                  
                 /// Initialize all resources for use.
    void         begin (void);
    
                 /// Dump state to serial port.
    void         dump (void);
    
                 /// Set an output value.
                 /// \param id The address of the port
                 /// \param timeval Time to keep it HIGH (0 to set it LOW,
                 ///                255 to keep it HIGH indefinitely).
    void         pinOut (uint16_t id, uint8_t timeval);
    
                 /// Event handler (for IRQs).
    void         handleEvent (eventtype tp, eventid id, uint16_t X,
                              uint8_t Y, uint8_t Z);

protected:
                 /// Find a PortBus instance by its i2c id.
    PortBus     *findPortBus (uint8_t i2cid);
    
    PortBus     *ports[8]; ///< Connected ports.
    uint8_t      numports; ///< Number of ports in the array.
    AButton      abuttons[8]; ///< Connected buttons.
    uint8_t      numbuttons; ///< Number of buttons in the array.
};

/// Global instance.
extern PortService Port;

/// Momentary pushbutton hanging off a PortBus
struct Button {
    uint16_t     id;
    uint8_t      state:1;
    uint8_t      prevstate:1;
};

/// Rotary encoder hanging off a PortBus
struct Encoder {
    uint16_t     id1;
    uint16_t     id2;
    uint8_t      state:2;
    uint8_t      prevstate:2;
    uint8_t      prevprevstate:2;
};

/// Utility class for handling input devices hanging off a PortBus.
/// Can keep track of pushbuttons and rotary encoders and turn them
/// into input events sent to a service of choice.
class InPortService : public EventReceiver
{
public:
                 /// Constructor
                 InPortService (void);
                 
                 /// Destructor
                ~InPortService (void);

                 /// Sets the serviceid to target with resulting
                 /// input events.                
    void         setReceiver (serviceid);
    
                 /// Add a pushbutton.
    void         addButton (uint16_t);
    
                 /// Add a rotary encoder.
    void         addEncoder (uint16_t, uint16_t);
    
                 /// Initialize all registered hardware.
    void         begin (void);

                 /// Service event handler
    void         handleEvent (eventtype tp, eventid id, uint16_t X,
                              uint8_t Y, uint8_t Z);

    uint8_t      outsvc; ///< Service-id to message our events to
    
protected:
                 /// Method for interpreting encoder data
    void         handleEncoder (uint8_t);

    Button       buttons[24]; ///< Registered buttons
    Encoder      encoders[16]; ///< Registered encoders
    uint8_t      numbuttons; ///< Number of buttons
    uint8_t      numencoders; ///< Number of encoders
};

/// Global instance
extern InPortService InPort;

/// Utility class for handling outputs (LEDs)
class OutPortHandler
{
public:
                 /// Constructor
                 OutPortHandler (void);
                 
                 /// Destructor
                ~OutPortHandler (void);
    
                 /// Register an output by its i2c address
    void         add (uint16_t);
    
                 /// Turn an output on
    void         on (uint8_t);
    
                 /// Turn an output off
    void         off (uint8_t);
    
                 /// Flash the output on for a limited number of ticks.
    void         flash (uint8_t, uint8_t);

protected:
    uint16_t     outputs[32];
    uint8_t      count;
};

/// Global instance
extern OutPortHandler OutPort;

#endif
