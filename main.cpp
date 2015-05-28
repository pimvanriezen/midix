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
#include "EEPROM.h"
#include "Hardware.h"

#define SVC_LOOP 0x42

#define DISPLAY_NOTES 0
#define DISPLAY_NUMBERS 1
#define DISPLAY_GRAPH 2

#define ABUTTON_PREV 0
#define ABUTTON_OK_MENU 1
#define ABUTTON_NEXT 2
#define ABUTTON_TEMPO_TRIGGER 3
#define ABUTTON_STOP_LAYER 4

// --------------------------------------------------------------------------
class Main : public Application
{
public:
                     Main (void) {
                        for (uint8_t i=0; i<16; ++i) {
                            encval[i] = 64;
                            oldval[i] = 0;
                        }
                        fmem = 0;
                        
                    }
                    ~Main (void) {}
    
    void             setup (void);
    void             start (void);
    void             updateButtonDisplay (void);
    void             updateEncoderDisplay (void);
    void             handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
    
    uint8_t          encval[16];
    uint8_t          oldval[16];
    uint8_t          dtype;
    uint16_t         missed;
    uint16_t         fmem;
    uint32_t         nextnote;
    uint8_t          seqpos;
};

Main M;

#include "helvetica11.h"
#include "helvetica24.h"
Font FontList[2] = {{FONT_HELVETICA_11, OFFS_HELVETICA_11, 11},
                    {FONT_HELVETICA_24, OFFS_HELVETICA_24, 29}};
Font *Fonts = FontList;

