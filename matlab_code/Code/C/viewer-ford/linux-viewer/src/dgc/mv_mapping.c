#include    <assert.h>
#include    <math.h>    // for definition of M_PI, M_PI_2
#include    <stdint.h>
#include    <common/config.h>
#include    "mv_mapping.h"

int
get_mv_mapping_params (mv_mapping_params_t *params, Config *config)
{
// Helper macros
#define GETCFD(key, val) \
    if (config_get_double (config, key, val) != 0) { \
        fprintf (stderr, "MV_MAPPING: Error reading %s!\n", key); \
        return -1; \
    }
#define GETCFI(key, val) \
    if (config_get_int (config, key, val) != 0) { \
        fprintf (stderr, "MV_MAPPING: Error reading %s!\n", key); \
        return -1; \
    }
// Helper macros

    GETCFI ("control.vehicle_constants.mv_max", &(params->mv_max));
    GETCFI ("control.vehicle_constants.mv_pos", &(params->mv_pos));
    GETCFD ("control.vehicle_constants.u_pos", &(params->u_pos));
    GETCFI ("control.vehicle_constants.mv_neg", &(params->mv_neg));
    GETCFD ("control.vehicle_constants.u_neg", &(params->u_neg));
    GETCFI ("control.vehicle_constants.mv_min", &(params->mv_min));
    GETCFI ("control.vehicle_constants.steer_center_volt", 
                        &(params->steer_center_volt));
    GETCFI ("control.vehicle_constants.steer_range_volt", 
                        &(params->steer_range_volt));
    GETCFD ("control.vehicle_constants.max_steer", &(params->max_steer));

#undef  GETCFD
#undef  GETCFI

    // Sanity checks
    int32_t mv_min, mv_neg, mv_pos, mv_max;
    int32_t steer_center_volt, steer_range_volt;
    double  u_neg, u_pos, max_steer;

    mv_min  = params->mv_min;
    mv_neg  = params->mv_neg;
    mv_pos  = params->mv_pos;
    mv_max  = params->mv_max;
    steer_center_volt   = params->steer_center_volt;
    steer_range_volt    = params->steer_range_volt;
    u_neg       = params->u_neg;
    u_pos       = params->u_pos;
    max_steer   = params->max_steer;

// Helper macros
#define COMPCONSTE(var, constant, op, pt) \
    if (var op constant) { \
        fprintf (stderr, "MV_MAPPING: (Error) " #var " = " pt " " #op " " pt \
                    "!\n", var, constant); \
        return -1; \
    }
#define COMPCONSTW(var, constant, op, pt) \
    if (var op constant) { \
        fprintf (stderr, "MV_MAPPING: (Warning) " #var " = " pt " " #op " " \
                    pt "!\n", var, constant); \
    }
#define COMPVARSE(var1, var2, op, pt) \
    if (var1 op var2) { \
        fprintf (stderr, "MV_MAPPING: (Error) " #var1 " = " pt " " #op " " \
                    #var2 " = " pt "!\n", var1, var2); \
        return -1; \
    }
#define COMPVARSW(var1, var2, op, pt) \
    if(var1 op var2) { \
        fprintf (stderr, "MV_MAPPING: (Warning) " #var1 " = " pt " " #op " " \
                    #var2 " = " pt "!\n", var1, var2); \
    }
// Helper macros

    // This is replicated in pure_pursuit.c, mv_mapping.c, sclsc.c
    COMPCONSTE (max_steer, 0.0, <=, "%e");
    COMPCONSTE (max_steer, M_PI_2, >=, "%e");
    COMPCONSTW (max_steer, 10.0 * M_PI / 180.0, <=, "%e");
    COMPCONSTW (max_steer, 45.0 * M_PI / 180.0, >=, "%e");
    // This is replicated in pure_pursuit.c, mv_mapping.c, sclsc.c

    COMPCONSTE (steer_center_volt, 0, <, "%d");
    COMPCONSTE (steer_center_volt, 5000, >, "%d");
    COMPCONSTW (steer_center_volt, 2300, <, "%d");
    COMPCONSTW (steer_center_volt, 2700, >, "%d");
    COMPCONSTE (steer_range_volt, 5000, >, "%d");
    COMPCONSTE (steer_range_volt, 3000, <, "%d");
    COMPCONSTW (steer_range_volt, 3500, <, "%d");
    int32_t min_steer_volt = steer_center_volt - 
                                (int32_t)ceil (steer_range_volt * 0.5);
    int32_t max_steer_volt = steer_center_volt + 
                                (int32_t)ceil (steer_range_volt * 0.5);
    COMPCONSTE (min_steer_volt, 0, <, "%d");
    COMPCONSTE (max_steer_volt, 5000, >, "%d");

    COMPCONSTE (u_neg, -1.0, <=, "%e");
    COMPVARSE (u_neg, u_pos, >=, "%e");
    COMPCONSTE (u_pos, 1.0, >=, "%e");
    COMPCONSTW (u_neg, 0.0, >, "%e");
    COMPCONSTW (u_pos, 0.0, <, "%e");

    COMPCONSTE (mv_min, 0, <, "%d");
    COMPVARSE (mv_min, mv_neg, >=, "%d");
    COMPVARSE (mv_neg, mv_pos, >=, "%d");
    COMPVARSE (mv_pos, mv_max, >=, "%d");
    COMPCONSTE (mv_max, 5000, >, "%d");
    COMPCONSTW (mv_neg, 2500, >, "%d");
    COMPCONSTW (mv_pos, 2500, <, "%d");

#undef  COMPCONSTE
#undef  COMPCONSTW
#undef  COMPVARSE
#undef  COMPVARSW

    return 0;
}

int32_t
steer_angle_to_steer_mv (const double steer_angle, 
                         const mv_mapping_params_t *params)
{
    double  steer_frac = -steer_angle / params->max_steer;
    assert (fabs (steer_frac) <= 1.0);

    return params->steer_center_volt + 
            (int32_t)(steer_frac * 0.5 * params->steer_range_volt);
}

double
steer_mv_to_steer_angle (const int32_t steer_mv, 
                         const mv_mapping_params_t *params)
{
    double  frac = -2.0 * ((double)(steer_mv - params->steer_center_volt)) / 
                    ((double)params->steer_range_volt);
    assert (fabs (frac) <= 1.0);

    return frac * params->max_steer;
}

int32_t
control_to_gas_brake_mv (const double u, const mv_mapping_params_t *params)
{
    assert (fabs (u) <= 1.0);

    double  u_pos = params->u_pos;
    double  u_neg = params->u_neg;

    if (u >= u_pos) {           // accelerating
        double  frac = (u - u_pos) / (1.0 - u_pos);
        int32_t mv_pos = params->mv_pos;
        return mv_pos + (int32_t)(frac * (params->mv_max - mv_pos));
    } else if (u > u_neg) {     // coasting
        double  frac = (u - u_neg) / (u_pos - u_neg);
        int32_t mv_neg = params->mv_neg;
        return mv_neg + (int32_t)(frac * (params->mv_pos - mv_neg));
    } else {                    // braking
        double  frac = (u_neg - u) / (u_neg + 1.0);     // frac >= 0.0
        int32_t mv_neg = params->mv_neg;
        return mv_neg - (int32_t)(frac * (mv_neg - params->mv_min));
    }
}

double
gas_brake_mv_to_control (const int32_t gas_brake_mv, 
                         const mv_mapping_params_t *params)
{
    int32_t mv_pos = params->mv_pos;
    int32_t mv_neg = params->mv_neg;

    if (gas_brake_mv >= mv_pos) {       // accelerating
        double  frac = (double)(gas_brake_mv - mv_pos) / 
                        (double)(params->mv_max - mv_pos);
        assert (frac <= 1.0);
        double  u_pos = params->u_pos;
        return u_pos + frac * (1.0 - u_pos);
    } else if (gas_brake_mv > mv_neg) { // coasting
        double  frac = (double)(gas_brake_mv - mv_neg) / 
                        (double)(mv_pos - mv_neg);
        double  u_neg = params->u_neg;
        return u_neg + frac * (params->u_pos - u_neg);
    } else {                            // braking
        double  frac = (double)(mv_neg - gas_brake_mv) / 
                        (double)(mv_neg - params->mv_min);  // frac >= 0.0
        assert (frac <= 1.0);
        double  u_neg = params->u_neg;
        return u_neg - frac * (u_neg + 1.0);
    }
}

