#ifndef _IRQ_H
#define _IRQ_H 1

#include <Arduino.h>

/// Type for interrupt callbacks with an argument
typedef void (*genisrptr)(uint8_t);

/// This class uses a pool of argument-less ISR functions to dispatch
/// certain interrupts to a callback function that takes the interrupt
/// number as an argument.
class InterruptCatcher
{
public:
                     /// Constructor.
                     InterruptCatcher (void);
                     
                     /// Destructor.
                    ~InterruptCatcher (void);
    
                     /// Register an irq-number to a callback function.
                     /// \param irqid The irq-number (pin on Due).
                     /// \param func The callback function.
    void             assign (uint8_t irqid, genisrptr func);

                     /// Dispatcher, called from an internal pool of
                     /// ISR functions.
    void             call (uint8_t idx);

protected:
    genisrptr        genisr[8]; ///< List of callbacks
    uint8_t          irqid[8]; ///< List of irq-numbers
    uint8_t          count; ///< Number of items in the list (max 8).
};

/// Global instance.
extern InterruptCatcher IRQ;

#endif
