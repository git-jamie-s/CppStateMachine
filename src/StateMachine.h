#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

#include "State.h"

// To use Serial.printf, #define STATE_DEBUG=1
// To use Serial.print only (some platforms don't have Serial.printf ), #define STATE_DEBUG=1
#ifdef STATE_DEBUG
#define DEBUG_PRINT(...) print(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

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

#ifdef STATE_DEBUG
    template <typename T>
    void print(T t) 
    {
        Serial.print(t);
    }

    template<typename T, typename... Args>
    void print(T t, Args... args)
    {
        Serial.print(t);
        print(args...) ;
    }
#endif
  public:
    StateMachine(int reserved=1) :
      transitionTableSize(reserved),
      transitionTableCount(0)
    {
      transitionTable = (StateMachineEdgePtr_t *)malloc(reserved * stateMachineEdgeBytes);
    }

    ~StateMachine(){
      free(transitionTable);
    }

    void addTransition(State const *startingState, int trigger, State *targetState)
    {
      if (transitionTableCount + 1 >= transitionTableSize)
      {
        DEBUG_PRINT("Reallocating more edge slots\n");
        // Increase the allocated storage for transition edges.
        transitionTableSize += 4;
        transitionTable = (StateMachineEdgePtr_t *)realloc(transitionTable, transitionTableSize * stateMachineEdgeBytes);
      }
      transitionTable[transitionTableCount++] = new StateMachineEdge(startingState, trigger, targetState);
    }

    void start(State *startingState)
    {
      currentState = startingState;
      startingState->enter();
    }

    void loop()
    {
      if (currentState == NULL)
      {
          return;
      }

      int trigger = currentState->loop();

      if (trigger != 0)
      {
          fire(trigger);
      }
    }


    void fire(int trigger) 
    {
        StateMachineEdgePtr_t edge = findEdge(currentState, trigger);
      if (edge) {
        State * next = edge->targetState;
        DEBUG_PRINT("State change: ", currentState->name(), "(", trigger, ") -> ", next->name(), "\n");

        currentState->exit();
        currentState = next;
        currentState->enter();
      }
    }

    State const *getCurrentState()
    {
      return currentState;
    }


  private:
    // Attributes
    StateMachineEdgePtr_t findEdge(State* currentState, int trigger) 
    {
      for(int i=0;i<transitionTableCount;i++)
      {
        StateMachineEdgePtr_t edge = transitionTable[i];
        if (edge->sourceState == currentState && edge->trigger == trigger)
        {
            return edge;
        }
      }
      DEBUG_PRINT("Failed to find edge for trigger:", currentState->name(), trigger, "\n\n");

      return NULL;
    }

    State *currentState = NULL;

    StateMachineEdgePtr_t *transitionTable;
    int transitionTableSize;
    int transitionTableCount;


};

#endif 