#include <Arduino.h>
#include "Port.h"
#include "I2C.h"
#include "EventQueue.h"

// ==========================================================================
// CLASS PortBus
// ==========================================================================
PortBus::PortBus (uint8_t id) {
    i2cid = id;
    pinmodes = 0;
    for (uint8_t i=0; i<16; ++i) pinvalues[i] = 0;
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
    
    // Set all outputs to LOW
    I2C.set (i2cid, 0x12, 0x00);
    I2C.set (i2cid, 0x13, 0x00);
}

// --------------------------------------------------------------------------
uint8_t PortBus::pinValue (uint8_t pin) {
    return (pinvalues[pin&15] ? HIGH : LOW);
}

// --------------------------------------------------------------------------
void PortBus::pinOut (uint8_t pin, uint8_t tval) {
    if (pinmodes & (1<<(pin&15))) return;
    
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
    bool changed_low = false;
    bool changed_high = false;
    for (uint8_t i=0; i<16; ++i) {
        if (pinvalues[i] && pinvalues[i]<255) {
            pinvalues[i]--;
            if (pinvalues[i] == 0) {
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
void PortBus::handleInterrupt (uint8_t bank) {
    uint8_t res = 0;
    uint8_t i2coffs = 0x12 + bank;
    uint8_t pins = I2C.get (i2cid, i2coffs);
    uint8_t offs = bank ? 8 : 0;
    
    for (uint8_t i=0; i<8; ++i) {
        uint8_t bit = 1 << i;
        if (pins & bit) {
            if (pinvalues[i+offs] == 0) {
                pinvalues[i+offs] = 255;
                res |= bit;
                EventQueue.sendEvent (TYPE_REQUEST, SVC_INPort,
                                      EV_INPort_CHANGE,
                                      (i2cid << 8) | (i+offs), 1);
            }
        }
        else {
            if (pinvalues[i+offs]) {
                pinvalues[i+offs] = 0;
                res |= bit;
                EventQueue.sendEvent (TYPE_REQUEST, SVC_INPort,
                                      EV_INPort_CHANGE,
                                      (i2cid << 8) | (i+offs), 0);
            }
        }
    }
}

// --------------------------------------------------------------------------
void portInterruptBank0 (void) {
    EventQueue.sendEvent (TYPE_IRQ, SVC_PORT, EV_PORT_IRQ, 0);
}

// --------------------------------------------------------------------------
void portInterruptBank1 (void) {
    EventQueue.sendEvent (TYPE_IRQ, SVC_PORT, EV_PORT_IRQ, 1);
}

// ==========================================================================
// CLASS PortService
// ==========================================================================
PortService::PortService (void) {
    tick = 0;
}

// --------------------------------------------------------------------------
PortService::~PortService (void) {
}

// --------------------------------------------------------------------------
void PortService::addBus (uint8_t i2cid) {
    if (numports<8) {
        ports[numports++] = new PortBus (i2cid);
    }
}

// --------------------------------------------------------------------------
void PortService::addOutput (uint16_t pid) {
    uint8_t i2cid = (pid & 0xff00) >> 8;
    uint8_t pinid = pid & 0xff;
    
    PortBus *p = findPortBus (i2cid);
    if (! p) {
        addBus (i2cid);
        p = findPortBus (i2cid);
    }
    if (p) {
        p->pinMode (pinid, OUTPUT);
    }
}

// --------------------------------------------------------------------------
void PortService::addInput (uint16_t pid) {
    uint8_t i2cid = (pid & 0xff00) >> 8;
    uint8_t pinid = pid & 0xff;
    
    PortBus *p = findPortBus (i2cid);
    if (! p) {
        addBus (i2cid);
        p = findPortBus (i2cid);
    }
    if (p) {
        p->pinMode (pinid, INPUT);
    }
}

// --------------------------------------------------------------------------
void PortService::assignInterrupt (uint8_t irq, uint8_t bank) {
    if (bank) attachInterrupt (irq, portInterruptBank1, RISING);
    else attachInterrupt (irq, portInterruptBank0, RISING);
}

// --------------------------------------------------------------------------
void PortService::begin (void) {
    EventQueue.subscribe (SVC_PORT, this);
    EventQueue.subscribeTimer (SVC_PORT);
    for (uint8_t i=0; i<numports; ++i) ports[i]->begin();
}

// --------------------------------------------------------------------------
void PortService::pinOut (uint16_t id, uint8_t timeval) {
    PortBus *p = findPortBus ((id&0xff00)>>8);
    if (p) p->pinOut (id&0xff, timeval);
}

// --------------------------------------------------------------------------
void PortService::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
    if (id == EV_TIMER_TICK) tick++;
    for (uint8_t i=0; i<numports; ++i) {
        switch (id) {
            case EV_PORT_IRQ:
                ports[i]->handleInterrupt (X&1);
                break;
            
            case EV_TIMER_TICK:
                ports[i]->decreaseTimers();
                
                // Do pre-emptive polling in case we missed an
                // interrupt.
                if (! (tick & 63)) ports[i]->handleInterrupt (0);
                else if ((tick&63) == 32) ports[i]->handleInterrupt (1);
                break;
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
    EventQueue.subscribe (SVC_INPort, this);
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
