#ifndef _MODEL_ADU_EMC_H
#define _MODEL_ADU_EMC_H

#include <stdint.h>

#include <common/config.h>

#include <lcmtypes/lcmtypes_adu_status_t.h>
#include <lcmtypes/lcmtypes_adu_command_t.h>
#include <lcmtypes/lcmtypes_adu_button_command_t.h>

typedef struct _model_adu_emc model_adu_emc_t;
struct _model_adu_emc
{
    // last rx'd utime
    int64_t  utime;

    // ADU model

    double    adu_gas_brake_state_mv;
    double    adu_gas_brake_goal_mv;
    double    adu_gas_brake_rate;

    double    adu_steer_state_mv;
    double    adu_steer_goal_mv;
    double    adu_steer_rate;

    int      adu_state;

    int      adu_button_status;

    // EMC model
    // internal state is modeled in units of mv
    double    emc_gas_brake_state_mv;
    double    emc_gas_brake_goal_mv;   
    double    emc_gas_brake_rate;
   
    double    emc_steer_state_mv;
    double    emc_steer_goal_mv;   
    double    emc_steer_rate;  
    
    int      emc_shifter_state;
    int      emc_shifter_goal;
    int      emc_shifting;           // are we shifting?

    // how much time has elapsed since a shift command was requested?
    double    emc_shifter_time_since_shift_started;

    // how long does each shift take?
    double    emc_shifter_time_per_shift;

    // How much are we pressing on each pedal?
    double    emc_gas_pedal;          // normalized [0,1]
    double    emc_brake_pedal;        // normalized [0,1]

    double    emc_steer_left_mv;
    double    emc_steer_center_mv;
    double    emc_steer_right_mv;
    double    emc_steer;              // normalized [-1, 1]

    // what ranges of gas_brake correspond to zero and full acceleration?
    double    emc_gas_none_mv;
    double    emc_gas_full_mv;  // maximum acceleration

    // what ranges of gas_brake correspond to zero and full braking?
    double    emc_brake_none_mv;
    double    emc_brake_full_mv;  // maximum braking
};

model_adu_emc_t *model_adu_emc_create(Config *config);

void model_adu_emc_step(model_adu_emc_t *aduemc, 
                        const lcmtypes_adu_command_t *adu_cmd, 
                        const lcmtypes_adu_button_command_t *button_cmd, 
                        double dt);

void model_adu_emc_get_status(model_adu_emc_t *aduemc, 
        lcmtypes_adu_status_t *adustatus);

#endif
