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
    if (! i) {
        write ("0", clear);
        return;
    }
    char c[5];
    uint8_t cpos = 0;
    if (i>99) c[cpos++] = '0' + i/100;
    if (i>9) c[cpos++] = '0' + (i%100)/10;
    c[cpos++] = '0' + i%10;
    while (cpos<4) c[cpos++] = ' ';
    c[4] = 0;
    write (c, clear);
}

void DisplayClient::write (uint16_t val, bool clear) {
    if (! val) {
        write ("0", clear);
        return;
    }

    uint16_t rest, v;
    uint8_t cpos = 0;
    char c[10];
    rest = val;
    
    v = rest/10000;
    c[cpos++] = '0' + v;
    rest = rest % 10000;
    
    v = rest/1000;
    c[cpos++] = '0' + v;
    rest = rest % 1000;
    
    v = rest/100;
    c[cpos++] = '0' + v;
    rest = rest % 100;
    
    v = rest/10;
    c[cpos++] = '0' + v;
    rest = rest % 10;
    
    c[cpos++] = '0' + rest;
    while (cpos<9) c[cpos++] = ' ';
    c[cpos] = 0;
    cpos = 0;
    while (c[cpos] == '0') cpos++;
    write (c+cpos, clear);
}

DisplayClient Display;
