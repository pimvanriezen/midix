#ifndef _CONSOLE_H
#define _CONSOLE_H 1

#include "EventQueue.h"

#define SVC_CONSOLE 1
#define EV_CONSOLE_OUT 1

/// Service handling console output.
class ConsoleService : public EventReceiver
{
public:
                 /// Constructor
                 ConsoleService (void);
                 
                 /// Destructor
                ~ConsoleService (void);
    
                 /// Set up serial device for output
    void         begin (void);
    
                 /// Handle events generated through write().
    void         handleEvent (eventtype, eventid, uint16_t, uint8_t, uint8_t);
    
                 /// Queue up console events to display a string.
    void         write (const char *);
};

extern ConsoleService Console;

#endif
