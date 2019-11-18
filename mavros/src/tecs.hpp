//#include <ros/ros.h>
//#include <std_msgs/String.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <iostream>
#include "mathlib.h"

#define CONSTANTS_ONE_G 9.80665

class TECS
{
public:
    TECS() : _tecs_state{},
             _update_50hz_last_usec(0),
             _update_speed_last_usec(0),
             _update_pitch_throttle_last_usec(0),
             // TECS tuning parameters
             _hgtCompFiltOmega(0.0f),
             _spdCompFiltOmega(0.0f),
             _maxClimbRate(2.0f),
             _minSinkRate(1.0f),
             _maxSinkRate(2.0f),
             _timeConst(5.0f),
             _timeConstThrot(8.0f),
             _ptchDamp(0.0f),
             _thrDamp(0.0f),
             _integGain(0.0f),
             _vertAccLim(0.0f),
             _rollComp(0.0f),
             _spdWeight(0.5f),
             _heightrate_p(0.0f),
             _heightrate_ff(0.0f),
             _speedrate_p(0.0f),
             _throttle_dem(0.0f),
             _pitch_dem(0.0f),
             _integ1_state(0.0f),
             _integ2_state(0.0f),
             _integ3_state(0.0f),
             _integ4_state(0.0f),
             _integ5_state(0.0f),
             _integ6_state(0.0f),
             _integ7_state(0.0f),
             _last_throttle_dem(0.0f),
             _last_pitch_dem(0.0f),
             _vel_dot(0.0f),
             _EAS(0.0f),
             _TASmax(30.0f),
             _TASmin(3.0f),
             _TAS_dem(0.0f),
             _TAS_dem_last(0.0f),
             _EAS_dem(0.0f),
             _hgt_dem(0.0f),
             _hgt_dem_in_old(0.0f),
             _hgt_dem_adj(0.0f),
             _hgt_dem_adj_last(0.0f),
             _hgt_rate_dem(0.0f),
             _hgt_dem_prev(0.0f),
             _TAS_dem_adj(0.0f),
             _TAS_rate_dem(0.0f),
             _STEdotErrLast(0.0f),
             _underspeed(false),
             _detect_underspeed_enabled(true),
             _badDescent(false),
             _climbOutDem(false),
             _pitch_dem_unc(0.0f),
             _STEdot_max(0.0f),
             _STEdot_min(0.0f),
             _THRmaxf(0.0f),
             _THRminf(0.0f),
             _PITCHmaxf(0.5f),
             _PITCHminf(-0.5f),
             _SPE_dem(0.0f),
             _SKE_dem(0.0f),
             _SPEdot_dem(0.0f),
             _SKEdot_dem(0.0f),
             _SPE_est(0.0f),
             _SKE_est(0.0f),
             _SPEdot(0.0f),
             _SKEdot(0.0f),
             _STE_error(0.0f),
             _STEdot_error(0.0f),
             _SEB_error(0.0f),
             _SEBdot_error(0.0f),
             _DT(0.02f),
             _airspeed_enabled(false),
             _states_initalized(false),
             _in_air(false),
             _throttle_slewrate(0.0f),
             _indicated_airspeed_min(3.0f),
             _indicated_airspeed_max(30.0f)

    {
    }
    enum ECL_TECS_MODE
    {
        ECL_TECS_MODE_NORMAL = 0,
        ECL_TECS_MODE_UNDERSPEED,
        ECL_TECS_MODE_BAD_DESCENT,
        ECL_TECS_MODE_CLIMBOUT
    };

    struct tecs_state
    {
        uint64_t timestamp;
        float altitude_filtered;
        float altitude_sp;
        float altitude_rate;
        float altitude_rate_sp;
        float airspeed_filtered;
        float airspeed_sp;
        float airspeed_rate;
        float airspeed_rate_sp;
        float energy_error_integ;
        float energy_distribution_error_integ;
        float total_energy_error;
        float total_energy_rate_error;
        float energy_distribution_error;
        float energy_distribution_rate_error;
        float throttle_integ;
        float pitch_integ;
        enum ECL_TECS_MODE mode;
    };

    bool airspeed_sensor_enabled()
    {
        return _airspeed_enabled;
    }

