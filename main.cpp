#include <Arduino.h>
#include <SPI.h>
#include "EventQueue.h"
#include "Port.h"
#include "Console.h"
#include "Application.h"
#include "Display.h"
#include "DriverILI9341P.h"
#include "Memory.h"
#include "IRQ.h"

/* RegDef:  External Memory Control Register A */
#define XMCRA _SFR_MEM8(0x74)

/* RegDef:  External Memory Control Register A */
#define XMCRB _SFR_MEM8(0x75)


#define SVC_LOOP 0x42
#define EV_LOOP_BTIRQ 0x1f

// --------------------------------------------------------------------------
class Main : public Application
{
public:
                     Main (void) {
                        encval[0] = encval[1] = 64;
                        oldval[0] = oldval[1] = 0;
                        missed = 255;
                        fmem = 0;
                        
                    }
                    ~Main (void) {}
    
    void             setup (void);
    void             start (void);
    void             handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
    
    uint8_t          encval[16];
    uint8_t          oldval[16];
    uint16_t         missed;
    uint16_t         fmem;
};

Main M;

#include "helvetica11.h"
#include "helvetica24.h"
Font FontList[2] = {{FONT_HELVETICA_11, OFFS_HELVETICA_11, 11},
                    {FONT_HELVETICA_24, OFFS_HELVETICA_24, 29}};
Font *Fonts = FontList;

// --------------------------------------------------------------------------
void Main::setup (void) {
    // Set the inPort to send to a bogus serviceid, so its
    // events end up in the main loop.
    InPort.setReceiver (SVC_LOOP);

    Port.addBus (0x20, 22, 23);
    Port.addBus (0x21, 24, 25);
    Port.addBus (0x22, 26, 27);
    Port.addBus (0x23, 30, 31);
    
    InPort.addEncoder (0x2003, 0x2004);
    InPort.addEncoder (0x200f, 0x2008);
    InPort.addButton (0x2006);
    InPort.addButton (0x200a);
    OutPort.add (0x2000);
    OutPort.add (0x200c);

    InPort.addEncoder (0x2103, 0x2104);
    InPort.addEncoder (0x210f, 0x2108);
    InPort.addButton (0x2106);
    InPort.addButton (0x210a);
    OutPort.add (0x2100);
    OutPort.add (0x210c);

    InPort.addEncoder (0x2203, 0x2204);
    InPort.addEncoder (0x220f, 0x2208);
    InPort.addButton (0x2206);
    InPort.addButton (0x220a);
    OutPort.add (0x2200);
    OutPort.add (0x220c);

    InPort.addEncoder (0x2303, 0x2304);
    InPort.addEncoder (0x230f, 0x2308);
    InPort.addButton (0x2306);
    InPort.addButton (0x230a);
    OutPort.add (0x2300);
    OutPort.add (0x230c);
    
    InPort.addEncoder (0x2002, 0x2005);
    InPort.addEncoder (0x200e, 0x2009);
    InPort.addButton (0x2001);
    InPort.addButton (0x200d);
    OutPort.add (0x2007);
    OutPort.add (0x200b);

    InPort.addEncoder (0x2102, 0x2105);
    InPort.addEncoder (0x210e, 0x2109);
    InPort.addButton (0x2101);
    InPort.addButton (0x210d);
    OutPort.add (0x2107);
    OutPort.add (0x210b);

    InPort.addEncoder (0x2202, 0x2205);
    InPort.addEncoder (0x220e, 0x2209);
    InPort.addButton (0x2201);
    InPort.addButton (0x220d);
    OutPort.add (0x2207);
    OutPort.add (0x220b);

    InPort.addEncoder (0x2302, 0x2305);
    InPort.addEncoder (0x230e, 0x2309);
    InPort.addButton (0x2301);
    InPort.addButton (0x230d);
    OutPort.add (0x2307);
    OutPort.add (0x230b);

    Display.begin (DriverILI9341P::load());
    
    Port.addAnalogButton (1, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (2, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (3, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (4, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (5, ABUTTON_DOWN_LOW, 1000);
}

// --------------------------------------------------------------------------
void Main::start (void) {
    // Report the happy news
    Console.write ("Application started\r\n");
    for (uint8_t i=0; i<12; ++i) {
        OutPort.flash (i, 20);
    }
    Display.setBackground (0x30, 0x50, 0x90);
    Display.clearBackground();
    Display.backlightOn();
//    Display.setBackground (0xff, 0xff, 0xff);
//    Display.setInk (0,0,0);
    Display.setFont (1);
    Display.setInk (255,255,255);
    Display.setCursor (5,5);
    Display.write ("MIDIx 1.0 Ready");
    Display.setCursor (5, 34);
    Display.setFont (0);
    Display.write ("Copyright 2015 Midilab");
    Display.setFont (1);
    EventQueue.subscribeTimer (SVC_LOOP);
    Serial1.begin (31250);
}

// --------------------------------------------------------------------------
void Main::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    char dbg[16];
    switch (id) {
        case EV_TIMER_TICK:
            if (EventQueue.ts & 7) return;
            
            for (int i=0; i<16; ++i) {
                if (oldval[i] != encval[i]) {
                    Display.setFont (0);
                    Display.setCursor (32+32*(i&7),(i&8)?120:100);
                    Display.write (encval[i], true);
                    oldval[i] = encval[i];
                }
            }

            if (missed != EventQueue.missedTicks) {
                Display.setCursor (5,225);
                Display.setFont (0);
                Display.write ("Miss: ");
                Display.write (EventQueue.missedTicks);
                Display.setFont (1);
                missed = EventQueue.missedTicks;
            }
            if (fmem != (uint16_t) Memory.available()) {
                fmem = (uint16_t) Memory.available();
                Display.setCursor (160,225);
                Display.setFont (0);
                Display.write ("MemFree: ");
                Display.write (fmem);
                Display.setFont (1);
            }
            if (Serial1.available()) {
                uint8_t bt = Serial1.read();
                Serial.println (bt, HEX);
            }
            break;
            
        case EV_INPUT_ABUTTON_DOWN:
            sprintf (dbg, "ADOWN %i\r\n", X);
            Console.write (dbg);
            break;
            
        case EV_INPUT_ABUTTON_UP:
            sprintf (dbg, "AUP %i\r\n", X);
            Console.write (dbg);
            break;
            
        case EV_LOOP_BTIRQ:
            sprintf (dbg, "BTIRQ %i\r\n", X);
            break;
            
        case EV_INPUT_BUTTON_DOWN:
            sprintf (dbg, "BTDOWN %i\r\n", X);
            Console.write (dbg);
            encval[X]=64;
            break;
        
        case EV_INPUT_BUTTON_UP:
            sprintf (dbg, "BTUP %i\r\n", X);
            Console.write (dbg);
            break;
            
        case EV_INPUT_ENCODER_LEFT:
            if (encval[X]) {
                encval[X]--;
            }
            OutPort.flash (X & 7, 1);
            break;

        case EV_INPUT_ENCODER_RIGHT:
            if (encval[X]<127) {
                encval[X]++;
            }
            OutPort.flash ((X&7)+8,1);
            break;
    }
}
