#ifndef _DRIVER_ILI9341P
#define _DRIVER_ILI9341P 1

#include "Display.h"
#include "EventQueue.h"

class DriverILI9341P
{
public:
    static EventReceiver *load (void);
};

#endif
