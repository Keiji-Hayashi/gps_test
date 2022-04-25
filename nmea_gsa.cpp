/**
 * @file nmea_gsa.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GSA
 * @version 0.1
 * @date 2022-04-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "nmea_gsa.hpp"
#include <iostream>

/**
 * @brief Construct a new nmea gsa::nmea gsa object
 * 
 * @param nmea 
 */
nmea_gsa::nmea_gsa(std::string nmea) :
pdop(99.99),
hdop(99.99),
vdop(99.99),
system_id(-1)
{
    // Structure
    //      $xxGSA,opMode,navMode{,svid},PDOP,HDOP,VDOP,systemId*cs\r\n
    // Example
    //      $GPGSA,A,3,23,29,07,08,09,18,26,28,,,,,1.94,1.18,1.54,1*0D\r\n
    try {
        std::vector<std::string> items;
        size_t pos;
        while ((pos = nmea.find_first_of(",*")) != std::string::npos) {
            items.push_back(nmea.substr(0, pos));
            nmea.erase(0, pos + 1);
        }
        items.push_back(nmea);
        if (items.size() == 20) {
            for (int i = 0; i < 12; i++) {
                std::string svid_str = items[3 + i];
                if (svid_str != "") {
                    svid.push_back(std::stoi(items[3 + i]));
                }
            }

            std::string pdop_str = items[15];
            pdop = std::stod(pdop_str);
            hdop = std::stod(items[16]);
            vdop = std::stod(items[17]);
            system_id = std::stoi(items[18]);
        }

    }
    catch (std::exception ex) {
        std::cerr << "GSA exception: " << ex.what() << std::endl;
    }
    
}

/**
 * @brief 
 * 
 * @return int 
 */
int nmea_gsa::get_system_id()
{
    return system_id;
}

/**
 * @brief 
 * 
 * @return std::vector<int> 
 */
std::vector<int> nmea_gsa::get_svid_list()
{
    return svid;
}

double nmea_gsa::get_pdop()
{
    return pdop;
}

double nmea_gsa::get_hdop()
{
    return hdop;
}

double nmea_gsa::get_vdop()
{
    return vdop;
}