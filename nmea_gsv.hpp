/**
 * @file nmea_gsv.hpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GSV
 * @version 0.1
 * @date 2022-04-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef NMEA_GSV_HPP
#define NMEA_GSV_HPP

#include <string>
#include <vector>

/**
 * @brief GSV
 * 
 */
class nmea_gsv
{
public:
    class sv_info {
    public:
        int svid;
        int elv;
        int az;
        int cno;
        int sys;
    };
    nmea_gsv(std::string nmea);
    int get_system_id();
    bool find_svid(int svid);
    sv_info get_svinfo(int svid);
    std::vector<int> get_svid_list();
private:
    int system_id;
    std::vector<sv_info> sv_list;
    int signal_id;
};

#endif
