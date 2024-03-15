/**
 * This example shows the use of different triggers in StateMachine
 * to control a curtain.
 * The curtain motor/sensor is simulated.
 * 
 * Three buttons will be checked:
 * OPEN, CLOSE, and STOP. 
 * 
*/
#include <Arduino.h>
#include "StateMachine.h"

#define LED 2
#define INPUT_BUTTON_STOP 16
#define INPUT_BUTTON_OPEN 17
#define INPUT_BUTTON_CLOSE 18

// A class to simulate a curtain 'servo' motor
class CurtainMotor
{
  public:
  int motion = 0;
  float position = 0.5f;

  unsigned long microTime;

  void start(int dir) {
    Serial.printf("Starting motor: %d from position: %f\n", dir, position);
    microTime = micros();
    digitalWrite(LED, 1);
    motion = dir;
  };
  void stop() {
    Serial.printf("Stopping motor\n");
    motion = 0;
    digitalWrite(LED, 0);
  }

 float getPosition()
  {
    // Simulate moving here....
    unsigned long now = micros();
    float time = (now - microTime) / 5000000.0;
    position += motion * time;
    microTime = now;

    return position;  
  }
};

CurtainMotor curtainMotor;

enum trigger_t {
  NO_TRIGGER,
  TRIGGER_CONTINUE, // Used to exit the INIT state
  TRIGGER_OPEN,
  TRIGGER_CLOSE,
  TRIGGER_STOP
};

/**
 * Define the initial state. It just needs to configure the serial port
 * and set the IO modes.
 * It only has one trigger, and exits immediately.
*/
class StateInit : public State
{
  public: 
  StateInit() : State("Init") {}

  void enter() {
    Serial.printf("Starting!\n");
    pinMode(LED, OUTPUT);
    pinMode(INPUT_BUTTON_STOP, INPUT_PULLDOWN);
    pinMode(INPUT_BUTTON_OPEN, INPUT_PULLDOWN);
    pinMode(INPUT_BUTTON_CLOSE, INPUT_PULLDOWN);
  }

  int loop() {
    return TRIGGER_CONTINUE;
  }
};

class StateWaitForInput : public State
{
  public: 

  StateWaitForInput() : State("WaitForInput") {}

  void enter() {
    Serial.printf("Waiting for input...\n");
  }

  int loop() {
    return NO_TRIGGER;
  }
};

class StateOpening : public State
{
  public:
  StateOpening() : State("Opening") {}

  void enter() {
    State::enter();
    curtainMotor.start(1);
  }

  void exit() { 
    curtainMotor.stop();
  }

  int loop()
  {
    if (curtainMotor.getPosition() >= 1.0)
    {
      return TRIGGER_STOP;
    }
    return NO_TRIGGER;
  }
};

class StateClosing : public State
{
  public:
  StateClosing() : State("Closing") {}

  void enter() {
    State::enter();
    curtainMotor.start(-1);
  }

  void exit() { 
    curtainMotor.stop();
  }

  int loop()
  {
    if (curtainMotor.getPosition() <= 0)
    {
      return TRIGGER_STOP;
    }
    return NO_TRIGGER;
  }
};


StateInit stateInit;
StateWaitForInput stateWaitForInput;
StateOpening stateOpening;
StateClosing stateClosing;

StateMachine stateMachine;


class EdgeDetectTrigger
{
  int milliTime;

  public:
  bool state;
  int pin;
  trigger_t trigger;
  EdgeDetectTrigger(int pin, trigger_t trigger) : 
    pin(pin), 
    trigger(trigger),
    state(false),
    milliTime(millis())
  {
  }

  bool read()
  {
    int now = millis();
    if (now - milliTime < 100)
    {
      return false;
    }

    bool value = digitalRead(pin);
    if (value != state)
    {
      milliTime = now;
      state = value;
      return value;
    }
    return false;
  }
};

EdgeDetectTrigger inputs[] = {
  EdgeDetectTrigger(INPUT_BUTTON_STOP, TRIGGER_STOP),
  EdgeDetectTrigger(INPUT_BUTTON_OPEN, TRIGGER_OPEN),
  EdgeDetectTrigger(INPUT_BUTTON_CLOSE, TRIGGER_CLOSE),
};

void setup() {
  Serial.begin(115200);

  stateMachine.addTransition(&stateInit, TRIGGER_CONTINUE, &stateWaitForInput);

  stateMachine.addTransition(&stateWaitForInput, TRIGGER_OPEN, &stateOpening);
  stateMachine.addTransition(&stateWaitForInput, TRIGGER_CLOSE, &stateClosing);

  stateMachine.addTransition(&stateOpening, TRIGGER_STOP, &stateWaitForInput);
  stateMachine.addTransition(&stateOpening, TRIGGER_CLOSE, &stateClosing);

  stateMachine.addTransition(&stateClosing, TRIGGER_STOP, &stateWaitForInput);
  stateMachine.addTransition(&stateClosing, TRIGGER_OPEN, &stateOpening);

  stateMachine.start(&stateInit);
}


int oldMillis = 0;
float oldPosition = 0;

void loop() {
  stateMachine.loop();

  int curMillis = millis();
  if (curMillis - oldMillis > 250)
  {
    if (curtainMotor.motion != 0)
    {
      Serial.printf("Curtain motor position: %f\n", curtainMotor.getPosition());
    }
    oldMillis = curMillis;
  }

  for(int i=0;i<3;i++)
  {
    if (inputs[i].read())
    {
        stateMachine.fire(inputs[i].trigger);
        break;
    }
  }
}
