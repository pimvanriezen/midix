#ifdef CONFIG_GFX_ILI9341P

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTLCD.h"
#include "helvetica24.h"
#include "helvetica11.h"
#include "DriverILI9341P.h"
#include "Console.h"
#include "Hardware.h"

/// Wrapper service for the Adafruit TFTLCD Library
class ILI9341ParallelService : public EventReceiver
{
public:
    //  ---------------------------------------------------------------------
    /// Constructor
    ILI9341ParallelService (Adafruit_TFTLCD *t) {
        tft = t;
        tft->begin(0x9341);
        tft->setRotation (1);
        cursor_x = 0;
        cursor_y = 0;
        pinMode (PIN_TFT_LIGHT, OUTPUT);
        backlightOff();
        EventQueue.subscribe (SVC_GFX, this);
    }
    
    //  ---------------------------------------------------------------------
    /// Destructor
    ~ILI9341ParallelService (void) {
    }
    
    //  ---------------------------------------------------------------------
    /// Turn on backlight
    void backlightOn (void) {
        digitalWrite (PIN_TFT_LIGHT, HIGH);
    }

    //  ---------------------------------------------------------------------
    /// Turn off backlight    
    void backlightOff (void) {
        digitalWrite (PIN_TFT_LIGHT, LOW);
    }
    
    //  ---------------------------------------------------------------------
    /// Set the background colour
    void setBackground (uint8_t r, uint8_t g, uint8_t b) {
        uint16_t bgcolor = 0;
        bg_r = r;
        bg_g = g;
        bg_b = b;
        inks[0] = tft->color565 (r,g,b);
    }

    //  ---------------------------------------------------------------------
    /// Clear the screen with the background colour. Will yield() back to
    /// the EventQueue 16 times per line on the TFT.
    void clearBackground (void) {
        bool first = true;
        for (uint16_t i=0; i<240; ++i) {
            for (uint16_t x=0; x<320; x+= 40) {
                tft->setAddrWindow (x,i,x+39,i);
                EventQueue.yield();
                tft->flood (inks[0], 40);
                first = false;
                EventQueue.yield();
            }
        }
    }
    
    //  ---------------------------------------------------------------------
    /// Fill a rectangle with the foreground colour. Will yield() back to
    /// the EventQueue twice for every line on the LCD.
    void fillRect (uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        for (uint16_t yy=y; yy<y+h; ++yy) {
            tft->setAddrWindow (x,yy,x+w-1,yy);
            EventQueue.yield();
            tft->flood (inks[3], w);
            EventQueue.yield();
        }
    }
    
    //  ---------------------------------------------------------------------
    /// Clear a rectangle with the background colour.
    void clearRect (uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        for (uint16_t yy=y; yy<y+h; ++yy) {
            tft->setAddrWindow (x,yy,x+w-1,yy);
            EventQueue.yield();
            tft->flood (inks[0], w);
            EventQueue.yield();
        }
    }
    
    //  ---------------------------------------------------------------------
    /// Set the foreground ink colour. Will recalculate blendings for
    /// quasi-antialiased font rendering.
    void setInk (uint8_t r, uint8_t g, uint8_t b) {
        uint8_t tr, tg, tb;
        tr = cblend (r, bg_r, 0x40);
        tg = cblend (g, bg_g, 0x40);
        tb = cblend (b, bg_b, 0x40);
        inks[1] = tft->color565 (tr,tg,tb);

        tr = cblend (r, bg_r, 0x80);
        tg = cblend (g, bg_g, 0x80);
        tb = cblend (b, bg_b, 0x80);
        inks[2] = tft->color565 (tr,tg,tb);
        inks[3] = tft->color565 (r, g, b);
    }
    
    //  ---------------------------------------------------------------------
    /// Set cursor position for text output
    void setCursor (uint16_t x, uint16_t y) {
        cursor_x = x;
        cursor_y = y;
    }
    
    //  ---------------------------------------------------------------------
    /// Set font for text output. Uses the global Fonts array
    /// to retrieve font information by font number. Caller is responsible
    /// for this to contain useful data.
    void setFont (uint8_t fontid) {
        const uint8_t *font = Fonts[fontid].data;
        const uint16_t *offs = Fonts[fontid].offsets;
        uint8_t h = Fonts[fontid].height;
        font_data = font;
        font_offs = offs;
        font_height = h;
    }
    
    //  ---------------------------------------------------------------------
    /// Print a string to the screen at the current position.
    void print (const char *c) {
        while (*c) drawChar (*c++);
    }
    
    //  ---------------------------------------------------------------------
    /// Draw a box with the foreground colour.
    void drawBox (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
        tft->drawFastHLine (x1,y1,x2-x1,inks[3]);
        EventQueue.yield();
        tft->drawFastHLine (x1,y2,x2-x1+1,inks[3]);
        EventQueue.yield();
        tft->drawFastVLine (x1,y1,y2-y1,inks[3]);
        EventQueue.yield();
        tft->drawFastVLine (x2,y1,y2-y1+1,inks[3]);
        EventQueue.yield();
    }
    
    //  ---------------------------------------------------------------------
    /// Draw a filled circle with the foreground colour.
    void drawCircle (uint16_t x, uint16_t y, uint16_t r) {
        tft->drawCircle (x,y,r+1,inks[1]);
        tft->fillCircle (x,y,r,inks[3]);
    }
    
