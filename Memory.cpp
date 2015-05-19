#include <Arduino.h>
#include "Memory.h"

#ifdef BOARD_MEGA
#include <avr/io.h>
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

#endif

struct freelist {
        size_t sz;
        struct freelist *nx;
};

extern struct freelist *__flp;

void MemoryController::scanForRAM (void) {
#ifdef BOARD_MEGA
    XMCRB=1;
    XMCRA= 0x80;
    
    volatile uint8_t *ptr;
    volatile uint8_t *endptr = (uint8_t *) 0xff00;
    uint8_t pattern;
    uint8_t start = 0;
    bool retry = false;
    
    Serial.print (F("Checking for external SRAM on BOARD_MEGA\r\n"));
    Serial.print (F("Testing memory "));
    
    do {
        pattern = start;
        ptr = (uint8_t *) 0x2200;
        do {
            *ptr = pattern ^ (((uint16_t) ptr) >> 8);
            ptr++;
            pattern++;
        } while (ptr != (endptr+0xff));

        pattern = start;
        ptr = (uint8_t *) 0x2200;
        retry = false;
        do {
            if ((*ptr) != (pattern ^ (((uint16_t) ptr) >> 8))) {
                endptr = endptr - 256;
                if (endptr == (uint8_t *) 0x2200) {
                    Serial.print (F("\rTesting memory 8192 bytes"));
                    Serial.println (F("\r\nNo extended memory found"));
                    return;
                }
                retry = true;
                break;
            }
            pattern++;
            ptr++;
            if (((uint16_t)ptr & 255) == 0) {
            }
        } while (ptr != (endptr+0xff));
        
        if (! start) {
            Serial.print (F("\rTesting memory "));
            Serial.print ((uint16_t)endptr - 0x100);
            Serial.print (F(" bytes "));
            Serial.write (8);
        }
        if (! retry) start += 37;
    } while (retry || start > 36);

    struct freelist *fp;
    
    ptr = (uint8_t *) 0x2200;
    fp = (struct freelist *) 0x2200;
    fp->nx = NULL;
    fp->sz = (uint16_t) (endptr - ptr);
    
    fp = __flp;
    while (fp && fp->nx) fp = fp->nx;
    if (fp) fp->nx = (struct freelist *) 0x2200;
    else __flp = (struct freelist *) 0x2200;
    Serial.println (F("\r\nMemory test done"));
#endif
}

int MemoryController::available (void) {
#ifdef BOARD_MEGA
      int free_memory;

      if((int)__brkval == 0)
         free_memory = ((int)&free_memory) - ((int)&__bss_end);
      else
        free_memory = ((int)&free_memory) - ((int)__brkval);

    struct freelist *fp = __flp;
    while (fp) {
        free_memory += fp->sz;
        fp = fp->nx;
    }
    return free_memory;
#endif
    return 0;
}

MemoryController Memory;
