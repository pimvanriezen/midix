#include <Arduino.h>
#include "Console.h"

ConsoleService::ConsoleService (void) {
}

ConsoleService::~ConsoleService (void) {
}

void ConsoleService::begin (void) {
    Serial.println ("MIDIx console active\n");
    EventQueue.subscribe (SVC_CONSOLE, this);
}

void ConsoleService::handleEvent (eventtype tp, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z) {
    if (id == EV_CONSOLE_OUT) {
        monoduo xx;
        xx.wval = X;
        char out[6];
        out[5] = 0;
        out[0] = xx.val[0];
        out[1] = xx.val[1];
        out[2] = Y;
        out[3] = Z;
        Serial.write (out);
    }
}

void ConsoleService::write (const char *c) {
    monoduo X;
    uint8_t Y;
    uint8_t Z;
    while (*c) {
        X.wval = 0;
        Y = Z = 0;
        X.val[0] = *c++;
        if (*c) X.val[1] = *c++;
        if (*c) Y = *c++;
        if (*c) Z = *c++;
        EventQueue.sendEvent (TYPE_REQUEST, SVC_CONSOLE,
                              EV_CONSOLE_OUT, X.wval, Y, Z);
    }
}

ConsoleService Console;