    void enable_airspeed(bool enabled)
    {
        _airspeed_enabled = enabled;
    }
    void update_pitch_throttle(float time_now, const float rotMat[3][3], float pitch, float baro_altitude, float hgt_dem,
                               float EAS_dem, float indicated_airspeed, float EAS2TAS, bool climbOutDem, float ptchMinCO,
                               float throttle_min, float throttle_max, float throttle_cruise, float pitch_limit_min, float pitch_limit_max);
    // Update of the estimated height and height rate internal state
    // Update of the inertial speed rate internal state
    // Should be called at 50Hz or greater
    void update_state(float time_now, float baro_altitude, float airspeed, const float rotMat[3][3],
                      const float accel_body[3], const float accel_earth[3], bool altitude_lock, bool in_air);

    // demanded throttle in percentage
    // should return 0 to 100
    float get_throttle_demand(void)
    {
        return _throttle_dem;
    }
    int get_throttle_demand_percent(void)
    {
        return get_throttle_demand();
    }

    void reset_state()
    {
        _states_initalized = false;
    }

    float get_pitch_demand() { return _pitch_dem; }

    // demanded pitch angle in centi-degrees
    // should return between -9000 to +9000
    int get_pitch_demand_cd() { return int(get_pitch_demand() * 5729.5781f); }

    // Rate of change of velocity along X body axis in m/s^2
    float get_VXdot(void) { return _vel_dot; }
    float get_speed_weight()
    {
        return _spdWeight;
    }
    //这个函数很重要，获得计算之后的值
    void get_tecs_state(struct tecs_state &state)
    {
        state = _tecs_state;
    }

    void set_time_const(float time_const)
    {
        _timeConst = time_const;
    }

    void set_time_const_throt(float time_const_throt)
    {
        _timeConstThrot = time_const_throt;
    }

    void set_min_sink_rate(float rate)
    {
        _minSinkRate = rate;
    }

    void set_max_sink_rate(float sink_rate)
    {
        _maxSinkRate = sink_rate;
    }

    void set_max_climb_rate(float climb_rate)
    {
        _maxClimbRate = climb_rate;
    }

    void set_throttle_damp(float throttle_damp)
    {
        _thrDamp = throttle_damp;
    }

    void set_integrator_gain(float gain)
    {
        _integGain = gain;
    }

    void set_vertical_accel_limit(float limit)
    {
        _vertAccLim = limit;
    }

    void set_height_comp_filter_omega(float omega)
    {
        _hgtCompFiltOmega = omega;
    }

    void set_speed_comp_filter_omega(float omega)
    {
        _spdCompFiltOmega = omega;
    }

    void set_roll_throttle_compensation(float compensation)
    {
        _rollComp = compensation;
    }

    void set_speed_weight(float weight)
    {
        _spdWeight = weight;
    }

    void set_pitch_damping(float damping)
    {
        _ptchDamp = damping;
    }

    void set_throttle_slewrate(float slewrate)
    {
        _throttle_slewrate = slewrate;
    }

    void set_indicated_airspeed_min(float airspeed)
    {
        _indicated_airspeed_min = airspeed;
    }

    void set_indicated_airspeed_max(float airspeed)
    {
        _indicated_airspeed_max = airspeed;
    }

    void set_heightrate_p(float heightrate_p)
    {
        _heightrate_p = heightrate_p;
    }

    void set_heightrate_ff(float heightrate_ff)
    {
        _heightrate_ff = heightrate_ff;
    }

    void set_speedrate_p(float speedrate_p)
    {
        _speedrate_p = speedrate_p;
    }

    void set_detect_underspeed_enabled(bool enabled)
    {
        _detect_underspeed_enabled = enabled;
    }

    // in case of a height reset driven by the estimator we need
    // to allow TECS to swallow the step in height and demanded height instantaneously
    void handle_alt_step(float delta_alt, float altitude)
    {
        // add height reset delta to all variables involved
        // in filtering the demanded height
        _hgt_dem_in_old += delta_alt;
        _hgt_dem_prev += delta_alt;
        _hgt_dem_adj_last += delta_alt;

        // reset height states
        _integ3_state = altitude;
        _integ1_state = _integ2_state = 0.0f;
    }

private:
    struct tecs_state _tecs_state;
    float _update_50hz_last_usec;
    // TECS tuning parameters
    float _hgtCompFiltOmega;
    float _spdCompFiltOmega;
    float _maxClimbRate;
    float _minSinkRate;
    float _maxSinkRate;
    float _timeConst;
    float _timeConstThrot;
    float _ptchDamp;
    float _thrDamp;
    float _integGain;
    float _vertAccLim;
    float _rollComp;
    float _spdWeight;
    float _heightrate_p;
    float _heightrate_ff;
    float _speedrate_p;
    // Time since last update of main TECS loop (seconds)
    float _DT;
    static constexpr float DT_MIN = 0.001;
    static constexpr float DT_DEFAULT = 0.02;
    static constexpr float DT_MAX = 1.0;

