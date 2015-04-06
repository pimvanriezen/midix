#ifndef _DRIVER_ILI9341
#define _DRIVER_ILI9341 1

#include "Display.h"
#include "EventQueue.h"

class DriverILI9341
{
public:
    static EventReceiver *load (void);
};

#endif
