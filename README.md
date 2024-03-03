### CppStateMachine
This arduino-targeted library defines a Class-oriented state machine.
States are defined by classes, where the work is defined by overriding the `enter`, `loop`, and `exit` functions.


#### States
Create states by extending the State base class.

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

  int loop() { return 0; /* Nothing to do here */ }

  void exit() {
    digitalWrite(gpioPin, LOW);
  }
}
```

#### State Methods
##### `void enter()`
The `State::enter()` method is called once, when the StateMachine activates this state.
Note: 
If you wish to use the `stateTime()` functions (see below), you _must_ call the parent `enter()` version so it can grab the current time. EG.,
```
void MyState::enter() {
  State::enter();
  // do more enter-y stuff here...
}
```

##### `void exit()`
The `State::exit()` method is called once, just before the StateMachine moves from this state to another state.

##### `int loop()`
The `State::loop()` method is called repeatedly while this state is active.
Your derived class must override this function.

Important! `loop()` must not call `StateMachine::fire()` directly!
Instead, if the loop wants to fire a trigger, it should return it.
```
int MyState::loop() {
  // bad:
  // if (digitalRead(openButton))
  //   stateMachine.fire(OPEN_CURTAINS);
  // Do this instead:
  if (digitalRead(openButton))
      return OPEN_CURTAINS;
  return NO_TRIGGER; // IE. 0
}
```
##### `unsigned long stateStartTime()`
This method may be called to determine the `millis()` time that this state started.

##### `unsigned long stateTime()`
This method may be called to determine how many milliseconds this state has been active.

#### Triggers
Define all your triggers for the statemachine in an enum, with 0(zero) being NO_TRIGGER. This is important, as the `int loop()` function should return 0 to indicate no trigger.
Triggers for your state machine should be defined with an `enum`, but any integer will work.
The value 0 (zero) always means "no trigger"!  Therefore, the first label in your enum should reflect that.

For example, a set of triggers for an automated curtain might be:
```
enum trigger_t {
  NO_TRIGGER,
  STOP,
  CLOSE,
  OPEN,
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
Please see [LatchMachine.cpp](/git-jamie-s/CppStateMachine/blob/main/examples/LatchMachine.cpp) for an example.

# NOTES
### Debugging
To turn on state change printouts to serial, define the `STATE_DEBUG` macro before including `StateMachine.h`
This will cause the statemachine to print transitions.
```
#include <Arduino.h>
#define STATE_DEBUG
#include <StateMachine.h>
```

For the example Blinky code, this would produce the following, where (n) indicates the numeric value of the trigger:
```
Reallocating more edge slots
Starting!
State change: Init(1) -> Blink
State change: Blink(1) -> Off
State change: Off(1) -> Blink
etc...
```

If you do not `#define STATE_DEBUG`, then the serial code is excluded completely, so as to minimize the library's footprint.

### Transition Table Memory
This library uses `malloc` to avoid pulling in more memory-intensive libraries, and to maximize compatibility.
Ideally, your code will specify the number of required transitions when you instantiate the `StateMachine`.
EG.,
```
// I need 10 transitions, sooo:
StateMachine stateMachine(10);

// And then call addTranstion 10 times:
stateMachine.addTransition(...)
```
If you do not preallocate a large enough table, addTransition will reallocate a bigger table.
If you turn on STATE_DEBUG, you will see a message about `Reallocating more edge slots`, indicating that you should preallocate a high number, if you want to avoid reallocations.
