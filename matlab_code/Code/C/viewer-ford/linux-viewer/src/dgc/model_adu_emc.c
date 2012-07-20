#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <common/config.h>
#include <common/timestamp.h>

#include <lcmtypes/lcmtypes_adu_command_t.h>
#include <lcmtypes/lcmtypes_adu_button_command_t.h>
#include <lcmtypes/lcmtypes_shift_enum_t.h>
#include <lcmtypes/lcmtypes_adu_status_t.h>
#include <lcmtypes/lcmtypes_adu_secondary_t.h>

#include "model_adu_emc.h"

#define SHIFT_GEARS_MIN_BRAKE_MV 1700.0

// XXX: to-do:

// * lcmtypes_adu_button_command_t and state simulation
// * adu_secondary simulation
// * steering-wheel "straight" offset
// * fit models to actual LR3/escape data

model_adu_emc_t *model_adu_emc_create(Config *config)
{
    model_adu_emc_t *aduemc = (model_adu_emc_t*) calloc(1, sizeof(model_adu_emc_t));

    aduemc->emc_gas_brake_rate = 5000.0 / config_get_double_or_fail(config, "vehicle.gas_brake_time");
    aduemc->emc_steer_rate     = 5000.0 / config_get_double_or_fail(config, "vehicle.steer_time");

    // lr3 values
    aduemc->emc_gas_none_mv     = config_get_double_or_fail(config, "vehicle.gas_none_mv");
    aduemc->emc_gas_full_mv     = config_get_double_or_fail(config, "vehicle.gas_full_mv");

    aduemc->emc_brake_none_mv   = config_get_double_or_fail(config, "vehicle.brake_none_mv");
    aduemc->emc_brake_full_mv   = config_get_double_or_fail(config, "vehicle.brake_full_mv");

    aduemc->emc_steer_left_mv   = config_get_double_or_fail(config, "vehicle.steer_left_mv");
    aduemc->emc_steer_center_mv = config_get_double_or_fail(config, "vehicle.steer_center_mv");
    aduemc->emc_steer_right_mv  = config_get_double_or_fail(config, "vehicle.steer_right_mv");

    aduemc->emc_shifter_state = LCMTYPES_SHIFT_ENUM_T_PARK;
    aduemc->emc_shifter_goal = LCMTYPES_SHIFT_ENUM_T_PARK;
    aduemc->emc_shifter_time_per_shift = 1.5;

    aduemc->emc_steer_state_mv = 2500;
    aduemc->emc_steer_goal_mv = 2500;
    aduemc->emc_gas_brake_state_mv = 1300;
    aduemc->emc_gas_brake_goal_mv = 1300;
    
    aduemc->adu_state = LCMTYPES_ADU_STATE_ENUM_T_RUN;

    aduemc->adu_gas_brake_state_mv = 1300;
    aduemc->adu_gas_brake_goal_mv = 1300;

    aduemc->adu_steer_state_mv = 2500;
    aduemc->adu_steer_goal_mv = 2500;

    return aduemc;
}

static inline float min(float a, float b)
{
    return (a < b) ? a : b;
}

// make state closer to goal, taking a maximum step of size maxstep
static inline float step_towards(float state, float goal, float maxstep)
{
    if (state < goal) {
        return state + min(goal-state, maxstep);
    } else {
        return state - min(state-goal, maxstep);
    }
}

// if min maps to 0, and max maps to 1, what does v map to?
static inline double normalize(double min, double max, double v)
{
    double n = (v-min) / (max-min);

    return fmax(0, fmin(1, n));
}

