#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <Arduino.h>
#include "EventQueue.h"

#define SVC_GFX 0x40
#define GFX_BACKLIGHT 0x01
#define GFX_SETBG 0x02
#define GFX_CLEARBG 0x03
#define GFX_FILLRECT 0x04
#define GFX_SETINK 0x05
#define GFX_SETCRSR 0x06
#define GFX_SETFONT 0x07
#define GFX_DRAWCHAR 0x08
#define GFX_DRAWBOX 0x09
#define GFX_DRAWCIRCLE 0x0a

struct Font
{
    const uint8_t   *data;
    const uint16_t  *offsets;
    uint8_t          height;
};

extern Font *Fonts;

class DisplayClient
{
public:
                     DisplayClient (void);
                    ~DisplayClient (void);
    
    void             begin (EventReceiver *);
    void             backlightOn (void);
    void             backlightOff (void);
    void             setBackground (uint8_t, uint8_t, uint8_t);
    void             clearBackground (void);
    void             setInk (uint8_t, uint8_t, uint8_t);
    void             setCursor (uint16_t, uint8_t);
    void             setFont (uint8_t);
    void             write (const char *, bool clear=false);
    void             write (uint8_t, bool clear=false);
    void             write (uint16_t, bool clear=false);
};

extern DisplayClient Display;

#endif
