#include <Arduino.h>
#include "Port.h"
#include "I2C.h"
#include "EventQueue.h"
#include "IRQ.h"
#include "Console.h"

// --------------------------------------------------------------------------
void portInterrupt (uint8_t irqid) {
    EventQueue.sendEvent (TYPE_IRQ, SVC_PORT, EV_PORT_IRQ, irqid);
}

// ==========================================================================
// CLASS PortBus
// ==========================================================================
PortBus::PortBus (uint8_t id, uint8_t irq0, uint8_t irq1) {
    i2cid = id;
    pinmodes = 0;
    activeTimers = 0;
    for (uint8_t i=0; i<16; ++i) pinvalues[i] = 0;
    irq0pin = irq0;
    irq1pin = irq1;
    IRQ.assign (irq0, portInterrupt);
    IRQ.assign (irq1, portInterrupt);
}

// --------------------------------------------------------------------------
PortBus::~PortBus (void) {
}

// --------------------------------------------------------------------------
void PortBus::pinMode (uint8_t pin, uint8_t mode) {
    if (mode == INPUT) pinmodes |= ((uint16_t)1<<pin);
}

// --------------------------------------------------------------------------
void PortBus::begin (void) {
    // Set up pinmodes
    I2C.set (i2cid, 0x00, pinmodes & 0xff);
    I2C.set (i2cid, 0x01, (pinmodes & 0xff00) >> 8);
    
    // Set up interrupts
    I2C.set (i2cid, 0x04, pinmodes & 0xff);
    I2C.set (i2cid, 0x05, (pinmodes & 0xff00) >> 8);
    
    // Set up global interrupt polarity
    I2C.set (i2cid, 0x0a, 0x02);
    
    // Set all outputs to their correct states
    uint8_t bt = (pinvalues[0] ? 1 : 0) |
                 (pinvalues[1] ? 2 : 0) |
                 (pinvalues[2] ? 4 : 0) |
                 (pinvalues[3] ? 8 : 0) |
                 (pinvalues[4] ? 16 : 0) |
                 (pinvalues[5] ? 32 : 0) |
                 (pinvalues[6] ? 64 : 0) |
                 (pinvalues[7] ? 128 : 0);
    I2C.set (i2cid, 0x12, bt);
    bt = (pinvalues[8] ? 1 : 0) |
         (pinvalues[9] ? 2 : 0) |
         (pinvalues[10] ? 4 : 0) |
         (pinvalues[11] ? 8 : 0) |
         (pinvalues[12] ? 16 : 0) |
         (pinvalues[13] ? 32 : 0) |
         (pinvalues[14] ? 64 : 0) |
         (pinvalues[15] ? 128 : 0);
    I2C.set (i2cid, 0x13, bt);
    I2C.set (i2cid, 0x13, 0x00);
}

// --------------------------------------------------------------------------
void PortBus::dump (void) {
    char dbg[64];
    sprintf (dbg, "Port %02x\r\n", i2cid);
    Console.write (dbg);
    sprintf (dbg, "    pinmodes %04x\r\n", pinmodes);
    Console.write (dbg);
    sprintf (dbg, "    pinvalues ");
    for (uint8_t i=0; i<16; ++i) {
        strcat (dbg, pinvalues[i] ? "1" : "0");
    }
    strcat (dbg, "\r\n");
    Console.write (dbg);
}

// --------------------------------------------------------------------------
uint8_t PortBus::pinValue (uint8_t pin) {
    return (pinvalues[pin&15] ? HIGH : LOW);
}

// --------------------------------------------------------------------------
void PortBus::pinOut (uint8_t pin, uint8_t tval) {
    if (pinmodes & (1<<(pin&15))) return;
    
    if (tval && tval<255) activeTimers++;
    
    pinvalues[pin&15] = tval;
    if (pin>7) {
        uint8_t bt = (pinvalues[8] ? 1 : 0) |
                     (pinvalues[9] ? 2 : 0) |
                     (pinvalues[10] ? 4 : 0) |
                     (pinvalues[11] ? 8 : 0) |
                     (pinvalues[12] ? 16 : 0) |
                     (pinvalues[13] ? 32 : 0) |
                     (pinvalues[14] ? 64 : 0) |
                     (pinvalues[15] ? 128 : 0);
        I2C.set (i2cid, 0x13, bt);
    }
    else {
        uint8_t bt = (pinvalues[0] ? 1 : 0) |
                     (pinvalues[1] ? 2 : 0) |
                     (pinvalues[2] ? 4 : 0) |
                     (pinvalues[3] ? 8 : 0) |
                     (pinvalues[4] ? 16 : 0) |
                     (pinvalues[5] ? 32 : 0) |
                     (pinvalues[6] ? 64 : 0) |
                     (pinvalues[7] ? 128 : 0);
        I2C.set (i2cid, 0x12, bt);
    }
}

