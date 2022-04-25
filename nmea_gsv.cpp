/**
 * @file nmea_gsv.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GSV
 * @version 0.1
 * @date 2022-04-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "nmea_gsv.hpp"
#include <iostream>

/**
 * @brief Construct a new nmea gsv::nmea gsv object
 * 
 * @param nmea 
 */
nmea_gsv::nmea_gsv(std::string nmea)
{
    // Structure
    //      $xxGSV,numMsg,msgNum,numSV{,svid,elv,az,cno},signalId*cs\r\n
    // Examples
    //      $GPGSV,3,1,09,09,,,17,10,,,40,12,,,49,13,,,35,1*6F\r\n
    //      $GPGSV,3,2,09,15,,,44,17,,,45,19,,,44,24,,,50,1*64\r\n
    //      $GPGSV,3,3,09,25,,,40,1*6E\r\n
    //      $GPGSV,1,1,03,12,,,42,24,,,47,32,,,37,5*66\r\n
    //      $GAGSV,1,1,00,2*76\r\n
    try {
        std::vector<std::string> items;
        size_t pos;
        while ((pos = nmea.find_first_of(",*")) != std::string::npos) {
            items.push_back(nmea.substr(0, pos));
            nmea.erase(0, pos + 1);
        }
        items.push_back(nmea);
        if (items[0] == "$GPGSV") {
            // GPS,SBAS
            system_id = 1;
        }
        else if (items[0] == "$GLGSV") {
            // GLONASS
            system_id = 2;
        }
        else if (items[0] == "$GAGSV") {
            // Galileo
            system_id = 3;
        }
        else if (items[0] == "$GBGSV") {
            // DeiDou
            system_id = 4;
        }
        else {
            // Other
            system_id = 0;
        }
        int num_msg = std::stoi(items[1]);
        int msg_num = std::stoi(items[2]);
        int num_sv = std::stoi(items[3]);
        int cnt = 0;
        if (num_msg > msg_num) {
            cnt = 4;
        }
        else {
            cnt = num_sv % 4;
        }

        sv_list.resize(cnt);
        for (int i = 0; i < cnt; i++) {
            std::string svid_str = items[4 + i * 4];
            std::string elv_str = items[5 + i * 4];
            std::string az_str = items[6 + i * 4];
            std::string cno_str = items[7 + i * 4];

            if (svid_str != "") {
                sv_list[i].svid = std::stoi(svid_str);
            }
            else {
                sv_list[i].svid = -1;
            }

            if (elv_str != "") {
                sv_list[i].elv = std::stoi(elv_str);
            }
            else {
                sv_list[i].elv = -1;
            }

            if (az_str != "") {
                sv_list[i].az = std::stoi(az_str);
            }
            else {
                sv_list[i].az = -1;
            }

            if (cno_str != "") {
                sv_list[i].cno = std::stoi(cno_str);
            }
            else {
                sv_list[i].cno = -1;
            }
            sv_list[i].sys = system_id;
        }
        signal_id = std::stoi(items[4 + cnt * 4]);
    }
    catch (std::exception ex) {
        std::cerr << "GSV exception: " << ex.what() << std::endl;
    }
}

/**
 * @brief systemId取得
 * 
 * @return int systemId
 */
int nmea_gsv::get_system_id()
{
    return system_id;
}

/**
 * @brief 衛星情報検索
 * 
 * @param svid 衛星ID
 * @return true 該当する衛星有り
 * @return false 該当する衛星無し
 */
bool nmea_gsv::find_svid(int svid)
{
    for (auto sv : sv_list) {
        if (sv.svid == svid) {
            return true;
        }
    }

    return false;
}

/**
 * @brief 衛星情報取得
 * 
 * @param svid 衛星ID
 * @return nmea_gsv::sv_info 衛星情報
 */
nmea_gsv::sv_info nmea_gsv::get_svinfo(int svid)
{
    sv_info info;
    for (auto sv : sv_list) {
        if (sv.svid == svid) {
            info = sv;
            break;
        }
    }

    return info;
}

std::vector<int> nmea_gsv::get_svid_list()
{
    std::vector<int> svid_list;
    for (auto sv : sv_list) {
        svid_list.push_back(sv.svid);
    }

    return svid_list;
}

