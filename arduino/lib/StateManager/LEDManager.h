/*
Manages the LEDs on darkstar given input from StateManager
*/
#ifndef LEDManager_h
#define LEDManager_h

#include <Arduino.h>
#include <StateManager.h>

/////USER-DEFINED CONSTANTS
#define LED_UPDATE_INTERVAL_ 50
#define LED_SLOW_UPDATE_INTERVAL_ LED_UPDATE_INTERVAL_ * 20

class LEDManager {
  public:
    LEDManager(int ledPin);
    void manageLED(StateManager::drone_state_t drone_state);
  private:
    typedef enum {
      FADE_IN,
      FADE_OUT,
      FAST_BLINK,
      SLOW_BLINK,
      ON
    } led_state_t;

    led_state_t   ledState           = FADE_IN;
    int           LED_PIN            = 6;
    int           ledValue           = 0;
    unsigned long previousMillis     = 0;
    unsigned long previousSlowMillis = 0;

    void fadeLED();
    void blinkLed();
};

#endif
