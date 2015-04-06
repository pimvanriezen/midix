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

void DisplayClient::setInk (uint8_t r, uint8_t g, uint8_t b) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_SETINK, r, g, b);
}

void DisplayClient::setCursor (uint16_t x, uint8_t y) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_SETCRSR, x, y);
}

void DisplayClient::setFont (uint8_t fontid) {
    EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_SETFONT, fontid);
}

void DisplayClient::write (const char *str, bool clear) {
    char c;
    
    while (c = *str++) {
        EventQueue.sendEvent (TYPE_REQUEST, SVC_GFX, GFX_DRAWCHAR, c,
                              clear ? 1 : 0);
    }
}

void DisplayClient::write (uint8_t i, bool clear) {
    char c[5];
    c[0] = '0' + i/100;
    c[1] = '0' + (i%100)/10;
    c[2] = '0' + (i%10);
    c[3] = ' ';
    c[4] = 0;
    write (c, clear);
}

DisplayClient Display;