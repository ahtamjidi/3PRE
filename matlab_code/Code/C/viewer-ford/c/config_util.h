#ifndef __config_util_h__
#define __config_util_h__

#include "config.h"
#include "lcmtypes_pose_t.h"

#ifdef __cplusplus
extern "C" {
#endif

int config_util_get_quat(Config *cfg, const char *name, double quat[4]);
int config_util_get_pos(Config *cfg, const char *name, double pos[3]);
int config_util_get_matrix(Config *cfg, const char *name, double m[16]);
int config_util_sensor_to_local(Config *cfg, const char *name, double m[16]);
int config_util_sensor_to_local_at(Config *cfg, const char *name, double m[16], 
        int64_t utime);
int config_util_sensor_to_local_with_pose(Config *cfg, const char *name, 
        double m[16], lcmtypes_pose_t *p);

#ifdef __cplusplus
}
#endif

#endif