// --------------------------------------------------------------------------
void PortBus::decreaseTimers (void) {
    if (! activeTimers) return;
    bool changed_low = false;
    bool changed_high = false;
    for (uint8_t i=0; i<16; ++i) {
        if (pinvalues[i] && pinvalues[i]<255) {
            pinvalues[i]--;
            if (pinvalues[i] == 0) {
                activeTimers--;
                if (i<8) changed_low = true;
                else changed_high = true;
            }
        }
    }
    
    if (changed_high) {
        uint8_t bt = (pinvalues[8] ? 1 : 0) |
                     (pinvalues[9] ? 2 : 0) |
                     (pinvalues[10] ? 4 : 0) |
                     (pinvalues[11] ? 8 : 0) |
                     (pinvalues[12] ? 16 : 0) |
                     (pinvalues[13] ? 32 : 0) |
                     (pinvalues[14] ? 64 : 0) |
                     (pinvalues[15] ? 128 : 0);
        I2C.set (i2cid, 0x13, bt);
    }
    if (changed_low) {
        uint8_t bt = (pinvalues[0] ? 1 : 0) |
                     (pinvalues[1] ? 2 : 0) |
                     (pinvalues[2] ? 4 : 0) |
                     (pinvalues[3] ? 8 : 0) |
                     (pinvalues[4] ? 16 : 0) |
                     (pinvalues[5] ? 32 : 0) |
                     (pinvalues[6] ? 64 : 0) |
                     (pinvalues[7] ? 128 : 0);
        I2C.set (i2cid, 0x12, bt);
    }
}

// --------------------------------------------------------------------------
void PortBus::handleInterrupt (uint8_t bank, bool isirq) {
    if (i2cid == 0x23) return;
    char dbg[16];
    uint8_t res = 0;
    uint8_t i2coffs = 0x12 + bank;
    uint8_t pins = I2C.get (i2cid, i2coffs);
    
    uint8_t offs = bank ? 8 : 0;
    
    if (I2C.error) {
        sprintf (dbg, "I2C error %i\r\n", I2C.error);
        Console.write (dbg);
        I2C.resetBus();
        Port.begin();
        return;
    }
    
    if (isirq) {
        //sprintf (dbg, "i%02x%02x %02x\r\n", i2cid, i2coffs, pins);
        //Console.write (dbg);
    }
 

    uint8_t numchanges = 0;
    for (uint8_t i=0; i<8; ++i) {
        uint8_t bit = 1 << i;
        if ((pins & bit) && (pinvalues[i+offs] == 0)) {
            numchanges++;
        }
        else if ((! (pins&bit)) && (pinvalues[i+offs])) {
            numchanges++;
        }
    }
    
    if (! numchanges) return;
    if (numchanges > 1) pins = I2C.get (i2cid, i2coffs);
   
    for (uint8_t i=0; i<8; ++i) {
        uint8_t bit = 1 << i;
        if (pins & bit) {
            if (pinvalues[i+offs] == 0) {
                if (! isirq) {
                    //sprintf (dbg, "c0 p%i %02x%02x\r\n", i, i2cid, i2coffs);
                    //Console.write (dbg);
                }
                pinvalues[i+offs] = 255;
                res |= bit;
                EventQueue.sendEvent (TYPE_REQUEST, SVC_INPORT,
                                      EV_INPORT_CHANGE,
                                      (i2cid << 8) | (i+offs), 1);
            }
        }
        else {
            if (pinvalues[i+offs]) {
                if (! isirq) {
                    //sprintf (dbg, "c1 p%i %02x%02x\r\n", i, i2cid, i2coffs);
                    //Console.write (dbg);
                }
                pinvalues[i+offs] = 0;
                res |= bit;
                EventQueue.sendEvent (TYPE_REQUEST, SVC_INPORT,
                                      EV_INPORT_CHANGE,
                                      (i2cid << 8) | (i+offs), 0);
            }
        }
    }
}

// ==========================================================================
// CLASS PortService
// ==========================================================================
PortService::PortService (void) {
}

// --------------------------------------------------------------------------
PortService::~PortService (void) {
}

