#ifndef _EVENTQUEUE_H
#define _EVENTQUEUE_H 1

/// The event type determines handling priority.
/// IRQ events have full priority. REQUEST events
/// will not be handled in EventQueueManager::yield(), only in the regular
/// waitEvent loop.
enum eventtype
{
    TYPE_NONE,
    TYPE_IRQ,
    TYPE_TIMER,
    TYPE_REQUEST
};

typedef uint8_t eventid;
typedef uint8_t serviceid;

/// Convenience type for mixing words and bytes.
union monoduo
{
    uint16_t    wval;
    uint8_t     val[2];
};

/// Event type for a timer tick.
#define EV_TIMER_TICK 0x3e

/// A queued event as it exists inside a ring buffer. Contains
/// identifying information and 32 bits of user data.
struct Event
{
    volatile eventtype    type:2;
    volatile eventid      id:6;
    volatile serviceid    service;
    volatile monoduo      X;
    volatile uint8_t      Y;
    volatile uint8_t      Z;
};

/// Base class for services set up to receive events from the
/// EventQueue.
class EventReceiver
{
public:
                 EventReceiver (void);
    virtual     ~EventReceiver (void);
    virtual void handleEvent (eventtype, eventid, uint16_t, uint8_t, uint8_t);
};

/// Structure mapping a serviceid to an EventReceiver instance.
struct Callback
{
    serviceid        id;
    EventReceiver   *svc;
};

/// Basic ring buffer storage. Note that the actual Event array
/// is kept at zero size here. Unions below define storage size
/// for both ring buffers used in the EventQueueManager.
struct RingBuffer
{
    volatile uint8_t     wpos;
    volatile uint8_t     rpos;
    volatile Event       rbuf[0];
};

#define SZ_HIBUF 8
#define SZ_LOBUF 128

/// Storage spec for the high priority buffer (8 entries).
union HighPriorityBuffer
{
    RingBuffer           ring;
    char                 storage[sizeof(RingBuffer)+SZ_HIBUF*sizeof(Event)];
};

/// Storage spec for the low priority buffer (64 entries).
union LowPriorityBuffer
{
    RingBuffer           ring;
    char                 storage[sizeof(RingBuffer)+SZ_LOBUF*sizeof(Event)];
};

/// Basic event pump that can also emit timer of its own volition.
class EventQueueManager
{
public:
                         /// Boring constructor.
                         EventQueueManager (void);
                         
                         /// Boring destructor.
                        ~EventQueueManager (void);
    
                         /// Handle high priority events, if any
                         /// are pending. This function should be
                         /// called from handlers of low priority
                         /// events that take up a lot of time.
    void                 yield (void);
    
                         /// Handle events of any priority. If any
                         /// unhandled event falls through the
                         /// cracks, it is returned.
                         /// \param forever If true, 
    volatile Event      *waitEvent (bool forever = true);
    
                         /// Send an event.
                         /// \param tp The 2 bit event type.
                         /// \param svc The service-id.
                         /// \param id The event-id.
                         /// \param X 16 bits of user data.
                         /// \param Y 8 bits of more user data.
                         /// \param Z yet 8 more bits of user data.
    void                 sendEvent (eventtype tp, serviceid svc=0,
                                    eventid id=0, uint16_t valX=0,
                                    uint8_t valY=0, uint8_t valZ=0);
                                    
                         /// Maps a serviceid to a specific
                         /// EventReceiver service.
    void                 subscribe (serviceid s, EventReceiver *svc);
    
                         /// Subscribe a serviceid to timer events.
    void                 subscribeTimer (serviceid s);
    
                         /// Initialize the timer.
                         /// \param p Interval in milliseconds.
    void                 startTimer (uint16_t p);

    unsigned long        ts; ///< Current time (if timer enabled).
    uint16_t             missedTicks; 
    
protected:
    HighPriorityBuffer   hbuf; ///< High prio ringbuffer
    LowPriorityBuffer    lbuf; ///< Low prio ringbuffer
    Callback             subscribers[16]; ///< Mapped services
    uint8_t              numsubscribers; ///< Count
    bool                 timeractive; ///< If true, timer is set up.
    unsigned long        timernext; ///< Timestamp of next timer tick.
    uint16_t             timerperiod; ///< Tick period in ms.
    uint8_t              timerclients[4]; ///< Services that want ticks.
    uint8_t              timerclientcount; ///< Count
};

extern EventQueueManager EventQueue;

#endif
