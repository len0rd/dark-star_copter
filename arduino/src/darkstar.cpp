#include<Arduino.h>
#include<CPPM.h>

const int QUAD_LED = 6;
const int CPPM_DELTA = 5;

//Command redundancy check:
struct command_t {
  int aile;
  int elev;
  int thro;
  int rudd;
} lastCommand;
unsigned int duplicateCommandCount = 0;

//LED State variables:
int                 ledValue                  = 0;
unsigned long       previousMillis            = 0;
unsigned long       previousSlowMillis        = 0;
const unsigned long LED_UPDATE_INTERVAL       = 50;
const unsigned long LED_SLOW_UPDATE_INTERVAL  = LED_UPDATE_INTERVAL * 20;
typedef enum {
  FADE_IN,
  FADE_OUT,
  FAST_BLINK,
  SLOW_BLINK,
  ON
} led_state_t;
led_state_t ledState = FADE_IN;

bool cmdWithinDelta(int cmd1, int cmd2) {
  return cmd2 >= (cmd1 - 5) && cmd2 <= (cmd1 + 5);
}

bool repetitiveCmd(command_t curCmd) {
  return cmdWithinDelta(lastCommand.aile, curCmd.aile) &&
         cmdWithinDelta(lastCommand.elev, curCmd.elev) &&
         cmdWithinDelta(lastCommand.thro, curCmd.thro) &&
         cmdWithinDelta(lastCommand.rudd, curCmd.rudd);
}

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

void manageLED() {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > LED_UPDATE_INTERVAL ) {

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
        if (currentMillis - previousSlowMillis > LED_SLOW_UPDATE_INTERVAL ) {
          blinkLed();
          previousSlowMillis = currentMillis;
        }
        break;
      case ON:
        ledValue = 255;
        break;
    }

    analogWrite(QUAD_LED, ledValue);
    previousMillis = currentMillis;
  }
}

void setup() {
  //pin declarations:
  pinMode(QUAD_LED, OUTPUT);

  //Init
  Serial.begin(9600);
  CPPM.begin();
}

void loop() {

  CPPM.cycle();

  if (CPPM.synchronized()) {

    int aile = CPPM.read_us(CPPM_AILE);
    int elev = CPPM.read_us(CPPM_ELEV);
    int thro = CPPM.read_us(CPPM_THRO);
    int rudd = CPPM.read_us(CPPM_RUDD);
    int armDisarm = CPPM.read_us(CPPM_GEAR);
    int rcOVRD = CPPM.read_us(CPPM_AUX1);

    command_t currentCommand;
    currentCommand.aile = aile;
    currentCommand.elev = elev;
    currentCommand.thro = thro;
    currentCommand.rudd = rudd;
    bool updateLastCommand = true;

    //LED control:
    if (armDisarm < 1250  && ledState != FADE_IN && ledState != FADE_OUT) {
      ledState = FADE_IN;
    }
    else if (armDisarm > 1250) {
      ledState = ON;
      if (rcOVRD < 1250) { //rc_override is active

        //check for command repitition
        if (repetitiveCmd(currentCommand)) {
          updateLastCommand = false;
          duplicateCommandCount++;
          if (duplicateCommandCount > 50000 ) {
            //aka, if we're armed and rc_override is on, but we havent received
            //a new command in a while this is bad and probably means we're in failsafe mode.
            //this will blink for about a second until our uint rolls over
            ledState = FAST_BLINK;
          }
        }
        else {
          duplicateCommandCount = 0;
        }
      }
      else {
        duplicateCommandCount = 0;
      }
    }

    if (updateLastCommand) {
      lastCommand = currentCommand;
    }

  }

  manageLED();
}
