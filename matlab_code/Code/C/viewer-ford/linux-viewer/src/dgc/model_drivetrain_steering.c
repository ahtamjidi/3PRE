#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <common/config.h>
#include <common/math_util.h>

#include "model_drivetrain_steering.h"

model_drivetrain_steering_t *model_drivetrain_steering_create(Config *config)
{
    model_drivetrain_steering_t *mds = (model_drivetrain_steering_t*) calloc(1, sizeof(model_drivetrain_steering_t));
    
    // lr3 defaults
        
    mds->max_engine_torque = 427; // Nm
    mds->idle_tq_fraction = 0.1;
    mds->eng_static_friction = 25; // Nm
    mds->eng_friction_coeff = 40 / (7000*2*M_PI / 60); // Nm per rad/s
    mds->eng_inertia = 0.25; // kg * m * m

    mds->couple_ratio = 0.85;
    mds->tqcnv_C1 = 1.874e-3;
    mds->tqcnv_C2 = 0.2946e-3;
    mds->tqcnv_C3 = -1.695e-3;
    mds->tqcnv_C5 = 2.2;
    mds->tqcnv_C4 = (1 - mds->tqcnv_C5) / mds->couple_ratio;
    
    mds->max_brake_torque = 20000; // Nm
    mds->drivetrain_inertia = 0.5; // kg * m * m
    mds->mass = 2579; // kg
    mds->roll_coeff = 0.01;
    mds->drag_coeff = 1.0;
    mds->wheel_radius = 0.45; // m
    mds->wheel_base = config_get_double_or_fail(config, "vehicle.wheel_base");
    mds->wheel_width = config_get_double_or_fail(config, "vehicle.wheel_width");
    mds->trans_ratios[0] = -3.40;
    mds->trans_ratios[1] = 4.17;
    mds->trans_ratios[2] = 2.34;
    mds->trans_ratios[3] = 1.52;
    mds->trans_ratios[4] = 1.14;
    mds->trans_ratios[5] = 0.87;
    mds->trans_ratios[6] = 0.69;
    mds->diff_ratio = 3.73;

    double max_turn_rate = config_get_double_or_fail(config, "vehicle.max_turn_rate_rad"); 
    mds->max_steer_rads = atan(max_turn_rate * mds->wheel_base);


    // TODO: there is a selectable "low" transfer ratio of 2.93:1 

    return mds;
}