void model_adu_emc_step(model_adu_emc_t *aduemc, 
                        const lcmtypes_adu_command_t *adu_cmd, const lcmtypes_adu_button_command_t *button_cmd, double dt)
{
    assert(dt > 0);

    /////////////////////////////////////////////
    // adu step
    if (button_cmd) {
        aduemc->adu_button_status = button_cmd->button_status;
    }

    if (adu_cmd) {

        aduemc->utime = adu_cmd->utime;

        if (adu_cmd->steer_mv < 0 || adu_cmd->steer_mv > 5000 ||
            adu_cmd->gas_mv < 0 || adu_cmd->gas_mv > 5000) {

            aduemc->adu_state = LCMTYPES_ADU_STATE_ENUM_T_STOP;
        }
        else {
            aduemc->adu_steer_goal_mv = adu_cmd->steer_mv;
            aduemc->adu_steer_rate = adu_cmd->steer_rate_mv_per_s;
            
            aduemc->adu_gas_brake_goal_mv = adu_cmd->gas_mv;
            aduemc->adu_gas_brake_rate = adu_cmd->gas_rate_mv_per_s;
        }

        // shifting logic. We don't model any adu-induced delay in shifting,
        // and we don't simulate transitioning through gears (i.e.,
        // park->drive goes through neutral), since the ADU conceals this
        // from us.
        
        if (adu_cmd->shift_enable && aduemc->emc_shifter_goal != adu_cmd->shift_target) {
            aduemc->emc_shifter_time_since_shift_started = 0;
            aduemc->emc_shifter_goal = adu_cmd->shift_target;
            aduemc->emc_shifting = 1;

            if (aduemc->adu_gas_brake_state_mv > SHIFT_GEARS_MIN_BRAKE_MV) {
                printf("WARNING: Brake too low when shift command received! (%f)\n", aduemc->adu_gas_brake_state_mv);
            }
        }
    }

    if (aduemc->adu_state == LCMTYPES_ADU_STATE_ENUM_T_STOP) {
        aduemc->adu_gas_brake_goal_mv = 1700;
    }

    if (aduemc->adu_steer_rate == 0)
        aduemc->adu_steer_state_mv = aduemc->adu_steer_goal_mv;
    else
        aduemc->adu_steer_state_mv = step_towards(aduemc->adu_steer_state_mv, 
                                               aduemc->adu_steer_goal_mv, 
                                               dt * aduemc->adu_steer_rate);
    
    if (aduemc->adu_gas_brake_rate == 0)
        aduemc->adu_gas_brake_state_mv = aduemc->adu_gas_brake_goal_mv;
    else
        aduemc->adu_gas_brake_state_mv = step_towards(aduemc->adu_gas_brake_state_mv, 
                                                      aduemc->adu_gas_brake_goal_mv, 
                                                      dt * aduemc->adu_gas_brake_rate);

    aduemc->emc_gas_brake_goal_mv = aduemc->adu_gas_brake_state_mv;
    aduemc->emc_steer_goal_mv = aduemc->adu_steer_state_mv;

    /////////////////////////////////////////////
    // emc step
    
    aduemc->emc_gas_brake_state_mv = step_towards(aduemc->emc_gas_brake_state_mv, 
                                                  aduemc->emc_gas_brake_goal_mv, 
                                                  dt*aduemc->emc_gas_brake_rate);
    
    aduemc->emc_steer_state_mv = step_towards(aduemc->emc_steer_state_mv, 
                                              aduemc->emc_steer_goal_mv, 
                                              dt*aduemc->emc_steer_rate);
    
    if (aduemc->emc_shifter_state != aduemc->emc_shifter_goal) {
        aduemc->emc_shifter_time_since_shift_started += dt;
        if (aduemc->emc_shifter_time_since_shift_started >= aduemc->emc_shifter_time_per_shift) {
            aduemc->emc_shifter_state = aduemc->emc_shifter_goal;
            aduemc->emc_shifting = 0;
        }
    }
    
    // map voltages to actuator positions

    aduemc->emc_gas_pedal = normalize(aduemc->emc_gas_none_mv,
                                      aduemc->emc_gas_full_mv,
                                      aduemc->emc_gas_brake_state_mv);
    
    aduemc->emc_brake_pedal = normalize(aduemc->emc_brake_none_mv,
                                        aduemc->emc_brake_full_mv,
                                        aduemc->emc_gas_brake_state_mv);

    if (aduemc->emc_steer_state_mv < aduemc->emc_steer_center_mv)
        aduemc->emc_steer = - normalize(aduemc->emc_steer_center_mv,
                                        aduemc->emc_steer_left_mv,
                                        aduemc->emc_steer_state_mv);
    else
        aduemc->emc_steer = normalize(aduemc->emc_steer_center_mv,
                                      aduemc->emc_steer_right_mv,
                                      aduemc->emc_steer_state_mv);
}

void model_adu_emc_get_status(model_adu_emc_t *aduemc, lcmtypes_adu_status_t *adustatus)
{
    adustatus->utime = timestamp_now(); // aduemc->utime;
    adustatus->state = aduemc->adu_state;
    adustatus->button_status = aduemc->adu_button_status;
    adustatus->shift_target = aduemc->emc_shifter_goal;
    adustatus->shift_current = aduemc->emc_shifter_state;
    adustatus->shift_in_progress = aduemc->emc_shifting;

    adustatus->gas_mv = aduemc->adu_gas_brake_state_mv;
    adustatus->gas_goal_mv = aduemc->adu_gas_brake_goal_mv;
    adustatus->gas_rate = aduemc->adu_gas_brake_rate;

    adustatus->steer_mv = aduemc->adu_steer_state_mv;
    adustatus->steer_goal_mv = aduemc->adu_steer_goal_mv;
    adustatus->steer_rate = aduemc->adu_steer_rate;
}
