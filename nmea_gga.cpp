/**
 * @file nmea_gga.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief GGA
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "nmea_gga.hpp"
#include <iostream>
#include <vector>
#include <cmath>

/**
 * @brief Construct a new nmea gga::nmea gga object
 * 
 * @param nmea GAA
 */
nmea_gga::nmea_gga(std::string nmea) : 
latitude(std::nan("")),
longitude(std::nan("")),
altitude(std::nan("")),
num_sv(0),
time("")
{
    try {
        std::vector<std::string> items;
        size_t pos;
        while ((pos = nmea.find_first_of(",*")) != std::string::npos) {
            items.push_back(nmea.substr(0, pos));
            nmea.erase(0, pos + 1);
        }
        items.push_back(nmea);

        // 緯度
        std::string latitude_str = items[2];
        if (latitude_str != "") {
            double latitude_deg = std::stod(latitude_str.substr(0,2));
            double latitude_minute = std::stod(latitude_str.substr(2,8));
            if (items[3] == "N") {
                // 北緯は正
                latitude = latitude_deg + (latitude_minute / 60.0);
            } 
            else {
                // 南緯は負
                latitude = -1 * (latitude_deg + (latitude_minute / 60.0)); 
            }
        }
        else {
            latitude = std::nan("");
        }

        // 経度
        std::string longitude_str = items[4];
        if (longitude_str != "") {
            double longitude_deg = std::stod(longitude_str.substr(0,3));
            double longitude_minute = std::stod(longitude_str.substr(3,8));
            if (items[5] == "E") {
                // 東経は正
                longitude = longitude_deg + (longitude_minute/60.0);
            }
            else {
                // 西経は負
                longitude = -1 * (longitude_deg + (longitude_minute/60.0));
            }
        }
        else {
            longitude = std::nan("");
        }

        // 海抜（標高）
        std::string altitude_str = items[9];
        if (altitude_str != "") {
            altitude = std::stod(altitude_str);
        }
        else {
            altitude = std::nan("");
        }

        // 使用した衛星の数
        std::string nmu_sv_str = items[7];
        if (nmu_sv_str != "") {
            num_sv = std::stod(nmu_sv_str);
        }
        else {
            num_sv = 0;
        }

        time = items[1];
    }
    catch (std::exception ex) {
        std::cerr << "GAA exception: " << ex.what() << std::endl;
        latitude = std::nan("");
        longitude = std::nan("");
        altitude = std::nan("");
        num_sv = 0;
    }
}

/**
 * @brief 緯度取得
 * 
 * @return double 緯度（度）
 */
double nmea_gga::get_latitude()
{
    return latitude;
}

/**
 * @brief 経度取得
 * 
 * @return double 経度（度）
 */
double nmea_gga::get_longitude()
{
    return longitude;
}

/**
 * @brief 高度取得
 * 
 * @return double 高度（m）
 */
double nmea_gga::get_altitude()
{
    return altitude;
}

/**
 * @brief 使用した衛星の数取得
 * 
 * @return int 使用した衛星の数
 */
int nmea_gga::get_num_sv()
{
    return num_sv;
}

std::string nmea_gga::get_time()
{
    return time;
}