/**
 * @file nmea_rmc.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief RMC
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "nmea_rmc.hpp"
#include <array>
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <numeric>
#include <climits>

/**
 * @brief 西暦1年からのうるう年をの回数を取得
 * 
 * @param y 
 * @return int 
 */
static inline int cnt_leap(int y)
{
    return ((int)((y) / 4) - (int)((y) / 100) + (int)((y) / 400));        
}

/**
 * @brief Construct a new nmea rmc::nmea rmc object
 * 
 * @param nmea RMC
 */
nmea_rmc::nmea_rmc(std::string nmea) :
 gps_time((time_t)-1)
{
    try {
        std::vector<std::string> items;
        size_t pos;
        while ((pos = nmea.find_first_of(",*")) != std::string::npos) {
            items.push_back(nmea.substr(0, pos));
            nmea.erase(0, pos + 1);
        }
        items.push_back(nmea);
        date = items[9];
        time = items[1];

        if (date.size() >= 6 && time.size() >= 6) {
            int day = std::stoi(date.substr(0,2));
            int mon = std::stoi(date.substr(2,2));
            int year = std::stoi("20" + date.substr(4,2));

            int hour = std::stoi(time.substr(0,2));
            int min = std::stoi(time.substr(2,2));
            int sec = std::stoi(time.substr(4,2));

            // time_t（エポック秒）を計算
            static const int dom[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            int days = 365 * (year - 1970) + cnt_leap(year - 1) - cnt_leap(1970 - 1);
            days += std::accumulate(&dom[0], &dom[mon - 1], 0);
            days += day - 1;
            if (mon > 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
                ++days;
            }
            gps_time = days * 86400 + hour * 3600 + min * 60 + sec;
        }
        else {
            gps_time = (time_t)-1;
        } 
    }
    catch (std::exception ex) {
        std::cerr << "RMC exception: " << ex.what() << std::endl;
        gps_time = (time_t)-1;
    }
}

/**
 * @brief 日時取得
 * 
 * @return std::string 日時（local）
 */
std::string nmea_rmc::get_local_datetime()
{
    std::array<char,100> buf;
    std::string str;
    if(gps_time > 0) {
        std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&gps_time));
        str = buf.data();
    }
    else {
        str = "";
    }
    return str;
}

/**
 * @brief 
 * 
 * @return std::string 
 */
std::string nmea_rmc::get_utc_datetime()
{
    std::array<char,100> buf;
    std::string str;
    if(gps_time > 0) {
        std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M:%S", std::gmtime(&gps_time));
        str = buf.data();
    }
    else {
        str = "";
    }
    return str;
}

time_t nmea_rmc::get_time_t()
{
    return gps_time;
}

std::string nmea_rmc::get_time()
{
    return time;
}