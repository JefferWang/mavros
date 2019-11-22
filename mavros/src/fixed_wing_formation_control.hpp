/* this software is developed by the CGNC_LAB BIT, all rights reserved.*/
/* 作者：CGNC_LAB,BIT
 * 时间：2019.11.09
 */

// ros程序必备头文件
#ifndef _FIXED_WING_FORMATION_CONTROL_HPP_
#define _FIXED_WING_FORMATION_CONTROL_HPP_

#include <ros/ros.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include "fixed_wing_sub_pub.hpp"
#include "mathlib.h"
#include "tecs.hpp"

using namespace std;

class _FIXED_WING_FORMATION_CONTROL
{

private:
	ros::Time begin_time;

	int simulate_type{0};

	int formation_type;

	_FIXED_WING_SUB_PUB fixed_wing_sub_pub;

	TECS _tecs;

	struct TECS::tecs_state tecs_outputs;

	float current_time;

	ros::NodeHandle nh;

	ros::ServiceClient set_mode_client;

	ros::ServiceClient arming_client;

	ros::ServiceClient waypoint_setcurrent_client;

	ros::ServiceClient waypoint_pull_client;

	ros::ServiceClient waypoint_push_client;

	ros::ServiceClient waypoint_clear_client;

	ros::Publisher fixed_wing_local_pos_sp_pub;

	ros::Publisher fixed_wing_global_pos_sp_pub;

	ros::Publisher fixed_wing_local_att_sp_pub;

	ros::Subscriber // 【订阅】无人机ned三向加速度
		fixed_wing_battrey_state_from_px4_sub;

	ros::Subscriber // 【订阅】无人机ned三向加速度
		fixed_wing_wind_estimate_from_px4_sub;

	ros::Subscriber // 【订阅】无人机ned三向加速度
		fixed_wing_acc_ned_from_px4_sub;

	ros::Subscriber // 【订阅】无人机ned三向速度
		fixed_wing_velocity_ned_fused_from_px4_sub;

	ros::Subscriber // 【订阅】无人机ned位置
		fixed_wing_local_position_from_px4;

	ros::Subscriber // 【订阅】无人机gps三向速度
		fixed_wing_velocity_global_fused_from_px4_sub;

	ros::Subscriber // 【订阅】无人机ump位置
		fixed_wing_umt_position_from_px4_sub;

	ros::Subscriber //【订阅】无人机gps相对alt
		fixed_wing_global_rel_alt_from_px4_sub;

	ros::Subscriber // 【订阅】无人机gps位置
		fixed_wing_global_position_form_px4_sub;

	ros::Subscriber // 【订阅】无人机imu信息，
		fixed_wing_imu_sub;

	ros::Subscriber // 【订阅】无人机当前模式
		fixed_wing_states_sub;

	ros::Subscriber // 【订阅】无人机当前航点
		fixed_wing_waypoints_sub;

	ros::Subscriber // 【订阅】无人机到达的航点
		fixed_wing_waypointsreach_sub;

	ros::Subscriber // 【订阅】无人机的高度
		fixed_wing_altitude_from_px4_sub;
	ros::Subscriber // 【订阅】无人机的空速地速
		fixed_wing_air_ground_speed_from_px4_sub;

	struct _s_fixed_wing_status
	{
		bool altitude_lock{false};

		bool in_air{true};

		string mode;

		float rotmat[3][3];

		float ground_speed_ned_param1{-20000};

		float ground_speed_ned_param2{-20000};

		float ground_speed{-20000};

		float global_vel_x{-20000};

		float global_vel_y{-20000};

		float global_vel_z{-20000};

		float air_speed{-20000};

		float wind_estimate_x{-20000};

		float wind_estimate_y{-20000};

		float wind_estimate_z{-20000};

		float relative_hight{-20000};

		float latitude{-20000};

		float altitude{-20000};

		float longtitude{-20000};

		float relative_alt{-20000};

		float ned_altitude{-20000};

		float ned_pos_x{-20000};

		float ned_pos_y{-20000};

		float ned_pos_z{-20000};

		float ned_vel_x{-20000};

		float ned_vel_y{-20000};

