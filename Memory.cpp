#include <Arduino.h>
#include "Memory.h"
#include <avr/io.h>

extern unsigned int __bss_end;

bool MemoryController::initializeExternalRAM (uint16_t sz) {
    XMCRB=1;
    XMCRA= 0x80;
    
    volatile uint8_t *ptr = (volatile uint8_t *) 0x2200;
    
    *ptr = 0x55;
    
    while (ptr < (uint8_t *) (0x2200+sz)) {
        *ptr++ = 0x55;
    }

    ptr = (uint8_t *) 0x2200;
    while (ptr < (uint8_t *) (0x2200+sz)) {
        if ((*ptr) != 0x55) return false;
        ptr++;
    }
    
    ptr = (volatile uint8_t *) 0x2200;
    while (ptr < (uint8_t *) (0x2200+sz)) {
        *ptr++ = 0xaa;
    }

    ptr = (uint8_t *) 0x2200;
    while (ptr < (uint8_t *) (0x2200+sz)) {
        if ((*ptr) != 0xaa) return false;
        ptr++;
    }

    __malloc_heap_end = (char*) ptr;
    __bss_end = (unsigned int) ptr;
    return true;
}

uint16_t MemoryController::available (void) {
    uint8_t *ptr = (uint8_t *) malloc (1);
    uint16_t res = ((uint16_t)__malloc_heap_end) - ((uint16_t)ptr);
    free (ptr);
    return res;
}

MemoryController Memory;
