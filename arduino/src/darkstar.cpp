#include <Arduino.h>
#include <CPPM.h>
#include <StateManager.h>
#include <LEDManager.h>

//Objects:
StateManager  stateManager;
LEDManager    ledManager(6); //Digital pin for led

void setup() {
  //Serial.begin(9600);
  //Init
  stateManager.setupSM();
}

void loop() {

  StateManager::drone_state_t state = stateManager.calculateState();
  //Serial.print("In State: "); Serial.println(state);
  ledManager.manageLED(state);
}
