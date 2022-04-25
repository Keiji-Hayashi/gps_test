/**
 * @file nmea_rmc.hpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief RMC
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef NMEA_RMC_HPP
#define NMEA_RMC_HPP

#include <ctime>
#include <string>

/**
 * @brief RMC
 * 
 */
class nmea_rmc
{
public:
    nmea_rmc(std::string nmea);
    std::string get_local_datetime();
    std::string get_utc_datetime();
    time_t get_time_t();
    std::string get_time();

private:
    std::time_t gps_time;
    std::string date;
    std::string time;
};

#endif