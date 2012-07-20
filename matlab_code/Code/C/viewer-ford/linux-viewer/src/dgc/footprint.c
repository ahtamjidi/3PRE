#include <stdio.h>

#include <common/rotations.h>
#include "footprint.h"

int
footprint_get_front_bumper_pose (CTrans * ctrans, Config * config, lcmtypes_pose_t * pose)
{
    if (ctrans_local_pose (ctrans, pose) < 0)
        return -1;

    /* TODO: get this value from the config file. */
    double pos[3] = { 0, 0, 0 };
    pos[0] = config_get_double_or_default (config,
            "control.vehicle_constants.rear_axle_to_front_bumper", 4.0);

    rot_quat_rotate (pose->orientation, pos);
    pose->pos[0] += pos[0];
    pose->pos[1] += pos[1];
    pose->pos[2] += pos[2];
    return 0;
}

int
footprint_get_rear_bumper_pose (CTrans * ctrans, Config * config, lcmtypes_pose_t * pose)
{
    if (ctrans_local_pose (ctrans, pose) < 0)
        return -1;

    /* TODO: get this value from the config file. */
    double pos[3] = { 0, 0, 0 };
    pos[0] = -config_get_double_or_default (config,
            "control.vehicle_constants.rear_bumper_to_rear_axle", 2.0);

    rot_quat_rotate (pose->orientation, pos);
    pose->pos[0] += pos[0];
    pose->pos[1] += pos[1];
    pose->pos[2] += pos[2];
    return 0;
}
