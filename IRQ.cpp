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
DEFISR(8)
DEFISR(9)
DEFISR(10)
DEFISR(11)
DEFISR(12)
DEFISR(13)
DEFISR(14)
DEFISR(15)

// ==========================================================================
// CLASS InterruptCatcher
// ==========================================================================
InterruptCatcher::InterruptCatcher (void) {
    count = 0;
}

// --------------------------------------------------------------------------
InterruptCatcher::~InterruptCatcher (void) {
}

// --------------------------------------------------------------------------
#define ATTACHISR(irqid, XX) { attachInterrupt (irqid, isr ## XX, RISING); }

void InterruptCatcher::assign (uint8_t id, genisrptr func) {
    if (count>15) return;
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
        case 8: ATTACHISR(id,8); break;
        case 9: ATTACHISR(id,9); break;
        case 10: ATTACHISR(id,10); break;
        case 11: ATTACHISR(id,11); break;
        case 12: ATTACHISR(id,12); break;
        case 13: ATTACHISR(id,13); break;
        case 14: ATTACHISR(id,14); break;
        case 15: ATTACHISR(id,15); break;
    }
    count++;
}

// --------------------------------------------------------------------------
void InterruptCatcher::call (uint8_t idx) {
    if (idx<count) {
        genisr[idx](irqid[idx]);
    }
}

// --------------------------------------------------------------------------
InterruptCatcher IRQ;
