#ifndef _APPLICATION_H
#define _APPLICATION_H 1

#include <Arduino.h>
#include "EventQueue.h"

class Application : public EventReceiver
{
public:
                     Application (void);
    virtual         ~Application (void);
    virtual void     setup (void);
    virtual void     start (void);
    virtual void     handleEvent (eventtype t, eventid id, uint16_t X,
                                  uint8_t Y, uint8_t Z);
};

void setup (void);
void loop (void);

extern Application *App;

#endif
