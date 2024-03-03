#include "StateMachine.h"

StateMachine::StateMachine(int reserved)
{
    transitionTable = (StateMachineEdgePtr_t *)malloc(reserved * stateMachineEdgeBytes);
    transitionTableSize = reserved;
    transitionTableCount = 0;
};
StateMachine::~StateMachine(){
    free(transitionTable);
};

void StateMachine::start(State *startingState)
{
    currentState = startingState;
    startingState->enter();
}

void StateMachine::loop()
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

void StateMachine::fire(int trigger)
{
    StateMachineEdgePtr_t edge = findEdge(currentState, trigger);
    if (edge) {
        State * next = edge->targetState;
        STATE_DEBUG("State change:");
        STATE_DEBUG(currentState->name());
        STATE_DEBUG(next->name());
        currentState->exit();
        currentState = next;
        currentState->enter();
    }
}

void StateMachine::addTransition(State const *startingState, int trigger, State *targetState)
{
    
    if (transitionTableCount + 1 >= transitionTableSize)
    {
        // Increase the allocated storage for transition edges.
        transitionTableSize += 4;
        transitionTable = (StateMachineEdgePtr_t *)realloc(transitionTable, transitionTableSize * stateMachineEdgeBytes);
    }
    transitionTable[transitionTableCount++] = new StateMachineEdge(startingState, trigger, targetState);
}

StateMachineEdgePtr_t StateMachine::findEdge(State* sourceState, int trigger)
{
    for(int i=0;i<transitionTableCount;i++)
    {
        StateMachineEdgePtr_t edge = transitionTable[i];
        if (edge->sourceState == sourceState && edge->trigger == trigger)
        {
            return edge;
        }
    }
    STATE_DEBUG("Failed to find edge for trigger!");
    STATE_DEBUG(sourceState->name());
    return NULL;
}