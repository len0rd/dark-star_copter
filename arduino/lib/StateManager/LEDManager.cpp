#include <Arduino.h>
#include <StateManager.h>
#include <LEDManager.h>

LEDManager::LEDManager(int ledPin) {
  LED_PIN = ledPin;
  pinMode(LED_PIN, OUTPUT);
}

void LEDManager::manageLED(StateManager::drone_state_t drone_state) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis > LED_UPDATE_INTERVAL_) {
    switch (drone_state) {
      case StateManager::STANDBY:
        fadeLED();
        break;
      case StateManager::FLYING:
        ledState = ON;
        ledValue = 255;
        break;
      case StateManager::LOW_BATT:
        ledState = SLOW_BLINK;
        if (currentMillis - previousSlowMillis > LED_SLOW_UPDATE_INTERVAL_) {
          blinkLed();
          previousSlowMillis = currentMillis;
        }
        break;
      case StateManager::NO_CMD:
      case StateManager::CRITICAL_BATT:
        ledState = FAST_BLINK;
        blinkLed();
        break;
      default:
        ledState = ON;
        ledValue = 255;
        break;
    }
    analogWrite(LED_PIN, ledValue);
    previousMillis = currentMillis;
  }
}

void LEDManager::fadeLED() {
  if (ledState != FADE_IN && ledState != FADE_OUT) {
    ledState = FADE_IN;
  }

  if (ledState == FADE_IN) {
    if (ledValue + 5 > 255) {
      ledState = FADE_OUT;
    }
    else {
      ledValue += 5;
    }
  }
  else if (ledState == FADE_OUT) {
    if (ledValue - 5 < 0) {
      ledState = FADE_IN;
    }
    else {
      ledValue -= 5;
    }
  }
}

void LEDManager::blinkLed() {
  if (ledValue != 0) {
    ledValue = 0;
  }
  else {
    ledValue = 255;
  }
}
