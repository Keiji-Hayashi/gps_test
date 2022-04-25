/**
 * @file nmea_gsa.hpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GSA
 * @version 0.1
 * @date 2022-04-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef NMEA_GSA_HPP
#define NMEA_GSA_HPP


#include <string>
#include <vector>

/**
 * @brief GSA
 * 
 */
class nmea_gsa
{
public:
    nmea_gsa(std::string nmea);
    int get_system_id();
    std::vector<int> get_svid_list();
    double get_pdop();
    double get_hdop();
    double get_vdop();
private:
    int nav_mode;
    std::vector<int> svid;
    double pdop;
    double hdop;
    double vdop;
    int system_id;
};

#endif