    bool _states_initalized;
    bool _in_air;

    // Last time update_pitch_throttle was called
    float _update_pitch_throttle_last_usec;
    // throttle demand in the range from 0.0 to 1.0
    float _throttle_dem;

    // pitch angle demand in radians
    float _pitch_dem;

    // Integrator state 1 - height filter second derivative
    float _integ1_state;

    // Integrator state 2 - height rate
    float _integ2_state;

    // Integrator state 3 - height
    float _integ3_state;

    // Integrator state 4 - airspeed filter first derivative
    float _integ4_state;

    // Integrator state 5 - true airspeed
    float _integ5_state;

    // Integrator state 6 - throttle integrator
    float _integ6_state;

    // Integrator state 7 - pitch integrator
    float _integ7_state;

    // throttle demand rate limiter state
    float _last_throttle_dem;

    // pitch demand rate limiter state
    float _last_pitch_dem;

    // Rate of change of speed along X axis
    float _vel_dot;

    // Equivalent airspeed
    float _EAS;

    // Maximum and minimum floating point throttle limits
    float _THRmaxf;
    float _THRminf;
    // Maximum and minimum floating point pitch limits
    float _PITCHmaxf;
    float _PITCHminf;

    // pitch demand before limiting
    float _pitch_dem_unc;
    // height demands
    float _hgt_dem;
    float _hgt_dem_in_old;
    float _hgt_dem_adj;
    float _hgt_dem_adj_last;
    float _hgt_rate_dem;
    float _hgt_dem_prev;
    // Current and last true airspeed demand
    float _TAS_dem;
    float _TAS_dem_last;

    // Speed demand after application of rate limiting
    // This is the demand tracked by the TECS control loops
    float _TAS_dem_adj;

    // Speed rate demand after application of rate limiting
    // This is the demand tracked by the TECS control loops
    float _TAS_rate_dem;
    // Underspeed condition
    bool _underspeed;

    // Underspeed detection enabled
    bool _detect_underspeed_enabled;

    // Bad descent condition caused by unachievable airspeed demand
    bool _badDescent;

    // climbout mode
    bool _climbOutDem;

    float _indicated_airspeed_min;
    float _indicated_airspeed_max;

    float _update_speed_last_usec;

    // Equivalent airspeed demand
    float _EAS_dem;

    // True airspeed limits
    float _TASmax;
    float _TASmin;
    // Maximum and minimum specific total energy rate limits
    float _STEdot_max;
    float _STEdot_min;

    // Specific energy quantities
    float _SPE_dem;
    float _SKE_dem;
    float _SPEdot_dem;
    float _SKEdot_dem;
    float _SPE_est;
    float _SKE_est;
    float _SPEdot;
    float _SKEdot;

    // Specific energy error quantities
    float _STE_error;

    // Energy error rate
    float _STEdot_error;

    float _throttle_slewrate;

    // Total energy rate filter state
    float _STEdotErrLast;
    // Specific energy balance error
    float _SEB_error;

    // Specific energy balance error rate
    float _SEBdot_error;

    bool _airspeed_enabled;

    // Initialise states and variables
    void _initialise_states(float pitch, float throttle_cruise, float baro_altitude, float ptchMinCO_rad, float EAS2TAS);

    // Update the airspeed internal state using a second order complementary filter
    void _update_speed(float time_now, float airspeed_demand, float indicated_airspeed,
                       float indicated_airspeed_min, float indicated_airspeed_max, float EAS2TAS);
    void _update_STE_rate_lim();
    void _detect_underspeed();
    void _update_speed_demand();
    // Update the demanded height
    void _update_height_demand(float demand, float state);
    void _update_energies();
    void _update_throttle(float throttle_cruise, const float rotMat[3][3]);
    void _update_pitch();
};