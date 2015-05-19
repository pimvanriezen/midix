#include "IRQ.h"

#define DEFISR(XX) void isr ## XX (void) { IRQ.call (XX); }

DEFISR(0)
DEFISR(1)
DEFISR(2)
DEFISR(3)
DEFISR(4)
DEFISR(5)
DEFISR(6)
DEFISR(7)

InterruptCatcher::InterruptCatcher (void) {
    count = 0;
}

InterruptCatcher::~InterruptCatcher (void) {
}

#define ATTACHISR(irqid, XX) { attachInterrupt (irqid, isr ## XX, RISING); }

void InterruptCatcher::assign (uint8_t id, genisrptr func) {
    if (count>7) return;
    genisr[count] = func;
    irqid[count] = id;
    switch (count) {
        case 0: ATTACHISR(id,0); break;
        case 1: ATTACHISR(id,1); break;
        case 2: ATTACHISR(id,2); break;
        case 3: ATTACHISR(id,3); break;
        case 4: ATTACHISR(id,4); break;
        case 5: ATTACHISR(id,5); break;
        case 6: ATTACHISR(id,6); break;
        case 7: ATTACHISR(id,7); break;
    }
    count++;
}

void InterruptCatcher::call (uint8_t idx) {
    if (idx<count) genisr[idx](irqid[idx]);
}

InterruptCatcher IRQ;