		float ned_vel_z{-20000};

		float ned_acc_x{-20000};

		float ned_acc_y{-20000};

		float ned_acc_z{-20000};

		float ned_acc[3];

		float body_acc_x{-20000};

		float body_acc_y{-20000};

		float body_acc_z{-20000};

		float body_acc[3];

		float pitch_angle{-20000};

		float yaw_angle{-20000};

		float roll_angle{-20000};

		float battery_voltage{-20000};

		float battery_precentage{-20000};

		float battery_current{-20000};

	} leader_status, follower_status;

	struct _s_follower_setpiont
	{
		string mode;

		float ground_speed_ned_param1{0};

		float ground_speed_ned_param2{0};

		float global_vel_x{0};

		float global_vel_y{0};

		float global_vel_z{0};

		float air_speed{0};

		float relative_hight{0};

		float latitude{0};

		float altitude{0};

		float longtitude{0};

		float relative_alt{0};

		float ned_pos_x{0};

		float ned_pos_y{0};

		float ned_pos_z{0};

		float ned_vel_x{0};

		float ned_vel_y{0};

		float ned_vel_z{0};

		float ned_acc_x{0};

		float ned_acc_y{0};

		float ned_acc_z{0};

		float body_acc_x{0};

		float body_acc_y{0};

		float boday_acc_z{0};

		float pitch_angle{0};

		float yaw_angle{0};

		float roll_angle{0};

		float thrust{0};

	} follower_setpoint, leader_setpoint, sp_to_send;

	struct _s_fixed_wing_params
	{
		int EAS2TAS{1};

		bool climboutdem{false};

		float climbout_pitch_min_rad{0.2};

		float throttle_min{0.1};

		float throttle_max{1};

		float throttle_cruise{0.1};

		float pitch_min_rad{-0.5};

		float pitch_max_rad{0.5};

		float speed_weight{1};

		float time_const_throt{1.0};

		float time_const{5.0};
	} params;

	struct _s_error_followern
	{
		float latitude{0};

		float altitude{0};

		float longtitude{0};

		float n_diatance{0};

		float e_distance{0};

		float airspeed{0};

		float groundspeed{0};

		float distance_level{0};

		float distance_vertical{0};

		float distance_3d{0};

		float ned_vel_x{0};

		float ned_vel_y{0};

	} error_follwer1;

	struct _s_formation_params
	{
		float v_kp{0};

		float altitude_offset{0};

		float longtitude_offset{0};

		float latitude_offset{0};

	} formation_params;

	string control_mode_prev;

	string control_mode_current;

	struct _s_control_lateral_params
	{
		float kp{0};
		float kd{0};
	} control_lateral_params;

public:
	void write_to_files(string file_path_name, float time_stamp, float data);

	float get_ros_time(ros::Time begin); //获取ros当前时间

	float get_air_speed(int type);

	void run(int argc, char **argv);

	bool update_follwer_status(_FIXED_WING_SUB_PUB *fixed_wing_sub_pub_pointer);

	void update_rotmat();

	bool set_fixed_wing_mode(_FIXED_WING_SUB_PUB *fixed_wing_sub_pub_pointer, string setpoint_mode);

	bool update_leader_status();

	void show_fixed_wing_status(int PlaneID);

	void show_fixed_wing_setpoint(int PlaneID);

	void show_control_state(_FIXED_WING_SUB_PUB *fixed_wing_sub_pub_pointer);

	void test();

	void ros_sub_and_pub(_FIXED_WING_SUB_PUB *fixed_wing_sub_pub_poiter);

	void handle_status_from_receiver();

	void handle_message_from_px4();

	void send_the_command_to_px4();

	void send_message_to_sender();

	void control_formation();

	void trans_the_sp_for_send();

	void send_setpoint_to_px4(_FIXED_WING_SUB_PUB *fixed_wing_sub_pub_pointer);

	void send_setpoint_to_ground_station();

	void calculate_error();

	void foramtion_demands_update(int formation_type);

	void calculate_the_desire_airspeed();

	void control_vertical(float current_time);

	void show_tecs_status();

	void control_lateral(float current_time);
};

#endif