// --------------------------------------------------------------------------
void PortService::addBus (uint8_t i2cid, uint8_t irq0, uint8_t irq1) {
    if (numports<8) {
        ports[numports++] = new PortBus (i2cid, irq0, irq1);
    }
}

// --------------------------------------------------------------------------
void PortService::addOutput (uint16_t pid) {
    uint8_t i2cid = (pid & 0xff00) >> 8;
    uint8_t pinid = pid & 0xff;
    
    PortBus *p = findPortBus (i2cid);
    if (! p) {
        Serial.print (F("ERROR Could not find portbus "));
        Serial.println (i2cid, HEX);
        return;
    }

    p->pinMode (pinid, OUTPUT);
}

// --------------------------------------------------------------------------
void PortService::addInput (uint16_t pid) {
    uint8_t i2cid = (pid & 0xff00) >> 8;
    uint8_t pinid = pid & 0xff;
    
    PortBus *p = findPortBus (i2cid);
    if (! p) {
        Serial.print (F("ERROR Could not find portbus "));
        Serial.println (i2cid, HEX);
        return;
    }

    p->pinMode (pinid, INPUT);
}

// --------------------------------------------------------------------------
void PortService::addAnalogButton (uint8_t pin, uint8_t type, uint16_t thr) {
    if (numbuttons < 8) {
        abuttons[numbuttons].pin = pin;
        abuttons[numbuttons].type = type;
        abuttons[numbuttons].threshold = thr;
        abuttons[numbuttons].state = 0;
        numbuttons++;
    }
}

// --------------------------------------------------------------------------
void PortService::begin (void) {
    EventQueue.subscribe (SVC_PORT, this);
    EventQueue.subscribeTimer (SVC_PORT);
    for (uint8_t i=0; i<numports; ++i) ports[i]->begin();
}

// --------------------------------------------------------------------------
void PortService::dump (void) {
    for (uint8_t i=0; i<numports; ++i) {
        ports[i]->dump();
    }
}

// --------------------------------------------------------------------------
void PortService::pinOut (uint16_t id, uint8_t timeval) {
    PortBus *p = findPortBus ((id&0xff00)>>8);
    if (p) p->pinOut (id&0xff, timeval);
}

// --------------------------------------------------------------------------
void PortService::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    
    for (uint8_t i=0; i<numports; ++i) {
        switch (id) {
            case EV_PORT_IRQ:
                if (ports[i]->irq0pin == X) {
                    ports[i]->handleInterrupt (0);
                }
                else if (ports[i]->irq1pin == X) {
                    ports[i]->handleInterrupt (1);
                }
                break;
            
            case EV_TIMER_TICK:
                if (! (EventQueue.ts & 7)) ports[i]->decreaseTimers();
                
                // Do pre-emptive polling in case we missed an
                // interrupt.
                if (! (EventQueue.ts & 63)) {
                    ports[i]->handleInterrupt (0, false);
                }
                else if ((EventQueue.ts&63) == 32) {
                    ports[i]->handleInterrupt (1, false);
                }
                break;
        }
    }
    
    if (id == EV_TIMER_TICK && (! (EventQueue.ts & 7))) {
        for (uint8_t i=0; i<numbuttons; ++i) {
            uint16_t val = analogRead (abuttons[i].pin);
            uint8_t state;
            switch (abuttons[i].type) {
                case ABUTTON_DOWN_LOW:
                    state = (val < abuttons[i].threshold) ? HIGH : LOW;
                    break;
                
                case ABUTTON_DOWN_HIGH:
                    state = (val > abuttons[i].threshold) ? HIGH : LOW;
                    break;
            }
            
            switch (state) {
                case HIGH:
                    if (abuttons[i].state == 0) {
                        abuttons[i].state = 5;
                    }
                    else if ((abuttons[i].state > 1) &&
                             (abuttons[i].state < 6)) {
                        abuttons[i].state--;
                        if (abuttons[i].state == 1) {
                            EventQueue.sendEvent (TYPE_REQUEST, InPort.outsvc,
                                                  EV_INPUT_ABUTTON_DOWN, i);
                        }
                    }
                    break;
                    
                case LOW:
                    if (abuttons[i].state == 1) {
                        abuttons[i].state = 6;
                    }
                    if (abuttons[i].state) {
                        abuttons[i].state++;
                        if (abuttons[i].state == 12) {
                            abuttons[i].state = 0;
                            EventQueue.sendEvent (TYPE_REQUEST, InPort.outsvc,
                                                  EV_INPUT_ABUTTON_UP, i);
                        }
                    }
                    break;
            }
        }
    }
}

