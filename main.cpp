#include <Arduino.h>
#include "EventQueue.h"
#include "Port.h"
#include "Console.h"
#include "Application.h"

#define SVC_LOOP 0x42

// --------------------------------------------------------------------------
class Main : public Application
{
public:
                     Main (void) {}
                    ~Main (void) {}
    
    void             setup (void);
    void             start (void);
    void             handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
};

Main M;

// --------------------------------------------------------------------------
void Main::setup (void) {
    // Assign interrupt 4 to pins 0-7 of i2c-connected port
    // extenders.
    Port.assignInterrupt (4,0);

    // Set the inPort to send to a bogus serviceid, so its
    // events end up in the main loop.
    InPort.setReceiver (SVC_LOOP);

    // Button 0 hangs off GPIO 7 on i2c address 0x21
    InPort.addButton (0x2107);

    // LED 0-1 hang off GPIO 8-9 on i2c address 0x21
    OutPort.add (0x2108);
    OutPort.add (0x2109);
}

// --------------------------------------------------------------------------
void Main::start (void) {
    // Report the happy news
    Console.write ("Application started\r\n");
    OutPort.flash (0, 80);
    OutPort.flash (1, 80);
}

// --------------------------------------------------------------------------
void Main::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    switch (id) {
        case EV_INPUT_BUTTON_DOWN:
            Console.write ("Button down\r\n");
            OutPort.flash (0, 10);
            break;
        
        case EV_INPUT_BUTTON_UP:
            Console.write ("Button up\r\n");
            OutPort.flash (1, 10);
            break;
    }
}
