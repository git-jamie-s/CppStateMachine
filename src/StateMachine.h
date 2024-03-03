#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#include "State.h"

#define STATE_DEBUG Serial.println

class StateMachineEdge
{
  public:
  State const * sourceState;
  int const trigger;
  State * targetState;

  StateMachineEdge(State const *sourceState, int trigger, State *targetState) :
    sourceState(sourceState),
    trigger(trigger),
    targetState(targetState)
    {}
};

typedef StateMachineEdge* StateMachineEdgePtr_t;

class StateMachine
{
  size_t const stateMachineEdgeBytes = sizeof(StateMachineEdge);

  public:
    StateMachine(int reserved=1);
    ~StateMachine();

    void addTransition(State const *startingState, int trigger, State *targetState);
    void start(State *startingState);
    void loop();

    void fire(int trigger);

  private:
    // Attributes
    StateMachineEdgePtr_t findEdge(State* currentState, int trigger);
    State *currentState = NULL;

    StateMachineEdgePtr_t *transitionTable;
    int transitionTableSize;
    int transitionTableCount;
};

#endif 