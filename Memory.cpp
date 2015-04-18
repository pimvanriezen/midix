#include <Arduino.h>
#include "Memory.h"
#include <avr/io.h>

extern unsigned int __bss_end;

void MemoryController::scanForRAM (void) {
    XMCRB=1;
    XMCRA= 0x80;
    
    volatile uint8_t *ptr;
    volatile uint8_t *endptr = (uint8_t *) 0xff00;
    uint8_t pattern;
    uint8_t start = 0;
    bool retry = false;
    
    Serial.write ("Testing memory ");
    
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
                retry = true;
                break;
            }
            pattern++;
            ptr++;
            if (((uint16_t)ptr & 255) == 0) {
            }
        } while (ptr != (endptr+0xff));
        
        if (! start) {
            Serial.write ("\rTesting memory ");
            Serial.print ((uint16_t)endptr - 0x100);
            Serial.write (" bytes ");
            Serial.write (8);
        }
        if (! retry) start += 37;
    } while (retry || start > 36);

    Serial.println ("\r\nMemory test done");
    __malloc_heap_end = (char*) (endptr + 0xff);
    __bss_end = (unsigned int) (endptr + 0xff);
}

uint16_t MemoryController::available (void) {
    uint8_t *ptr = (uint8_t *) malloc (1);
    uint16_t res = ((uint16_t)__malloc_heap_end) - ((uint16_t)ptr);
    free (ptr);
    return res;
}

MemoryController Memory;
