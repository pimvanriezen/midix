#include "Application.h"
#include "Console.h"
#include "I2C.h"
#include "Port.h"
#include "EventQueue.h"

Application::Application (void) {
    App = this;
}

Application::~Application (void) {
}

void Application::setup (void) {
}

void Application::start (void) {
}

void Application::handleEvent (eventtype tp, eventid id, uint16_t X,
                               uint8_t Y, uint8_t Z) {
}

Application *App;

void setup (void) {
    I2C.begin ();
    Console.begin ();
    App->setup ();
    InPort.begin();
    Port.begin();
    EventQueue.startTimer (1);
    App->start();
}

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
