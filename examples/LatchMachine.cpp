#include <Arduino.h>
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

class StateHigh : public State
{
  public:
  StateHigh() :
    State("StateHigh") {}

  void enter() {
    digitalWrite(LED, HIGH);
  }

  int loop() { return NO_TRIGGER;}

  void exit() {
    digitalWrite(LED, LOW);
  }
};

class StateLow : public State
{
  public:
    StateLow() :
      State("StateLow") {}

  int loop() { return NO_TRIGGER;}
};



StateHigh stateHigh;
StateLow stateLow;
StateMachine stateMachine;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SET, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);

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
