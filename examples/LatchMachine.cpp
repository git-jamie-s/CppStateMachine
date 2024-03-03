#include <Arduino.h>

// Turn on Serial.print statements
#define STATE_DEBUG
#include <StateMachine.h> 

// Set the LED GPIO that we want to control
#define LED 13

// Short this pin to ground to set the latch
#define PIN_SET 11
// Short this pin to ground to clear the latch
#define PIN_RESET 12

enum trigger_t {
  NO_TRIGGER,
  TRIGGER_INPUT_SET,
  TRIGGER_INPUT_RESET
};

// This state will set the GPIO hi when it is entered, and set the GPIO low
// when the state exits.
class StateHigh : public State
{
  public:
  StateHigh() :
    State("StateHigh") {}

  void enter() {
    State::enter();
    digitalWrite(LED, HIGH);
  }

  // Nothing to do here, just waiting for button presses.
  int loop() { return NO_TRIGGER;}

  void exit() {
    digitalWrite(LED, LOW);
  }
};

// Seeing as how the StateHigh manages all the GPIO setting,
// this state has nothing to do at all.
// Optionally, you could move the `digitalWrite(LED, LOW)` here into
// StateLow::enter() if you preferred.
class StateLow : public State
{
  public:
    StateLow() :
      State("StateLow") {}

  int loop() { return NO_TRIGGER;}
};


// Define an instance of each state.
StateHigh stateHigh;
StateLow stateLow;

// Define the machine!
// Note that this uses the default transitionTable size to
// show that addTransition will print a message if it needs to grow the table.
StateMachine stateMachine;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SET, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);

  // Configure the state transitions!
  stateMachine.addTransition(&stateLow, TRIGGER_INPUT_SET, &stateHigh);
  stateMachine.addTransition(&stateHigh, TRIGGER_INPUT_RESET, &stateLow);
  stateMachine.start(&stateLow);
}

void loop() {
  stateMachine.loop();

  if (digitalRead(PIN_SET) == LOW)
  {
    stateMachine.fire(TRIGGER_INPUT_SET);
    delay(100);
  }
  else if (digitalRead(PIN_RESET) == LOW)
  {
    stateMachine.fire(TRIGGER_INPUT_RESET);
    delay(100);
  }
}
