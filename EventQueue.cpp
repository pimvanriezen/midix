#include <Arduino.h>
#include "EventQueue.h"
#include "Debug.h"

// ==========================================================================
// CLASS EventQueueManager
// ==========================================================================
EventQueueManager::EventQueueManager (void) {
    missedTicks = 0;
    numsubscribers = 0;
    lbuf.ring.wpos = lbuf.ring.rpos = 0;
    hbuf.ring.wpos = hbuf.ring.rpos = 0;
    for (uint8_t i=0; i<SZ_LOBUF; ++i) {
        lbuf.ring.rbuf[i].type = TYPE_NONE;
        lbuf.ring.rbuf[i].id = 0;
        lbuf.ring.rbuf[i].X.wval = 0;
        lbuf.ring.rbuf[i].Y = lbuf.ring.rbuf[i].Z = 0;
    }
    for (uint8_t i=0; i<SZ_HIBUF; ++i) {
        hbuf.ring.rbuf[i].type = TYPE_NONE;
        hbuf.ring.rbuf[i].id = 0;
        hbuf.ring.rbuf[i].X.wval = 0;
        hbuf.ring.rbuf[i].Y = lbuf.ring.rbuf[i].Z = 0;
    }
    timeractive = false;

    pinMode (PIN_DEBUG_1, OUTPUT);
    pinMode (PIN_DEBUG_2, OUTPUT);
    pinMode (PIN_DEBUG_3, OUTPUT);
    digitalWrite (PIN_DEBUG_1, LOW);
    digitalWrite (PIN_DEBUG_2, LOW);
    digitalWrite (PIN_DEBUG_3, LOW);
}

// --------------------------------------------------------------------------
EventQueueManager::~EventQueueManager (void) {
}

// --------------------------------------------------------------------------
void EventQueueManager::sendEvent (eventtype tp, serviceid svc, eventid id,
                                   uint16_t x, uint8_t y, uint8_t z) {
    if (tp == TYPE_REQUEST) {
        cli();
        uint8_t wpos = lbuf.ring.wpos;
        lbuf.ring.wpos = (lbuf.ring.wpos+1) & (SZ_LOBUF-1);
        sei();
        lbuf.ring.rbuf[wpos].type = tp;
        lbuf.ring.rbuf[wpos].service = svc;
        lbuf.ring.rbuf[wpos].id = id;
        lbuf.ring.rbuf[wpos].X.wval = x;
        lbuf.ring.rbuf[wpos].Y = y;
        lbuf.ring.rbuf[wpos].Z = z;
    }
    else {
        cli();
        uint8_t wpos = hbuf.ring.wpos;
        hbuf.ring.wpos = (hbuf.ring.wpos+1) & (SZ_HIBUF-1);
        sei();
        hbuf.ring.rbuf[wpos].type = tp;
        hbuf.ring.rbuf[wpos].service = svc;
        hbuf.ring.rbuf[wpos].id = id;
        hbuf.ring.rbuf[wpos].X.wval = x;
        hbuf.ring.rbuf[wpos].Y = y;
        hbuf.ring.rbuf[wpos].Z = z;
    }
}

// --------------------------------------------------------------------------
void EventQueueManager::yield (void) {
    DBG_ENTER(0);
    if (hbuf.ring.rpos != hbuf.ring.wpos) {
        volatile Event *ev = &hbuf.ring.rbuf[hbuf.ring.rpos];
        hbuf.ring.rpos = (hbuf.ring.rpos+1) & (SZ_HIBUF-1);
        for (uint8_t i=0; i<numsubscribers; ++i) {
            if (subscribers[i].id == ev->service) {
                DBG_LEAVE(0);
                subscribers[i].svc->handleEvent (ev->type, ev->id,
                                                 ev->X.wval,
                                                 ev->Y, ev->Z);
                DBG_ENTER(0);
                break;
            }
        }
    }
    if (timeractive && ((ts=millis()) >= timernext)) {
        for (uint8_t i=0; i<timerclientcount; ++i) {
            for (uint8_t j=0; j<numsubscribers; ++j) {
                if (subscribers[j].id == timerclients[i]) {
                    DBG_LEAVE(0);
                    subscribers[j].svc->handleEvent (TYPE_TIMER,
                                                     EV_TIMER_TICK,
                                                     (uint16_t) 0,
                                                     (uint8_t) 0,
                                                     (uint8_t) 0);
                    DBG_ENTER(0);
                }
            }
        }
        timernext += timerperiod;
        if (ts>=timernext) timernext += timerperiod;
        while (ts>=timernext) {
            timernext += timerperiod;
            missedTicks++;
        }
    }
    DBG_LEAVE(0);
}

