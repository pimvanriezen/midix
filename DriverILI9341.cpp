#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "helvetica24.h"
#include "helvetica11.h"
#include "DriverILI9341.h"
#include "Console.h"

#define PIN_BACKLIGHT 8

class MXScreen : public EventReceiver
{
public:
    MXScreen (Adafruit_ILI9341 *t) {
        tft = t;
        tft->begin();
        tft->setRotation (1);
        cursor_x = 0;
        cursor_y = 0;
        pinMode (PIN_BACKLIGHT, OUTPUT);
        backlightOff();
        EventQueue.subscribe (SVC_GFX, this);
    }
    
    ~MXScreen (void) {
    }
    
    void backlightOn (void) {
        digitalWrite (PIN_BACKLIGHT, HIGH);
    }
    
    void backlightOff (void) {
        digitalWrite (PIN_BACKLIGHT, LOW);
    }
    
    void setBackground (uint8_t r, uint8_t g, uint8_t b) {
        uint16_t bgcolor = 0;
        bg_r = r;
        bg_g = g;
        bg_b = b;
        inks[0] = tft->color565 (r,g,b);
    }

    void clearBackground (void) {
        for (uint16_t i=0; i<240; ++i) {
            for (uint16_t x=0; x<320; x+= 40) {
                tft->setAddrWindow (x,i,x+39,i);
                EventQueue.yield();
                tft->pushColor (inks[0], 40);
                EventQueue.yield();
            }
        }
    }
    
    void fillRect (uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        for (uint16_t yy=y; yy<y+h; ++yy) {
            tft->setAddrWindow (x,yy,x+w-1,yy);
            EventQueue.yield();
            tft->pushColor (inks[0], w);
            EventQueue.yield();
        }
    }
    
    void setInk (uint8_t r, uint8_t g, uint8_t b) {
        uint8_t tr, tg, tb;
        tr = cblend (r, bg_r, 0x48);
        tg = cblend (g, bg_g, 0x48);
        tb = cblend (b, bg_b, 0x48);
        inks[1] = tft->color565 (tr,tg,tb);

        tr = cblend (r, bg_r, 0x80);
        tg = cblend (g, bg_g, 0x80);
        tb = cblend (b, bg_b, 0x80);
        inks[2] = tft->color565 (tr,tg,tb);
        inks[3] = tft->color565 (r, g, b);
    }
    
    void setCursor (uint16_t x, uint16_t y) {
        cursor_x = x;
        cursor_y = y;
    }
    
    void setFont (uint8_t fontid) {
        const uint8_t *font = Fonts[fontid].data;
        const uint16_t *offs = Fonts[fontid].offsets;
        uint8_t h = Fonts[fontid].height;
        font_data = font;
        font_offs = offs;
        font_height = h;
    }
    
    void print (const char *c) {
        while (*c) drawChar (*c++);
    }
    
    void drawBox (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
        tft->drawFastHLine (x1,y1,x2-x1,inks[3]);
        tft->drawFastHLine (x1,y2,x2-x1,inks[3]);
        tft->drawFastVLine (x1,y1,y2-y1,inks[3]);
        tft->drawFastVLine (x2,y1,y2-y1,inks[3]);
    }
    
    void drawCircle (uint16_t x, uint16_t y, uint16_t r) {
        tft->drawCircle (x,y,r+1,inks[1]);
        tft->fillCircle (x,y,r,inks[3]);
    }
    
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
    
    void drawText (uint16_t width, const char *txt) {
        uint16_t twidth = 0;
        const char *c = txt;
        while (*c) twidth += textWidth (*c++);
        if (twidth < width) cursor_x += (width-twidth)/2;
        print (txt);
    }
    
    void drawChar (char c, bool clear=false) {
        if (c<32) c = ' ';
        else if (c>128) c = ' ';
        if (c == ' ') {
            if (clear)
            {
                tft->setAddrWindow (cursor_x, cursor_y,
                                    cursor_x+font_height/3,
                                    cursor_y + font_height-1);
                
                for (uint8_t y=0; y<font_height;++y) {
                    for (uint8_t x=0; x<((font_height/3)+1); ++x) {
                        tft->pushColor (inks[0]);
                    }
                    EventQueue.yield();
                }
            }
            cursor_x += font_height/3;
            return;
        }
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
                tft->pushColor (inks[pix]);
            }
            tft->pushColor (inks[0]);
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
                fillRect (xx.val[0], Y, xx.val[1], Z);
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
                drawBox (xx.val[0], Y, xx.val[1], Z);
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
    Adafruit_ILI9341 *tft;
};

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

EventReceiver *DriverILI9341::load (void) {
    // Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
    Adafruit_ILI9341 *tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
    return new MXScreen (tft);
}
