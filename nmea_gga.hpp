/**
 * @file nmea_gga.hpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GGA
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef NMEA_GGA_HPP
#define NMEA_GGA_HPP

#include <string>

/**
 * @brief GAA
 * 
 */
class nmea_gga
{
public:
    nmea_gga (std::string nmea);
    double get_latitude();
    double get_longitude();
    double get_altitude();
    int get_num_sv();
    std::string get_time();

private:    
    double latitude;
    double longitude;
    double altitude;
    int num_sv;
    std::string time;
};

#endif