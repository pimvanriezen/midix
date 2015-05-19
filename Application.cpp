#include "Application.h"
#include "Console.h"
#include "I2C.h"
#include "Port.h"
#include "EventQueue.h"
#include "Memory.h"

// ==========================================================================
// CLASS Application
// ==========================================================================
Application::Application (void) {
    App = this;
}

// --------------------------------------------------------------------------
Application::~Application (void) {
}

// --------------------------------------------------------------------------
void Application::setup (void) {
}

// --------------------------------------------------------------------------
void Application::start (void) {
}

// --------------------------------------------------------------------------
void Application::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
}

Application *App;

// ==========================================================================
// FUNCTION setup
// ==========================================================================
void setup (void) {
    pinMode (13, OUTPUT);
    digitalWrite (13, LOW);
    pinMode (6, OUTPUT);
    digitalWrite (6, LOW);
    Serial.begin (115200);
    Serial.write (27);
    Serial.print (F("[2J"));
    Serial.write (27);
    Serial.print (F("[HMIDIx 1.0\r\n---------------------------------------"
                    "-------------------------------------\r\n"));
    Memory.scanForRAM();
    I2C.begin ();
    Console.begin ();
    digitalWrite (13, HIGH);
    App->setup ();
    InPort.begin();
    Port.begin();
    EventQueue.startTimer (1);
    App->start();
}

// ==========================================================================
// FUNCTION loop
// ==========================================================================
void loop (void) {
    volatile Event *e = EventQueue.waitEvent();
    if (e) {
        eventtype tp = e->type;
        eventid id = e->id;
        uint16_t X = e->X.wval;
        uint8_t Y = e->Y;
        uint8_t Z = e->Z;
        App->handleEvent (tp,id,X,Y,Z);
    }
}
