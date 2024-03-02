/**
 * This example shows the very basic usage of StateMachine.
 * It has three states:
 *   An Init state
 *   A Blinking State
 *   An Off State.
 * 
 * This example only shows a single trigger for each state, TRIGGER_EVENT_TIMEOUT.
 * 
 * Once the Init state completes, the Blink and Off states loop around to each other forever,
 * using timerouts.
*/
#include <Arduino.h>
#include "StateMachine.h"

#define LED 2

// Define all your triggers, making sure that ordinal 0 is not used as a trigger.
enum {
  NO_TRIGGER,
  TRIGGER_EVENT_TIMEOUT,
} trigger_t;

/**
 * Define the initial state. It just needs to configure the serial port
 * and set the LED as an output.
 * It exits immediately.
*/
class StateInit : public State
{
  public: 
  StateInit() :
    State("Init")
  {}

  void enter() {
    Serial.printf("Starting!\n");
    pinMode(LED, OUTPUT);
  }

  int loop() {
    return TRIGGER_EVENT_TIMEOUT;
  }
};


/** 
 * Define the "Blink" state.
 * It will blink the LED during it's loop, until it times out.
 * It will exit after it times out.
*/
class StateBlink : public State
{
  public: 
  StateBlink() :
    State("Blink")
  {
  }

  // Turn the LED on and off every 200ms
  int loop() 
  {
    int time = stateTime();

    bool onOff = (time / 100) & 1;
    digitalWrite(LED, onOff);

    // Exit after two seconds
    return  (stateTime() > 2000) ? TRIGGER_EVENT_TIMEOUT : NO_TRIGGER;
  }
};

class StateOff : public State
{
  public:
  StateOff() :
    State("Off") 
  {
  }
  
  void enter() {
    State::enter();
    digitalWrite(LED, 0);
  }

  int loop() {
    // exit after 2 seconds
    return (stateTime() > 2000) ? TRIGGER_EVENT_TIMEOUT : NO_TRIGGER;
  }
};

StateInit stateInit;
StateBlink stateBlink;
StateOff stateOff;
StateMachine stateMachine;

void setup() 
{
  Serial.begin(115200);

  // Define all the transitions of our state machine
  stateMachine.addTransition(&stateInit, TRIGGER_EVENT_TIMEOUT, &stateBlink);
  stateMachine.addTransition(&stateBlink, TRIGGER_EVENT_TIMEOUT, &stateOff);
  stateMachine.addTransition(&stateOff, TRIGGER_EVENT_TIMEOUT, &stateBlink);

  // Set the state machine's initial state
  stateMachine.start(&stateInit);
}

void loop() {
  stateMachine.loop();
}

 