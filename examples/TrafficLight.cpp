#include <Arduino.h>

/**
 * A CppStateMachine demonstration of reusing a common class to define multiple states.
*/

/**
 * SAMPLE OUTPUT
 * If you run this program and examine the Serial Monitor, you should see the following:

--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
State change: Init(1) -> NorthSouth Go
State change: NorthSouth Go(1) -> NorthSouth Warn
State change: NorthSouth Warn(1) -> NorthSouth Stop
State change: NorthSouth Stop(1) -> EastWest Go
State change: EastWest Go(1) -> EastWest Warn
State change: EastWest Warn(1) -> EastWest Stop
State change: EastWest Stop(1) -> NorthSouth Go
State change: NorthSouth Go(1) -> NorthSouth Warn
State change: NorthSouth Warn(1) -> NorthSouth Stop
State change: NorthSouth Stop(1) -> EastWest Go
etc...

 * If you short Pin 5 to ground, you should see the machine transition
 * back to the Init state

State change: EastWest Stop(1) -> NorthSouth Go
State change: NorthSouth Go(2) -> Init
Failed to find edge for trigger:Init2
Failed to find edge for trigger:Init2
State change: Init(1) -> NorthSouth Go
State change: NorthSouth Go(1) -> NorthSouth Warn
State change: NorthSouth Warn(1) -> NorthSouth Stop
*/


// Turn on Serial.print statements
#define STATE_DEBUG
#include <StateMachine.h> 

// Short this pin to ground to reset the lights.
#define RESET_PIN 5

// East-West leds:
#define LED_EW_RED 8
#define LED_EW_AMBER 9
#define LED_EW_GREEN 10

// North-South leds:
#define LED_NS_GREEN 11
#define LED_NS_AMBER 12
#define LED_NS_RED 13

// Define an array of all our GPIO pins, to make it easier
// to loop through them all.
int const LEDs[] = {
  LED_EW_RED,
  LED_EW_AMBER,
  LED_EW_GREEN,
  LED_NS_RED,
  LED_NS_AMBER,
  LED_NS_GREEN
};

enum trigger_t {
  NO_TRIGGER,
  TRIGGER_TIMEOUT,
  TRIGGER_RESET
};

// This enum describes the various LED configurations.
enum mode_t {
  OFF = 0,
  NS_GREEN_EW_RED = (1 << LED_NS_GREEN | 1 << LED_EW_RED),
  NS_AMBER_EW_RED = (1 << LED_NS_AMBER | 1 << LED_EW_RED),
  ALL_RED = (1 << LED_NS_RED || 1 << LED_EW_RED),
  NS_RED_EW_GREEN = (1 << LED_NS_RED | 1 << LED_EW_GREEN),
  NS_RED_EW_AMBER = (1 << LED_NS_RED | 1 << LED_EW_AMBER)
};

// Given an LED mode, set all the LED GPIOs appropriately.
void setLEDs(mode_t mode)
{
  int bitmask = mode;
  for(int i=0;i<6; i++)
  {
    int ledPin = LEDs[i];
    bool on = bitmask & 1<<ledPin;
    digitalWrite(ledPin, on ? HIGH : LOW);
  }
}

// This state will blink the North/South RED led, and the Amber East/West LED.
// After 5 seconds, `loop()` will issue a TRIGGER_TIMEOUT event.
class StateInit : public State
{
  public: 
  StateInit() : State("Init") {}

  void enter() {
    State::enter();
    setLEDs(OFF);
  }

  int loop() {
    // Flash the red LEDs
    int sTime = stateTime();

    bool on = (sTime / 500) & 1;
    setLEDs( on ? NS_RED_EW_AMBER : OFF);

    // Stay in this state for a while
    if (stateTime() > 5000)
    {
      return TRIGGER_TIMEOUT;
    }
    return NO_TRIGGER;
  }
};

// Define a class that can set the specified LEDs, with a timeout.
class ModeState : public State
{
  mode_t ledState;
  unsigned long timeout;

  public: 
  ModeState(char const *name, mode_t mode, unsigned long stateTimeout) :
    State(name),
    ledState(mode),
    timeout(stateTimeout)
  {
  }

  void enter()
  {
    State::enter();
    setLEDs(ledState);
  }

  int loop() {
    // issue a TRIGGER_TIMEOUT event after the specified time.
    if (stateTime() > timeout) {
      return TRIGGER_TIMEOUT;
    }
    return NO_TRIGGER;
  }
};


// Define our states.
StateInit stateInit;
ModeState stateNSGo("NorthSouth Go", NS_GREEN_EW_RED, 5000);
ModeState stateNSWarn("NorthSouth Warn", NS_AMBER_EW_RED, 1000);
ModeState stateNSStop("NorthSouth Stop", ALL_RED, 500);
ModeState stateEWGo("EastWest Go", NS_RED_EW_GREEN, 5000);
ModeState stateEWWarn("EastWest Warn", NS_RED_EW_AMBER, 1000);
ModeState stateEWStop("EastWest Stop", ALL_RED, 500);

// Define the machine, with enough edge slots to hold all our transitions
StateMachine stateMachine(14);

void setup() {
  Serial.begin(115200);

  for(int i=0;i<6;i++)
  {
    pinMode(LEDs[i], OUTPUT);
  }
  pinMode(RESET_PIN, INPUT_PULLUP);

  // Set up the normal timing transitions.
  stateMachine.addTransition(&stateInit,   TRIGGER_TIMEOUT, &stateNSGo);
  stateMachine.addTransition(&stateNSGo,   TRIGGER_TIMEOUT, &stateNSWarn);
  stateMachine.addTransition(&stateNSWarn, TRIGGER_TIMEOUT, &stateNSStop);
  stateMachine.addTransition(&stateNSStop, TRIGGER_TIMEOUT, &stateEWGo);
  stateMachine.addTransition(&stateEWGo,   TRIGGER_TIMEOUT, &stateEWWarn);
  stateMachine.addTransition(&stateEWWarn, TRIGGER_TIMEOUT, &stateEWStop);
  stateMachine.addTransition(&stateEWStop, TRIGGER_TIMEOUT, &stateNSGo);

  // Make all states go back to StateInit if RESET is triggered.
  stateMachine.addTransition(&stateNSGo,   TRIGGER_RESET, &stateInit);
  stateMachine.addTransition(&stateNSWarn, TRIGGER_RESET, &stateInit);
  stateMachine.addTransition(&stateNSStop, TRIGGER_RESET, &stateInit);
  stateMachine.addTransition(&stateEWGo,   TRIGGER_RESET, &stateInit);
  stateMachine.addTransition(&stateEWWarn, TRIGGER_RESET, &stateInit);
  stateMachine.addTransition(&stateEWStop, TRIGGER_RESET, &stateInit);

  stateMachine.start(&stateInit);
}

void loop() {
  stateMachine.loop();
  
  delay(100);

  if (digitalRead(RESET_PIN) == 0)
  {
    stateMachine.fire(TRIGGER_RESET);
  }
}
