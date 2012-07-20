#ifndef __FOOTPRINT_H__
#define __FOOTPRINT_H__

#include <common/config.h>
#include <dgc/ctrans.h>
#include <lcmtypes/lcmtypes_pose_t.h>

int footprint_get_front_bumper_pose (CTrans * ctrans, Config * config, lcmtypes_pose_t * pose);
int footprint_get_rear_bumper_pose (CTrans * ctrans, Config * config, lcmtypes_pose_t * pose);

#endif

