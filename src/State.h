#ifndef _STATE_H
#define _STATE_H

#include <Arduino.h>

class State{
  private:
  unsigned long startTime;
  char const * stateName;

  public:
    State(char const *name = "noname") : stateName(name) {};
    virtual ~State() {};

    char const *name() const {return stateName;}
    unsigned long stateStartTime() { return startTime; }
    unsigned long stateTime() { return millis() - startTime;}

    // Here are the functions that you can implement to do work!
    // If you override `enter`, be sure to call State::enter to capture the start time.
    virtual void enter() { startTime = millis(); };
    virtual void exit() {};
    
    // loop will be called repeatedly while this state is active.
    // DO NOT CALL StateMachine::fire() from loop()!
    // Instead:
    //   if your loop generates a trigger, return that trigger.
    //   otherwise, return 0 for "no trigger".
    virtual int loop() = 0;
};

#endif