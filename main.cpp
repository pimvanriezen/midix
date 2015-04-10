#include <Arduino.h>
#include "EventQueue.h"
#include "Port.h"
#include "Console.h"
#include "Application.h"
#include "Display.h"
#include "DriverILI9341.h"
#include <avr/io.h>

/* RegDef:  External Memory Control Register A */
#define XMCRA _SFR_MEM8(0x74)

/* RegDef:  External Memory Control Register A */
#define XMCRB _SFR_MEM8(0x75)


#define SVC_LOOP 0x42

// --------------------------------------------------------------------------
class Main : public Application
{
public:
                     Main (void) {
                        encval[0] = encval[1] = 64;
                        oldval[0] = oldval[1] = 0;
                        missed = 255;
                    }
                    ~Main (void) {}
    
    void             setup (void);
    void             start (void);
    void             handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
    
    uint8_t          encval[2];
    uint8_t          oldval[2];
    uint16_t         missed;
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
    
    XMCRB=1;
    XMCRA= 0x85;
    
    volatile uint8_t *ptr = (volatile uint8_t *) 0x2200;
    
    *ptr = 0x55;
    
    while (ptr < (uint8_t *) 0xA200) {
        *ptr++ = 0x55;
    }
    ptr = (uint8_t *) 0x2200;
    while (ptr < (uint8_t *) 0xA200) {
        if (*ptr == 0x55) {
            ptr++;
            continue;
        }
        if ((*ptr) != 0x55) {
            Serial.print ("!");
            Serial.print ((uint16_t) ptr, HEX);
            Serial.print (":");
            Serial.print (*ptr, HEX);
        }
        ptr++;
    }
    
    ptr = (uint8_t *) 0x2200;

    while (ptr < (uint8_t *) 0xA200) {
        *ptr++ = 0xaa;
    }
    ptr = (uint8_t *) 0x2200;

    while (ptr < (uint8_t *) 0xA200) {
        *ptr++ = 0xaa;
    }
    ptr = (uint8_t *) 0x2200;
    while (ptr < (uint8_t *) 0xA200) {
        if (*ptr == 0xaa) {
            ptr++;
            continue;
        }
        if ((*ptr) != 0xaa) {
            Serial.print ("!");
            Serial.print ((uint16_t) ptr, HEX);
            Serial.print (":");
            Serial.print (*ptr, HEX);
        }
        ptr++;
    }

    Serial.println (*ptr, HEX);
}

// --------------------------------------------------------------------------
void Main::start (void) {
    // Report the happy news
    Console.write ("Application started\r\n");
    OutPort.flash (0, 2);
    OutPort.flash (1, 2);
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
    
    EventQueue.subscribeTimer (SVC_LOOP);
}

// --------------------------------------------------------------------------
void Main::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    switch (id) {
        case EV_TIMER_TICK:
            if (EventQueue.ts & 31) return;
            if (oldval[0] != encval[0]) {
                Display.setCursor (150,100);
                Display.write (encval[0], true);
                oldval[0] = encval[0];
            }
            if (oldval[1] != encval[1]) {
                Display.setCursor (150,130);
                Display.write (encval[1], true);
                oldval[1] = encval[1];
            }
            if (missed != EventQueue.missedTicks) {
                Display.setCursor (5,200);
                Display.setFont (0);
                Display.write (EventQueue.missedTicks>>8);
                Display.setFont (1);
                missed = EventQueue.missedTicks;
            }
            return;
            
        case EV_INPUT_BUTTON_DOWN:
            Console.write ("Button down\r\n");
            encval[X]=64;
            break;
        
        case EV_INPUT_BUTTON_UP:
            Console.write ("Button up\r\n");
            break;
            
        case EV_INPUT_ENCODER_LEFT:
            if (encval[X]) {
                encval[X]--;
            }
            OutPort.flash (0,1);
            break;

        case EV_INPUT_ENCODER_RIGHT:
            if (encval[X]<127) {
                encval[X]++;
            }
            OutPort.flash (1,1);
            break;
    }
}
