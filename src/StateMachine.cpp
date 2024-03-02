#include "StateMachine.h"

StateMachine::StateMachine(){};
StateMachine::~StateMachine(){};

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
        STATE_DEBUG("State change: %s --> %s\n", currentState->name(), next->name());
        currentState->exit();
        currentState = next;
        currentState->enter();
    }
}

void StateMachine::addTransition(State const *startingState, int trigger, State *targetState)
{
    transitionTable.push_back(new StateMachineEdge(startingState, trigger, targetState));
}

StateMachineEdgePtr_t StateMachine::findEdge(State* sourceState, int trigger)
{
    int count = transitionTable.size();
    for(int i=0;i<count;i++)
    {
        StateMachineEdgePtr_t edge = transitionTable.at(i);
        if (edge->sourceState == sourceState && edge->trigger == trigger)
        {
            return edge;
        }
    }
    STATE_DEBUG("Failed to find edge for trigger! %s %d\n", sourceState->name(), trigger);
    return NULL;
}