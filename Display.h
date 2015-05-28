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
#define GFX_CLEARRECT 0x0b

/// Representation of a 2-bit font
struct Font
{
    const uint8_t   *data;      ///< Raw data stream
    const uint16_t  *offsets;   ///< Offset table
    uint8_t          height;    ///< General font height
};

/// Global font list
extern Font *Fonts;

/// Client class for the TFT display service
class DisplayClient
{
public:
                     DisplayClient (void);
                    ~DisplayClient (void);

                     /// Initialize with a specific driver / service.    
    void             begin (EventReceiver *);
    
                     /// Turn the TFT backlight on
    void             backlightOn (void);
    
                     /// Turn the TFT backlight off
    void             backlightOff (void);
    
                     /// Set the background colour for draw operations.
    void             setBackground (uint8_t r, uint8_t g, uint8_t b);
    
                     /// Clear the screen with the background colour.
    void             clearBackground (void);
    
                     /// Fill a rectangle with the foreground colour.
                     /// \param x Left coordinate, note that it gets rounded
                     ///          to an even number.
                     /// \param y Top coordinate.
                     /// \param w Width (max 255).
                     /// \param h Height
    void             fillRect (uint16_t x, uint8_t y, uint8_t w, uint8_t h);

                     /// Fill a rectangle with the background colour.
                     /// \param x Left coordinate, note that it gets rounded
                     ///          to an even number.
                     /// \param y Top coordinate.
                     /// \param w Width (max 255).
                     /// \param h Height
    void             clearRect (uint16_t x, uint8_t y, uint8_t w, uint8_t h);

                     /// Draw a rectangle with the foreground colour.
                     /// \param x Left coordinate, note that it gets rounded
                     ///          to an even number.
                     /// \param y Top coordinate.
                     /// \param w Width (max 255).
                     /// \param h Height
    void             drawBox (uint16_t x, uint8_t y, uint8_t w, uint8_t h);
    
                     /// Set foreground colour. This will also create
                     /// 2 extra blend colours with the background for
                     /// font anti-aliasing.
    void             setInk (uint8_t r, uint8_t g, uint8_t b);
    
                     /// Sets the font for text drawing.
    void             setFont (uint8_t id);
    
                     /// Set cursor for text drawing.
    void             setCursor (uint16_t x, uint8_t y);
    
                     /// Write text.
    void             write (const char *, bool clear=false);
    
                     /// Write a decimal byte value
    void             write (uint8_t, bool clear=false);
    
                     /// Write a decimal word value.
    void             write (uint16_t, bool clear=false);
};

/// Global instance
extern DisplayClient Display;

#endif
