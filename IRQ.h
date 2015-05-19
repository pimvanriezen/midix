#ifndef _IRQ_H
#define _IRQ_H 1

#include <Arduino.h>

typedef void (*isrptr)(void);
typedef void (*genisrptr)(uint8_t);

class InterruptCatcher
{
public:
                     InterruptCatcher (void);
                    ~InterruptCatcher (void);
    
    void             assign (uint8_t irqid, genisrptr func);
    void             call (uint8_t idx);

protected:
    genisrptr        genisr[8];
    uint8_t          irqid[8];
    uint8_t          count;
};

extern InterruptCatcher IRQ;

#endif
