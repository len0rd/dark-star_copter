/*
Simple statemanager for my quadcopter
*/
#ifndef StateManager_h
#define StateManager_h

#include <Arduino.h>

//////USER-DEFINED CONSTANTS
#define CPPM_DELTA_ 5 //min deviation for a command to count as unique
#define BATTERY_CELL_COUNT_ 3 //Battery cell type

class StateManager {
  public:
    typedef enum {
      STANDBY,
      FLYING,
      NO_CMD,
      LOW_BATT,
      CRITICAL_BATT,
      REPEAT_LAST
    } drone_state_t;

    void setupSM(void);
    drone_state_t calculateState(void);
  private:
    typedef enum {
      BATT_CRIT,
      BATT_LOW,
      BATT_MID,
      BATT_OK
    } battery_state_t;
    struct command_t {
      int aile;
      int elev;
      int thro;
      int rudd;
    } lastCommand;

    const float R2 = 15000.0;
    const float R1 = 27000.0;
    const int     _ANALOG_PINS[6] = {A0, A1, A2, A3, A4, A5};
    unsigned int  _dup_cmd_count  = 0;
    drone_state_t _last_cmd       = REPEAT_LAST;

    float readVoltage(int pin);
    float readVoltageWODivider(int pin);
    battery_state_t getBatteryState();
    drone_state_t processCPPM();
    bool repetitiveCmd(command_t curCmd);
    bool cmdWithinDelta(int cmd1, int cmd2);

};

#endif