    //  ---------------------------------------------------------------------
    /// Calculate the width of a character in the selected font.
    uint16_t textWidth (char c) {
        if (c<32) c = ' ';
        else if (c>128) c = ' ';
        if (c == ' ') return font_height/3;
        c = c-32;
        const uint8_t *crsr = font_data + pgm_read_word (font_offs+c);
        uint16_t bitpos = 0;
        crsr += 2;
        uint8_t c_width = pgm_read_byte (crsr++);
        return (uint16_t) c_width+1;
    }
    
    //  ---------------------------------------------------------------------
    void drawChar (char c, bool clear=false) {
    
        // Map crap to whitespace.
        if (c<32) c = ' ';
        else if (c>128) c = ' ';
        
        // Draw whitespace
        if (c == ' ') {
            if (clear)
            {
                bool f = true;
                tft->setAddrWindow (cursor_x, cursor_y,
                                    cursor_x+font_height/3,
                                    cursor_y + font_height-1);
                
                for (uint8_t y=0; y<font_height;++y) {
                    for (uint8_t x=0; x<((font_height/3)+1); ++x) {
                        tft->pushColor (inks[0], f);
                        f = false;
                    }
                    EventQueue.yield();
                }
            }
            cursor_x += font_height/3;
            return;
        }
        
        // Look up the character by offset
        c = c-32;
        const uint8_t *crsr __attribute__((progmem)) = 
                                font_data + pgm_read_word(font_offs+c);
        uint16_t bitpos = 0;
        uint8_t c_yoffs = pgm_read_byte (crsr++);
        uint8_t c_height = pgm_read_byte (crsr++);
        uint8_t c_width = pgm_read_byte (crsr++);

        tft->setAddrWindow (cursor_x, cursor_y,
                            cursor_x + c_width,
                            cursor_y + font_height - 1);
        bool f=true; // True if another colour follows

        for (uint8_t y=0; y<font_height; ++y) {
            for (uint8_t x=0;x<c_width; ++x) {
                uint8_t pix = 0;
                if (y>=c_yoffs && y<(c_height+c_yoffs)) {
                    switch (bitpos) {
                        case 0:
                            pix = (pgm_read_byte(crsr) & 0xc0) >> 6;
                            break;
                    
                        case 2:
                            pix = (pgm_read_byte(crsr) & 0x30) >> 4;
                            break;
                        
                        case 4:
                            pix = (pgm_read_byte(crsr) & 0x0c) >> 2;
                            break;
                    
                        case 6:
                            pix = (pgm_read_byte(crsr) & 0x03);
                            break;
                    }
                    bitpos += 2;
                    if (bitpos>7) {
                        crsr++;
                        bitpos-=8;
                    }
                }
                tft->pushColor (inks[pix], f);
                f = false;
            }
            tft->pushColor (inks[0], f);
            EventQueue.yield();
        }
        
        cursor_x += c_width+1;
    }
    
    void handleEvent (eventtype tp, eventid id, uint16_t X,
                      uint8_t Y, uint8_t Z) {
        char xstr[2];
        xstr[0] = X&127;
        xstr[1] = 0;
        monoduo xx;
        xx.wval = X;
        switch (id) {
            case GFX_BACKLIGHT:
                if (X) backlightOn();
                else backlightOff();
                break;
            
            case GFX_SETBG:
                setBackground (X&255, Y, Z);
                break;
            
            case GFX_CLEARBG:
                clearBackground();
                break;
                
            case GFX_FILLRECT:
                fillRect ((uint16_t)xx.val[0]*2, Y, xx.val[1], Z);
                break;

            case GFX_CLEARRECT:
                clearRect ((uint16_t)xx.val[0]*2, Y, xx.val[1], Z);
                break;
            
            case GFX_SETINK:
                setInk (X&255, Y, Z);
                break;
            
            case GFX_SETCRSR:
                setCursor (X, Y);
                break;
            
            case GFX_SETFONT:
                setFont (X);
                break;
            
            case GFX_DRAWCHAR:
                drawChar ((char) X&127, Y==1);
                break;
            
            case GFX_DRAWBOX:
                drawBox ((uint16_t)xx.val[0]*2, Y, 
                         ((uint16_t)xx.val[0]*2) + xx.val[1], Y+Z);
                break;
            
            case GFX_DRAWCIRCLE:
                drawCircle (X,Y,Z);
                break;
                
            default:
                break;
        }
    }

protected:

    uint8_t cblend (uint8_t ink, uint8_t bg, uint8_t alpha) {
        uint16_t acc = 0;
        acc += (ink * alpha);
        acc += (bg * (255 - alpha));
        return (acc >> 8);
    }

    uint8_t bg_r, bg_g, bg_b;
    uint16_t inks[4];
    uint16_t cursor_x, cursor_y;
    const uint8_t *font_data;
    const uint16_t *font_offs;
    uint8_t font_height;
    Adafruit_TFTLCD *tft;
};

// --------------------------------------------------------------------------
EventReceiver *DriverILI9341P::load (void) {
    Adafruit_TFTLCD *tft = new Adafruit_TFTLCD(PIN_TFT_CS, PIN_TFT_CD,
                                               PIN_TFT_WR, PIN_TFT_RD, 
                                               PIN_TFT_RST);
    return new ILI9341ParallelService (tft);
}

#endif