void model_drivetrain_steering_step(model_drivetrain_steering_t *mds, struct model_drivetrain_steering_inputs *inputs, double dt)
{
    assert(fabs(inputs->steer) <= 1);
    assert(inputs->brake >=0 && inputs->brake <=1);
    assert(inputs->gas >=0 && inputs->gas <=1);
    assert(dt >= 0);

    // engine model

    int decouple_transmission = 0; // disconnect drive shaft from transmission ("perfect clutch")
    int lock_output_shaft = 0;     // disallow drive shaft rotation ("parking brake")

    switch (inputs->shifter_pos) 
    {
    case MODEL_DRIVETRAIN_SHIFTER_PARK:
        decouple_transmission = 1;
        lock_output_shaft = 1;
        break;

    case MODEL_DRIVETRAIN_SHIFTER_NEUTRAL:
        decouple_transmission = 1;
        break;

    case MODEL_DRIVETRAIN_SHIFTER_REVERSE:
        mds->gear = 0;
        break;

    case MODEL_DRIVETRAIN_SHIFTER_DRIVE:
        // TODO: automatic transmission currently stuck in first gear
        mds->gear = 1;
        break;

    default:
        assert(0);
    }

    /* Engine torque = (Indicated torque) - (Friction) - (Pump) - (Accessories)
     *
     * "Pump" refers to the torque imposed by the torque converter.  Accesories
     * are the alternator and air conditioner.  Indicated torque is the result
     * of combustion in the cylinders.
     *
     * We model friction as a linear function of rpm plus an offset.  A better
     * model would be a 2D lookup table of both rpm and indicated torque.
     */

    /* Indicated torque, the amount of torque requested by the pedal.
     * TODO: I assume the gas pedal maps linearly into indicated torque.  That's
     * probably a bad assumption. */

    double ind_tq = (mds->idle_tq_fraction + inputs->gas) /
        (mds->idle_tq_fraction + 1.0) * mds->max_engine_torque;

    // Friction torque, as a function of engine speed 
    double friction_tq = mds->eng_static_friction +
        mds->engine_rad_per_sec * mds->eng_friction_coeff;

    // Speed of the turbine in the torque converter 
    // (the speed given our current state)
    double trans_input_speed = mds->driveshaft_rad_per_sec * mds->diff_ratio *
        mds->trans_ratios[mds->gear];

    // if the transmission is decoupled from the drive shaft, it'll
    // just spin at the engine's rate (in steady state, at least)
    if (decouple_transmission)
        trans_input_speed = mds->engine_rad_per_sec;

    assert (isfinite (ind_tq));
    assert (isfinite (friction_tq));
    assert (isfinite (trans_input_speed));

    // Torque acting against the pump 
    double pump_tq = mds->tqcnv_C1 * mds->engine_rad_per_sec * mds->engine_rad_per_sec +
        mds->tqcnv_C2 * mds->engine_rad_per_sec * trans_input_speed +
        mds->tqcnv_C3 * trans_input_speed * trans_input_speed;

    // We could model alternator and air conditioner torque here...
    double acc_tq = 0;

    double eng_torque = ind_tq - friction_tq - pump_tq - acc_tq;

    mds->engine_rad_per_sec += eng_torque / mds->eng_inertia * dt;
    assert (isfinite (mds->engine_rad_per_sec));

    /* We assume the drive shaft is rigidly coupled to the road without
     * longitudinal wheel slip.
     *
     * (Drive Shaft Torque) = (Turbine Tq.) * (Trans. Ratio) * (Diff. Ratio) -
     *          (Brake Torque) - (Rolling Resistance) - (Climbing Resistance) -
     *          (Wind Resistance)
     *
     * (Shaft Inertia) = (Drivetrain Inertia) + (Mass) * (Wheel Radius)^2
     */

    // Ratio of turbine to pump angular velocity in the torque converter 
    double speed_ratio = trans_input_speed / mds->engine_rad_per_sec;

    // Compute the torque exerted on the transmission by the turbine 
    double turbine_torque = 0;
    if (!decouple_transmission) {
        // is this modeling the point at which the pump/turbine are spinning
        // at about the same rate and the stator switches to the free-wheeling
        // regime?
        if (speed_ratio < mds->couple_ratio)
            turbine_torque = (mds->tqcnv_C4 * speed_ratio +
                    mds->tqcnv_C5) * pump_tq;
        else
            turbine_torque = pump_tq;
    }

    double air_speed = mds->speed;
    double d = mds->wheel_radius;

    /* Torques that always act on the shaft in the same direction regardless of
     * the shaft's direction of rotation */

    double torque_fixeddir =
        // Torque from the turbine (passed through the trans. and differential) 
        turbine_torque * mds->trans_ratios[mds->gear] * mds->diff_ratio -
        // Climbing resistance 
        mds->mass * 9.8 * sin (inputs->climbing_rad) * d -
        // Wind resistance 
        mds->drag_coeff * air_speed * fabs (air_speed) * d;
    
    /* Torques that always act on the shaft against its direction of rotation */

    double torque_wheeldir = 
        // Brake torque 
        inputs->brake * mds->max_brake_torque +
        // Rolling resistance 
        mds->roll_coeff * mds->mass * 9.8 * cos (inputs->climbing_rad) * d;

    torque_wheeldir *= sgn (mds->driveshaft_rad_per_sec);

    //printf ("turb %f ", turbine_torque);
    //printf ("pump %f\n", pump_tq);

    double inertia = mds->drivetrain_inertia + mds->mass * d * d;

    double delta_speed = (torque_fixeddir - torque_wheeldir) / inertia * dt;

    /* Clamp the speed to zero if the braking/resistance torques exceed the
     * other torques and the speed would pass through zero otherwise. */
    if (-sgn (mds->driveshaft_rad_per_sec) * delta_speed >
            sgn (mds->driveshaft_rad_per_sec) * mds->driveshaft_rad_per_sec &&
            fabs (torque_wheeldir) > fabs (torque_fixeddir))
        mds->driveshaft_rad_per_sec = 0;
    else
        mds->driveshaft_rad_per_sec += delta_speed;

    if (lock_output_shaft)
        mds->driveshaft_rad_per_sec = 0;

    mds->speed = mds->driveshaft_rad_per_sec * mds->wheel_radius;

    // steering

    // This is a bicycle model, with some extra math to estimate
    // wheel-speeds for wheels laterally offset
    // from the center line of the bicycle.

    double b = mds->wheel_base;
    double w = mds->wheel_width;

    double K = -tan ( inputs->steer * mds->max_steer_rads ) / b;

    mds->wheel_speeds[0] = sqrt (K*K*b*b + (1 + K*w/2)*(1 + K*w/2)) * mds->speed;
    mds->wheel_speeds[1] = sqrt (K*K*b*b + (1 - K*w/2)*(1 - K*w/2)) * mds->speed;
    mds->wheel_speeds[2] = (1 + K*w/2) * mds->speed;
    mds->wheel_speeds[3] = (1 - K*w/2) * mds->speed;

    mds->yaw_rate = K * mds->speed;
}