// --------------------------------------------------------------------------
PortBus *PortService::findPortBus (uint8_t i2cid) {
    for (uint8_t i=0; i<numports; ++i) {
        if (ports[i]->i2cid == i2cid) return ports[i];
    }
    return NULL;
}

// --------------------------------------------------------------------------
PortService Port;

// ==========================================================================
// CLASS InPortService
// ==========================================================================
InPortService::InPortService (void) {
    numbuttons = numencoders = 0;
    outsvc = 0;
}

// --------------------------------------------------------------------------
InPortService::~InPortService (void) {
}

// --------------------------------------------------------------------------
void InPortService::setReceiver (serviceid id) {
    outsvc = id;
}

// --------------------------------------------------------------------------
void InPortService::addButton (uint16_t id) {
    Port.addInput (id);
    uint8_t i = numbuttons;
    buttons[i].id = id;
    buttons[i].state = 0;
    buttons[i].prevstate = 0;
    numbuttons++;
}

// --------------------------------------------------------------------------
void InPortService::addEncoder (uint16_t id1, uint16_t id2) {
    uint8_t i = numencoders;
    if (i<16) {
        Port.addInput (id1);
        Port.addInput (id2);
        encoders[i].id1 = id1;
        encoders[i].id2 = id2;
        encoders[i].state = 0;
        encoders[i].prevstate = 0;
        encoders[i].prevprevstate = 0;
        numencoders++;
    }
}

// --------------------------------------------------------------------------
void InPortService::begin (void) {
    EventQueue.subscribe (SVC_INPORT, this);
}

// --------------------------------------------------------------------------
void InPortService::handleEncoder (uint8_t idx) {
    if (encoders[idx].state == 0) {
        if (encoders[idx].prevstate == 1 &&
            encoders[idx].prevprevstate == 3) {
            EventQueue.sendEvent (TYPE_REQUEST, outsvc,
                                  EV_INPUT_ENCODER_LEFT, idx);
        }
        else if (encoders[idx].prevprevstate == 3) {
            EventQueue.sendEvent (TYPE_REQUEST, outsvc,
                                  EV_INPUT_ENCODER_RIGHT, idx);
        }
    }
}

// --------------------------------------------------------------------------
void InPortService::handleEvent (eventtype tp, eventid id, uint16_t X,
                                 uint8_t Y, uint8_t Z) {
    uint8_t i;
    
    for (i=0; i<numencoders;++i) {
        if (encoders[i].id1 == X) {
            encoders[i].prevprevstate = encoders[i].prevstate;
            encoders[i].prevstate = encoders[i].state;
            if (Y) encoders[i].state |= 1;
            else encoders[i].state &= 2;
            handleEncoder (i);
            return;
        }
        else if (encoders[i].id2 == X) {
            encoders[i].prevprevstate = encoders[i].prevstate;
            encoders[i].prevstate = encoders[i].state;
            if (Y) encoders[i].state |= 2;
            else encoders[i].state &= 1;
            handleEncoder (i);
            return;
        }
    }
    for (i=0; i<numbuttons; ++i) {
        if (buttons[i].id == X) {
            buttons[i].prevstate = buttons[i].state;
            buttons[i].state = Y ? 1 : 0;
            if (Y) {
                EventQueue.sendEvent (TYPE_REQUEST, outsvc,
                                      EV_INPUT_BUTTON_DOWN, i);
            }
            else {
                EventQueue.sendEvent (TYPE_REQUEST, outsvc,
                                      EV_INPUT_BUTTON_UP, i);
            }
        }
    }
}

// --------------------------------------------------------------------------
InPortService InPort;

// ==========================================================================
// CLASS OutPortHandler
// ==========================================================================
OutPortHandler::OutPortHandler (void) {
    count = 0;
}

OutPortHandler::~OutPortHandler (void) {
}

void OutPortHandler::add (uint16_t id) {
    outputs[count++] = id;
    Port.addOutput (id);
}

void OutPortHandler::on (uint8_t outnum) {
    if (outnum < count) {
        Port.pinOut (outputs[outnum], 255);
    }
}

void OutPortHandler::off (uint8_t outnum) {
    if (outnum < count) {
        Port.pinOut (outputs[outnum], 0);
    }
}

void OutPortHandler::flash (uint8_t outnum, uint8_t ti) {
    if (outnum < count) {
        Port.pinOut (outputs[outnum], ti);
    }
}

OutPortHandler OutPort;