// --------------------------------------------------------------------------
void Main::setup (void) {
    EEPROM.setAddress (I2C_EEPROM);
    if (! EEPROM.valid ('FReQ',1,0)) {
        Serial.println (F("EEPROM invalid"));
        EEPROM.init ('FReQ',1,0,512);
        Serial.println (F("EEPROM initialized"));
    }
    else {
        Serial.println (F("Initialized EEPROM found"));
    }

    // Set the inPort to send to a bogus serviceid, so its
    // events end up in the main loop.
    InPort.setReceiver (SVC_LOOP);

    Port.addBus (I2C_MCP23017_0, PIN_IRQ0, PIN_IRQ1);
    Port.addBus (I2C_MCP23017_1, PIN_IRQ2, PIN_IRQ3);
    Port.addBus (I2C_MCP23017_2, PIN_IRQ4, PIN_IRQ5);
    
    // FIXME: broken IRQ connect on prototype PCB
    Port.addBus (I2C_MCP23017_3, PIN_IRQ7, PIN_IRQ7);
    
    // Encoders & LEDS 1&2 (MCP23017 0x20)
    InPort.addEncoder (0x2003, 0x2004);
    InPort.addEncoder (0x200f, 0x2008);
    InPort.addButton (0x2006);
    InPort.addButton (0x200a);
    OutPort.add (0x2000);
    OutPort.add (0x200c);

    // Encoders & LEDS 3&4 (MCP23017 0x21)
    InPort.addEncoder (0x2103, 0x2104);
    InPort.addEncoder (0x210f, 0x2108);
    InPort.addButton (0x2106);
    InPort.addButton (0x210a);
    OutPort.add (0x2100);
    OutPort.add (0x210c);

    // Encoders & LEDS 5&6 (MCP23017 0x22)
    InPort.addEncoder (0x2203, 0x2204);
    InPort.addEncoder (0x220f, 0x2208);
    InPort.addButton (0x2206);
    InPort.addButton (0x220a);
    OutPort.add (0x2200);
    OutPort.add (0x220c);

    // Encoders & LEDS 7&8 (MCP23017 0x23)
    InPort.addEncoder (0x2303, 0x2304);
    InPort.addEncoder (0x230f, 0x2308);
    InPort.addButton (0x2306);
    InPort.addButton (0x230a);
    OutPort.add (0x2300);
    OutPort.add (0x230c);
    
    // Encoders & LEDS 9&10 (MCP23017 0x20)
    InPort.addEncoder (0x2002, 0x2005);
    InPort.addEncoder (0x200e, 0x2009);
    InPort.addButton (0x2001);
    InPort.addButton (0x200d);
    OutPort.add (0x2007);
    OutPort.add (0x200b);

    // Encoders & LEDS 11&12 (MCP23017 0x21)
    InPort.addEncoder (0x2102, 0x2105);
    InPort.addEncoder (0x210e, 0x2109);
    InPort.addButton (0x2101);
    InPort.addButton (0x210d);
    OutPort.add (0x2107);
    OutPort.add (0x210b);

    // Encoders & LEDS 13&14 (MCP23017 0x22)
    InPort.addEncoder (0x2202, 0x2205);
    InPort.addEncoder (0x220e, 0x2209);
    InPort.addButton (0x2201);
    InPort.addButton (0x220d);
    OutPort.add (0x2207);
    OutPort.add (0x220b);

    // Encoders & LEDS 15&16 (MCP23017 0x23)
    InPort.addEncoder (0x2302, 0x2305);
    InPort.addEncoder (0x230e, 0x2309);
    InPort.addButton (0x2301);
    InPort.addButton (0x230d);
    OutPort.add (0x2307);
    OutPort.add (0x230b);

    Display.begin (DriverILI9341P::load());
    
    Port.addAnalogButton (APIN_BUTTON_PREV, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (APIN_BUTTON_OK_MENU, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (APIN_BUTTON_NEXT, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (APIN_BUTTON_TRIGGER_TEMPO, ABUTTON_DOWN_LOW, 1000);
    Port.addAnalogButton (APIN_BUTTON_LAYER_STOP, ABUTTON_DOWN_LOW, 1000);
    
    dtype = DISPLAY_NOTES;
}

// --------------------------------------------------------------------------
void Main::updateButtonDisplay (void) {
    if (dtype == 0) {
        Display.setBackground (30,73,23);
    }
    else {
        Display.setBackground (90,232,70);
    }
    Display.setInk (0x10,0x18,0x30);
    Display.clearRect (2,224,80,16);
    Display.setCursor (16, 226);
    Display.write ("Previous");

    if (dtype == 2) {
        Display.setBackground (30,73,23);
    }
    else {
        Display.setBackground (90,232,70);
    }
    Display.setInk (0x10,0x18,0x30);
    Display.clearRect (238,224,80,16);
    Display.setCursor (266, 226);
    Display.write ("Next");
    
    Display.setBackground (57,241,247);
    Display.clearRect (125,224,70,16);
    Display.setInk (0x10,0x18,0x30);
    Display.setCursor (145, 226);
    Display.write ("Menu");
    
    Display.setBackground (0x18, 0x28, 0x48);
    Display.setInk (255,255,255);
}

// --------------------------------------------------------------------------
void Main::start (void) {
    // Report the happy news
    Console.write ("Application started\r\n");
    for (uint8_t i=0; i<16; ++i) {
        OutPort.flash (i, 20);
    }
    Display.setBackground (0x18, 0x28, 0x48);
    Display.clearBackground();
    Display.setFont (1);
    Display.setBackground (220,220,200);
    Display.setInk (0x10,0x18,0x30);
    Display.clearRect (0,0,120,48);
    Display.clearRect (120,0,200,48);
    Display.setCursor (5,5);
    Display.write ("Edit Sequence");
    Display.setCursor (5, 34);
    Display.setFont (0);
    Display.write ("TRIGGER 1  LAYER 1");
    Display.setInk (0,0,0);
    Display.fillRect (0,49,160,2);
    Display.fillRect (160,49,160,2);
    Display.fillRect (0,219,160,21);
    Display.fillRect (160,219,160,21);
    
    updateButtonDisplay();
   
    Display.backlightOn();
    EventQueue.subscribeTimer (SVC_LOOP);
    EventQueue.subscribeMIDI (SVC_LOOP, this);
    Serial1.begin (31250);
    nextnote = millis() + 250;
    seqpos = 15;
}

// --------------------------------------------------------------------------
void Main::updateEncoderDisplay (void) {
    static uint8_t lasttype = DISPLAY_NUMBERS;
    
    if (lasttype != dtype) {
        Display.clearRect (0,66,120,129);
        Display.clearRect (120,66,200,129);
        lasttype = dtype;
        for (uint8_t i=0; i<16; ++i) oldval[i] = 128;
        updateButtonDisplay();
    }
    
    if (dtype == DISPLAY_NUMBERS) {
        for (uint8_t i=0; i<16; ++i) {
            if (oldval[i] != encval[i]) {
                Display.setInk (255,255,255);
                Display.setCursor (38+32*(i&7),(i&8)?130:110);
                EventQueue.yield();
                Display.write (encval[i], true);
                Display.setInk (224,224,210);
                Display.drawBox (32+32*(i&7),(i&8)?125:105,32,20);
                oldval[i] = encval[i];
                break;
            }
        }
    }
    else if (dtype == DISPLAY_GRAPH) {
        for (uint8_t i=0; i<16; ++i) {
            if (oldval[i] != encval[i]) {
                uint8_t top, bot;
                uint16_t x = (2+i) * 16;
                uint8_t v = encval[i];
                Display.setInk (255,255,255);
                if (v<64) {
                    if (v<63) Display.clearRect (x,131,11,63-v);
                    Display.drawBox (x,130,11,(64-v));
                    top = 130;
                    bot = 130 + (64-v);
                }
                else {
                    Display.fillRect (x,(194-v),12,131-(194-v));
                    top = 194-v;
                    bot = 130;
                }
                EventQueue.yield();
                if (top>66) {
                    Display.clearRect (x,66,12,top-66);
                }
                if (bot < 194) {
                    Display.clearRect (x,bot+1,12,195-bot);
                }
                oldval[i] = encval[i];
                break;
            }
        }
    }
    else if (dtype == DISPLAY_NOTES) {
        for (uint8_t i=0; i<16; ++i) {
            if (oldval[i] != encval[i]) {
                char nstr[6];
                nstr[1] = ' ';
                nstr[3] = ' ';
                nstr[4] = nstr[5] = 0;
                switch (encval[i] % 12) {
                    case 1:
                        nstr[1] = '#';
                    case 0:
                        nstr[0] = 'C';
                        break;
                        
                    case 3:
                        nstr[1] = '#';
                    case 2:
                        nstr[0] = 'D';
                        break;
                    
                    case 4:
                        nstr[0] = 'E';
                        break;
                    
                    case 6:
                        nstr[1] = '#';
                    case 5:
                        nstr[0] = 'F';
                        break;
                    
                    case 8:
                        nstr[1] = '#';
                    case 7:
                        nstr[0] = 'G';
                        break;
                    
                    case 10:
                        nstr[1] = '#';
                    case 9:
                        nstr[0] = 'A';
                        break;
                    
                    case 11:
                        nstr[0] = 'B';
                        break;
                }
                if (encval[i]/12) {
                    nstr[2] = '0' + (encval[i]/12) -1;
                }
                else {
                    nstr[2] = '-';
                    nstr[3] = '1';
                    nstr[4] = ' ';
                }
                        
                Display.setInk (255,255,255);
                Display.setCursor (38+32*(i&7),(i&8)?130:110);
                EventQueue.yield();
                Display.write (nstr, true);
                Display.setInk (224,224,210);
                Display.drawBox (32+32*(i&7),(i&8)?125:105,32,20);
                oldval[i] = encval[i];
                break;
            }
        }
    }     
}

// --------------------------------------------------------------------------
void Main::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    char dbg[16];
    switch (id) {
        case EV_TIMER_MIDIPULSE:
            if ((EventQueue.midiclock % 48) == 0) {
                OutPort.off (seqpos);
                seqpos = (seqpos+1) & 15;
                OutPort.on (seqpos);
            }
            break;
        
        case EV_TIMER_TICK:
            if (EventQueue.ts & 7) return;
            
            updateEncoderDisplay();

            if (Serial1.available()) {
                uint8_t bt = Serial1.read();
                Serial.println (bt, HEX);
            }
            break;
            
        case EV_INPUT_ABUTTON_DOWN:
            if (X == ABUTTON_STOP_LAYER) {
                for (uint8_t i=0; i<16; ++i) {
                    OutPort.on (i);
                }
            }
            else if (X == ABUTTON_PREV && dtype) dtype--;
            else if (X == ABUTTON_NEXT && dtype < 2) dtype++;
            break;
            
        case EV_INPUT_ABUTTON_UP:
            if (X == ABUTTON_STOP_LAYER) {
                for (uint8_t i=0; i<16; ++i) {
                    OutPort.off (i);
                }
            }
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
