#include <Arduino.h>
#include "EventQueue.h"
#include "Display.h"

DisplayClient::DisplayClient (void) {
}

DisplayClient::~DisplayClient (void) {
}

void DisplayClient::begin (EventReceiver *r) {
}

void DisplayClient::backlightOn (void) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_BACKLIGHT, 1);
}

void DisplayClient::backlightOff (void) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_BACKLIGHT, 0);
}

void DisplayClient::setBackground (uint8_t r, uint8_t g, uint8_t b) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_SETBG, r, g, b);
}

void DisplayClient::clearBackground (void) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_CLEARBG);
}

DisplayClient Display;
