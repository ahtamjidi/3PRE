#ifndef _MODEL_DRIVETRAIN_STEERING_H
#define _MODEL_DRIVETRAIN_STEERING_H

#define MODEL_DRIVETRAIN_SHIFTER_PARK    0
#define MODEL_DRIVETRAIN_SHIFTER_DRIVE   1
#define MODEL_DRIVETRAIN_SHIFTER_REVERSE 2
#define MODEL_DRIVETRAIN_SHIFTER_NEUTRAL 3

// This is a model of an automatic transmission system with
// user-selectable park/drive/reverse/neutral.

typedef struct _model_drivetrain_steering model_drivetrain_steering_t;
struct _model_drivetrain_steering
{
    // steering parameters:

    // maximum angle of wheels for a normalized steering input of +/- 1
    // NOT angle of steering wheel, but the wheels themselves.
    double max_steer_rads;

    // drivetrain parameters

    // TODO: make this a curve dependent on rpm, not a constant 
    double max_engine_torque;   // Nm
    double idle_tq_fraction;    // baseline amount of engine torque (when gas=0)
    double eng_static_friction; // Nm (about 30)
    double eng_friction_coeff;  // Nm per (rads/s) (about 40/7000)
    double eng_inertia;

    double tqcnv_C1; // 1.874e-3
    double tqcnv_C2; // 0.2946e-3
    double tqcnv_C3; // -1.695e-3
    double tqcnv_C4; // -1.412
    double tqcnv_C5; // 2.2
    double couple_ratio; // 0.85

    double trans_ratios[7];
    double diff_ratio;         // gear ratio of differential

    double max_brake_torque;   // Nm, corresponds to brake = 1.0
    double drivetrain_inertia;
    double mass;
    double roll_coeff;
    double drag_coeff;
    double wheel_radius;
    double wheel_base;         // distance between front and back wheels
    double wheel_width;        // distance between left and right wheels

    /////////////////////////////////////////////
    // state:
    double engine_rad_per_sec; // like RPM, just rad/s
    double driveshaft_rad_per_sec;  // drive shaft speed, rad/s

    double speed;              // speed (signed) of center of rear axle
    double wheel_speeds[4];    // speed (signed) of each individual wheel

    double yaw_rate;           // rate at which vehicle is rotating around zaxis

    int    gear;               // the currently selected transmission gear (i.e., index into trans_ratios)
};

struct model_drivetrain_steering_inputs
{
    double gas;          // [0,1]    (1 = maximum acceleration, 0 = unpressed)
    double brake;        // [0,1]    (1 = maximum deceleration, 0 = unpressed)
    double steer;        // [-1,1]   (-1 = max left steer, 1 = max right steer)
    int    shifter_pos;  // position of shifter selector (MODEL_DRIVETRAIN_SHIFTER_**)
    double climbing_rad; // are we going up hill? (radians)
};

model_drivetrain_steering_t *model_drivetrain_steering_create(Config *config);

void model_drivetrain_steering_step(model_drivetrain_steering_t *mds, struct model_drivetrain_steering_inputs *inputs, double dt);

#endif
