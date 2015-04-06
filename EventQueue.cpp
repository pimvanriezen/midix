#include <Arduino.h>
#include "EventQueue.h"

// ==========================================================================
// CLASS EventQueueManager
// ==========================================================================
EventQueueManager::EventQueueManager (void) {
    numsubscribers = 0;
    lbuf.ring.wpos = lbuf.ring.rpos = 0;
    hbuf.ring.wpos = hbuf.ring.rpos = 0;
    for (uint8_t i=0; i<64; ++i) {
        lbuf.ring.rbuf[i].type = TYPE_NONE;
        lbuf.ring.rbuf[i].id = 0;
        lbuf.ring.rbuf[i].X.wval = 0;
        lbuf.ring.rbuf[i].Y = lbuf.ring.rbuf[i].Z = 0;
    }
    for (uint8_t i=0; i<8; ++i) {
        hbuf.ring.rbuf[i].type = TYPE_NONE;
        hbuf.ring.rbuf[i].id = 0;
        hbuf.ring.rbuf[i].X.wval = 0;
        hbuf.ring.rbuf[i].Y = lbuf.ring.rbuf[i].Z = 0;
    }
    timeractive = false;
    pinMode (13, OUTPUT);
    digitalWrite (13, LOW);
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
        lbuf.ring.wpos = (lbuf.ring.wpos+1) & 63;
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
        hbuf.ring.wpos = (hbuf.ring.wpos+1) & 7;
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
    while (true) {
        if (hbuf.ring.rpos != hbuf.ring.wpos) {
            volatile Event *ev = &hbuf.ring.rbuf[hbuf.ring.rpos];
            hbuf.ring.rpos = (hbuf.ring.rpos+1) & 7;
            for (uint8_t i=0; i<numsubscribers; ++i) {
                if (subscribers[i].id == ev->service) {
                    subscribers[i].svc->handleEvent (ev->type, ev->id,
                                                     ev->X.wval,
                                                     ev->Y, ev->Z);
                    break;
                }
            }
        }
        else if (timeractive && ((ts=millis()) < timernext)) {
            for (uint8_t i=0; i<timerclientcount; ++i) {
                sendEvent (TYPE_TIMER, timerclients[i],
                           EV_TIMER_TICK);
            }
            timernext += timerperiod;
        }
        else break;
    }
}

// --------------------------------------------------------------------------
volatile Event *EventQueueManager::waitEvent (void) {
    uint8_t i;
    bool matched;
    while (1) {
        while (hbuf.ring.rpos == hbuf.ring.wpos &&
               ((! timeractive) || ((ts=millis()) < timernext)) &&
               lbuf.ring.rpos == lbuf.ring.wpos) {
        }
        
        //digitalWrite (13, HIGH);
        volatile Event *ev;
        if (hbuf.ring.rpos != hbuf.ring.wpos) {
            ev = &hbuf.ring.rbuf[hbuf.ring.rpos];
            hbuf.ring.rpos = (hbuf.ring.rpos+1) & 7;
        }
        else if (ts >= timernext) {
            for (i=0; i<timerclientcount; ++i) {
                sendEvent (TYPE_TIMER, timerclients[i],
                           EV_TIMER_TICK);
            }
            timernext += timerperiod;
            continue;
        }
        else {
            ev = &lbuf.ring.rbuf[lbuf.ring.rpos];
            lbuf.ring.rpos = (lbuf.ring.rpos+1) & 63;
        }
        matched = false;
        for (i=0; i<numsubscribers; ++i) {
            if (subscribers[i].id == ev->service) {
                subscribers[i].svc->handleEvent (ev->type, ev->id,
                                                 ev->X.wval,
                                                 ev->Y, ev->Z);
                matched = true;
                break;
            }
        }
        if (! matched) return ev;
    }
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
    Serial.println ("misfire");
}
