#ifndef __mv_mapping_h__
#define __mv_mapping_h__

#include    <inttypes.h>
#include    <common/config.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct _mv_mapping_params_t {
    int32_t mv_max;     // [mV] upper limit of throttle command
    int32_t mv_pos;     // [mV] start applying the gas (RPM starts responding)
    double  u_pos;      // [dimensionless] normalized u corresponding to mv_pos
    int32_t mv_neg;     // [mV] start applying the brake (steady state speed is 
                        // slower than coasting)
    double  u_neg;      // [dimensionless] normalized u corresponding to mv_neg
    int32_t mv_min;     // [mV] lower limit of brake command
    int32_t steer_center_volt;      // [mV] steer center voltage measured in 
                                    // field
    int32_t steer_range_volt;       // [mV] range of steer voltage == 
                                    // max_steer_volt - min_steer_volt
    double  max_steer;              // [rad] maximum steering angle
};

typedef struct _mv_mapping_params_t mv_mapping_params_t;

int     get_mv_mapping_params (mv_mapping_params_t *params, Config *config);
int32_t steer_angle_to_steer_mv (const double steer_angle, 
                                 const mv_mapping_params_t *params);
double  steer_mv_to_steer_angle (const int32_t steer_mv, 
                                 const mv_mapping_params_t *params);
int32_t control_to_gas_brake_mv (const double u, 
                                 const mv_mapping_params_t *params);
double  gas_brake_mv_to_control (const int32_t gas_brake_mv, 
                                 const mv_mapping_params_t *params);

#ifdef  __cplusplus
}
#endif

#endif

