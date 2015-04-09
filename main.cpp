#include <Arduino.h>
#include "EventQueue.h"
#include "Port.h"
#include "Console.h"
#include "Application.h"
#include "Display.h"
#include "DriverILI9341.h"

#define SVC_LOOP 0x42

// --------------------------------------------------------------------------
class Main : public Application
{
public:
                     Main (void) { encval[0] = encval[1] = 64;}
                    ~Main (void) {}
    
    void             setup (void);
    void             start (void);
    void             handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
    
    uint8_t          encval[2];
};

Main M;

#include "helvetica11.h"
#include "helvetica24.h"
Font FontList[2] = {{FONT_HELVETICA_11, OFFS_HELVETICA_11, 11},
                    {FONT_HELVETICA_24, OFFS_HELVETICA_24, 29}};
Font *Fonts = FontList;

// --------------------------------------------------------------------------
void Main::setup (void) {
    // Assign interrupt 4 to pins 0-7 of i2c-connected port
    // extenders.
    Port.assignInterrupt (4,0);

    // Set the inPort to send to a bogus serviceid, so its
    // events end up in the main loop.
    InPort.setReceiver (SVC_LOOP);

    // Two encoders, hanging off pins 3&4 and 2&5 respectively
    InPort.addEncoder (0x2103, 0x2104);
    InPort.addEncoder (0x2102, 0x2105);
    
    // And their respective push button functions
    InPort.addButton (0x2106);
    InPort.addButton (0x2101);
    
    // LED 0-1 hang off GPIO pins 0 and 7 on i2c address 0x21
    OutPort.add (0x2100);
    OutPort.add (0x2107);
    
    Display.begin (DriverILI9341::load());
}

// --------------------------------------------------------------------------
void Main::start (void) {
    // Report the happy news
    Console.write ("Application started\r\n");
    OutPort.flash (0, 100);
    OutPort.flash (1, 100);
    Display.setBackground (0x20, 0x30, 0x50);
    Display.clearBackground();
    Display.backlightOn();
    Display.setFont (1);
    Display.setInk (255,255,255);
    Display.setCursor (5,5);
    Display.write ("MIDIx 1.0 Ready");
    Display.setCursor (5, 34);
    Display.setFont (0);
    Display.write ("Copyright 2015 Midilab");
    Display.setFont (1);
    for (uint8_t X=0; X<2; ++X) {
        Display.setCursor (150,100+30*X);
        Display.write (encval[X], true);
    }
}

// --------------------------------------------------------------------------
void Main::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    switch (id) {
        case EV_INPUT_BUTTON_DOWN:
            Console.write ("Button down\r\n");
            encval[X]=64;
            Display.setCursor (150,100+30*X);
            Display.write (encval[X], true);
            break;
        
        case EV_INPUT_BUTTON_UP:
            Console.write ("Button up\r\n");
            break;
            
        case EV_INPUT_ENCODER_LEFT:
            Console.write ("Encoder left\r\n");
            if (encval[X]) {
                encval[X]--;
                Display.setCursor (150,100+30*X);
                Display.write (encval[X], true);
            }
            OutPort.flash (0,10);
            break;

        case EV_INPUT_ENCODER_RIGHT:
            Console.write ("Encoder right\r\n");
            if (encval[X]<127) {
                encval[X]++;
                Display.setCursor (150,100+30*X);
                Display.write (encval[X], true);
            }
            OutPort.flash (1,10);
            break;
    }
}
