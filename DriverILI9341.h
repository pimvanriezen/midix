#ifndef _DRIVER_ILI9341P
#define _DRIVER_ILI9341P 1

#ifdef CONFIG_GFX_ILI9341SPI

#include "Display.h"
#include "EventQueue.h"

class DriverILI9341
{
public:
    static EventReceiver *load (void);
};

#endif
#endif
