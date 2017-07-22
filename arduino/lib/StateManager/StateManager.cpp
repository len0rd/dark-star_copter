
#include <Arduino.h>
#include <CPPM.h>
#include <StateManager.h>


void StateManager::setupSM(void) {
  CPPM.begin();
}

StateManager::drone_state_t StateManager::calculateState(void) {
  drone_state_t stateToReturn = REPEAT_LAST;

  //checkup on battery
  battery_state_t battState = getBatteryState();
  if ( battState < BATT_MID) { //we can do more fancy stuff later
    stateToReturn = (battState == BATT_LOW) ? LOW_BATT : CRITICAL_BATT;
  }

  if (stateToReturn == REPEAT_LAST) {
    stateToReturn = processCPPM();
  }

  //update _last_cmd we need something like this because the CPPM isnt
  //always synced, which could cause us to randomly enter some other state
  //this allows us to stay in our last state until we get something that updates
  if (stateToReturn == REPEAT_LAST && _last_cmd == REPEAT_LAST) {
    stateToReturn = STANDBY;
  }
  else if (stateToReturn == REPEAT_LAST) {
    stateToReturn = _last_cmd;
  }
  else {
    _last_cmd = stateToReturn;
  }

  return stateToReturn;
}

StateManager::drone_state_t StateManager::processCPPM() {
  drone_state_t stateToReturn = REPEAT_LAST;

  CPPM.cycle();

  if (CPPM.synchronized()) {

    command_t currentCommand;
    currentCommand.aile = CPPM.read_us(CPPM_AILE);
    currentCommand.elev = CPPM.read_us(CPPM_ELEV);
    currentCommand.thro = CPPM.read_us(CPPM_THRO);
    currentCommand.rudd = CPPM.read_us(CPPM_RUDD);

    int  armDisarm         = CPPM.read_us(CPPM_GEAR);
    int  rcOVRD            = CPPM.read_us(CPPM_AUX1);
    bool updateLastCommand = true;

    if (armDisarm > 1250) {
      stateToReturn = FLYING;
      if (rcOVRD < 1250) {
        //check for command repitition:
        if (repetitiveCmd(currentCommand)) {
          updateLastCommand = false;
          _dup_cmd_count++;
          if (_dup_cmd_count > 50000 ) {
            //aka, if we're armed and rc_override is on, but we havent received
            //a new command in a while this is bad and probably means we're in failsafe mode.
            //this will blink for about a second until our uint rolls over
            stateToReturn = NO_CMD;
          }
        }
      }
    }
    else {
      stateToReturn = STANDBY;
    }

    if (updateLastCommand) {
      lastCommand = currentCommand;
      _dup_cmd_count = 0;
    }
  }
  return stateToReturn;
}

bool StateManager::repetitiveCmd(command_t curCmd) {
  return cmdWithinDelta(lastCommand.aile, curCmd.aile) &&
         cmdWithinDelta(lastCommand.elev, curCmd.elev) &&
         cmdWithinDelta(lastCommand.thro, curCmd.thro) &&
         cmdWithinDelta(lastCommand.rudd, curCmd.rudd);
}

bool StateManager::cmdWithinDelta(int cmd1, int cmd2) {
  return cmd2 >= (cmd1 - CPPM_DELTA_) && cmd2 <= (cmd1 + CPPM_DELTA_);
}

StateManager::battery_state_t StateManager::getBatteryState() {
  battery_state_t currentState = BATT_OK;

  float measurementOffset = 0.5; //this is just an observed estimation
  for (size_t pin = 0; pin < BATTERY_CELL_COUNT_; pin++) {
    float cellVoltage = 0.0;
    if (pin == 0) {
      cellVoltage = readVoltageWODivider(_ANALOG_PINS[pin]);
    }
    else {
      cellVoltage = readVoltage(_ANALOG_PINS[pin]);
    }

    cellVoltage = (cellVoltage - ((pin + 1) * measurementOffset)) / (pin + 1);

    if (cellVoltage < 3.4 && cellVoltage > 2.0) {
      //anything below 2 almost certainly indicates
      //that the battery isnt currently connected
      currentState = BATT_CRIT;
      break;
    }
    else if (cellVoltage < 3.65) {
      currentState = BATT_LOW;
    }
    else if (cellVoltage < 3.85) {
      currentState = BATT_MID;
    }
  }
  return currentState;
}

float StateManager::readVoltage(int pin) {
  float vout = (analogRead(pin) * 5.0) / 1024.0;
  return (vout / (R2/(R1+R2)));
}

float StateManager::readVoltageWODivider(int pin) {
  return (analogRead(pin) * 5.0) / 1024.0;;
}
