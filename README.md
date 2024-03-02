### CppStateMachine
This arduino-targeted library defines a Class-oriented state machine.
States are defined by classes, where the work is defined by overriding the `enter`, `loop`, and `exit` functions.


#### States
For example, here is a state that turns on a GPIO when it is entered, and turns it off when the state exits:
```
class StateGPIOOn : State
{
  private:
  int gpioPin;

  public:
  StateGPIOOn(int pin) : gpioPin(pin) {}

  void enter() {
    digitalWrite(gpioPin, HIGH);
  }

  void loop() { /* Nothing to do here */ }

  void exit() {
    digitalWrite(gpioPin, LOW);
  }
}
```

#### Triggers
Triggers for your state machine should ideally be defined with an `enum`, but any integer will work.
The value 0 (zero) always means "no trigger"!  Therefore, the first label in your enum should reflect that.

For example, a set of triggers for an automated curtain might be:
```
enum trigger_t {
  NO_TRIGGER,
  STOP,
  CLOSE,
  OPEN
};
```  

#### Transitions / Edges
Armed with a set of states and triggers, you then define the StateMachine by adding state transitions (or edges if you prefer).
This is done by calling `StateMachine::addTranstion(State* source, int trigger, State* target)`
For example, a statemachine that models a flip-flop:
```
enum trigger_t { NO_TRIGGER, SET, RESET }
StateLow stateLow;
StateHigh stateHigh;
StateMachine stateMachine;
void setup() {
  stateMachine.addTransition(&stateLow, SET, &stateHigh);
  stateMachine.addTransition(&stateHigh, CLEAR, &stateLow);
  etc...
```

#### Running
The starting state of the machine is configured by calling `StateMachine::start(State* startingState)`.
After the starting state has been set, the state machine is operated by calling `StateMachine::loop()`.
Triggers / Events can be injected into the state machine by calling `StateMachine::fire(int trigger)`.

### Putting it all together
Please see the JKFlipFlop.cpp for an example.
