#ifndef _MEMORY_H
#define _MEMORY_H 1

class MemoryController
{
public:
    bool initializeExternalRAM (uint16_t sz);
    uint16_t available (void);
};

extern MemoryController Memory;

#endif
