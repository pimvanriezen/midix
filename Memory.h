#ifndef _MEMORY_H
#define _MEMORY_H 1

class MemoryController
{
public:
    void scanForRAM (void);
    uint16_t available (void);
};

extern MemoryController Memory;

#endif