// --------------------------------------------------------------------------
volatile Event *EventQueueManager::waitEvent (bool forever) {
    uint8_t i;
    bool matched;
    while (forever) {
        while (forever && hbuf.ring.rpos == hbuf.ring.wpos &&
               ((! timeractive) || ((ts=millis()) < timernext)) &&
               lbuf.ring.rpos == lbuf.ring.wpos) {
        }
        
        DBG_ENTER(0);
        //digitalWrite (13, HIGH);
        volatile Event *ev;
        if (hbuf.ring.rpos != hbuf.ring.wpos) {
            ev = &hbuf.ring.rbuf[hbuf.ring.rpos];
            hbuf.ring.rpos = (hbuf.ring.rpos+1) & (SZ_HIBUF-1);
        }
        else if (ts >= timernext) {
            DBG_ENTER(1);
            for (i=0; i<timerclientcount; ++i) {
                sendEvent (TYPE_TIMER, timerclients[i],
                           EV_TIMER_TICK);
            }
            timernext += timerperiod;
            if (ts>=timernext) timernext += timerperiod;
            while (ts>=timernext) {
                timernext += timerperiod;
                missedTicks++;
            }
            DBG_LEAVE(1);
            DBG_LEAVE(0);
            continue;
        }
        else if (lbuf.ring.rpos != lbuf.ring.wpos) {
            ev = &lbuf.ring.rbuf[lbuf.ring.rpos];
            lbuf.ring.rpos = (lbuf.ring.rpos+1) & (SZ_LOBUF-1);
        }
        else {
            if (forever) continue;
            return NULL;
        }
        matched = false;
        for (i=0; i<numsubscribers; ++i) {
            if (subscribers[i].id == ev->service) {
                DBG_LEAVE(0);
                DBG_ENTER(2);
                subscribers[i].svc->handleEvent (ev->type, ev->id,
                                                 ev->X.wval,
                                                 ev->Y, ev->Z);
                matched = true;
                DBG_LEAVE(2);
                DBG_ENTER(0);
                break;
            }
        }
        if (! matched) {
            DBG_LEAVE(0);
            return ev;
        }
    }
    return NULL;
}

// --------------------------------------------------------------------------
void EventQueueManager::subscribe (serviceid svcid, EventReceiver *svc) {
    if (numsubscribers<16) {
        subscribers[numsubscribers].id = svcid;
        subscribers[numsubscribers].svc = svc;
        numsubscribers++;
    }
}

// --------------------------------------------------------------------------
void EventQueueManager::subscribeTimer (serviceid svcid) {
    if (timerclientcount<4) {
        timerclients[timerclientcount++] = svcid;
    }
}

// --------------------------------------------------------------------------
void EventQueueManager::startTimer (uint16_t p) {
    ts = millis();
    timerperiod = p;
    timernext = ts + p;
    timeractive = true;
}
        
EventQueueManager EventQueue;

// ==========================================================================
// CLASS EventReceiver
// ==========================================================================
EventReceiver::EventReceiver (void) {
}

// --------------------------------------------------------------------------
EventReceiver::~EventReceiver (void) {
}

// --------------------------------------------------------------------------
void EventReceiver::handleEvent (eventtype e, eventid id, uint16_t X,
                                 uint8_t Y, uint8_t Z) {
    Serial.println (F("misfire"));
}
