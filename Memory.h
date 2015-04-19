#ifndef _MEMORY_H
#define _MEMORY_H 1

class MemoryController
{
public:
    void scanForRAM (void);
    int available (void);
};

extern MemoryController Memory;

#endif
