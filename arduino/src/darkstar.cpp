#include<Arduino.h>

//Pin Configuration:
const int quadArm1 = 11; //rear-right arm
const int quadArm2 = 10; //front-right arm
const int quadArm3 = 6; //rear-left arm
const int quadArm4 = 5; //front-left arm

//LED State variables:
int ledValue = 0;
unsigned long previousMillis = 0;
unsigned long previousSlowMillis = 0;
const unsigned long ledUpdateInterval = 50;
const unsigned long ledSlowUpdateInterval = ledUpdateInterval * 20;
typedef enum {
  FADE_IN,
  FADE_OUT,
  FAST_BLINK,
  SLOW_BLINK,
  ON
} led_state_t;
led_state_t ledState = ON;

void blinkLed() {
  //need to do something like this in the event we're
  //transitioning from the middle of a FADE_IN/OUT
  if (ledValue != 0) {
    ledValue = 0;
  }
  else {
    ledValue = 255;
  }
}

void setup() {
  //pin declarations:
  pinMode(quadArm1, OUTPUT);
  pinMode(quadArm2, OUTPUT);
  pinMode(quadArm3, OUTPUT);
  pinMode(quadArm4, OUTPUT);
}

void loop()
{

  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > ledUpdateInterval) {

    switch (ledState) {
      case FADE_IN:
        if (ledValue + 5 > 255) {
          ledState = FADE_OUT;
        }
        else {
          ledValue += 5;
        }
        break;
      case FADE_OUT:
        if (ledValue - 5 < 0) {
          ledState = FADE_IN;
        }
        else {
          ledValue -= 5;
        }
        break;
      case FAST_BLINK:
        blinkLed();
        break;
      case SLOW_BLINK:
        if (currentMillis - previousSlowMillis > ledSlowUpdateInterval) {
          blinkLed();
          previousSlowMillis = currentMillis;
        }
        break;
      case ON:
        ledValue = 255;
        break;
    }

    analogWrite(quadArm1, ledValue);
    analogWrite(quadArm2, ledValue);
    analogWrite(quadArm3, ledValue);
    analogWrite(quadArm4, ledValue);
    previousMillis = currentMillis;
  }

}
