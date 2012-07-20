include ../common.mk

CFLAGS = $(CFLAGS_STD) $(CFLAGS_LC) -I.. -O4 -fPIC
LDFLAGS = $(LDFLAGS_STD) $(LDFLAGS_LC)

LCMTYPES = pose_t mtig_t compat_global_trans_t goal_t goal_list_t\
    adu_button_command_t adu_secondary_t compat_pose_t \
    shift_enum_t adu_state_enum_t adu_status_t adu_command_t \
    gridmap_tile_t laser_t lcgl_data_t velodyne_t \
    lane_boundary_t lane_t lane_list_t \
    pmsd_sheriff_cmd_t pmsd_deputy_cmd_t pmsd_orders_t pmsd_info_t \
    pmsd_printf_t gps_to_local_t nmea_t \
    image_lcsync_t image_t image_fragment_t image_fragment_announce_t \
    car_summary_t emc_feedback_t trans_switch_t\
    sim_lane_t sim_lane_list_t raw_t \
    mplan_step_t mplan_step_list_t \
	point2d_t pointlist2d_t \
	spline_t lane_detection_t lane_clines_t \
	comment_t \
	rect_t rect_list_t car_temps_t timesync_t \
	motion_plan_action_t motion_plan_t \
    point2d_float_t \
    waypoint_id_t \
    predicted_states_t rrt_plan_step_t rrt_plan_t rrt_steer_shift_plan_t\
    rrt_debug_node_t rrt_debug_t \
    rndf_locale_t navigator_plan_t \
    lcplot_data_t lcplot_figure_t  navigator_constraint_enum_t \
	tsim_associations_t \
	navigator_cmd_t navigator_status_t rrt_status_t rrt_cost_t \
    controller_state_t controller_aux_t rrt_plan_lsc_step_t rrt_plan_lsc_t \
	track_t track_list_t obstacle_list_t failsafe_t vel_advice_t curb_polylines_t \
	lane_velocity_advice_t rgbvelodyne_t 
 
# prevent auto-generated lcm .c/.h files from being deleted
.SECONDARY : $(LCMTYPES:lcmtypes_%.c) $(LCMTYPES:lcmtypes_%.h)

LCMTYPES_O=$(LCMTYPES:%=lcmtypes_%.o)

LCM = lcm-gen
LIBLCMTYPES = $(LIB_PATH)/liblcmtypes.a

all: $(LIBLCMTYPES)

$(LIBLCMTYPES): $(LCMTYPES_O)
	ar rc $@ $(LCMTYPES_O)

%.o: %.c %.h
	@echo "    [$@]"
	gcc $(CFLAGS) -c $< 

%.c %.h: ../../lcmtypes/%.lcm
	@echo "    [$@]"
	$(LCM) -c $<

lcmtypes_%.c lcmtypes_%.h: ../../lcmtypes/%.lcm
	@echo "    [$@]"
	$(LCM) -c $<

clean:
	rm -f *.c *.h *.o *~ $(LIBLCMTYPES)
