#ifndef _DEBUG_H
#define _DEBUG_H 1

#define PIN_DEBUG 46
#define PIN_DEBUG_1 46
#define PIN_DEBUG_2 47
#define PIN_DEBUG_3 48

#ifdef DEBUG

#define DBG_ENTER(x) { digitalWrite (PIN_DEBUG + x, HIGH); }
#define DBG_LEAVE(x) { digitalWrite (PIN_DEBUG + x, LOW); }

#else

#define DBG_ENTER(x) {}
#define DBG_LEAVE(x) {}

#endif

#endif
