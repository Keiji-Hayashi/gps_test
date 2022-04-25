/**
 * @file gps_test.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief gps test
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ubx.hpp"
#include "nmea_gga.hpp"
#include "nmea_rmc.hpp"
#include "nmea_gsa.hpp"
#include "nmea_gsv.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <limits>
#include <random>
#include <thread>
#include <signal.h>

static std::atomic<bool> terminate(false);

/**
 * @brief ランダムな時間ウェイト(500ms ～ 1s)
 * 
 */
static void random_sleep()
{
    const int min = 500;
    const int max = 1000;
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(min, max);
    int interval = distr(eng);
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

/**
 * @brief サムチェック
 * 
 * @param sentence NMEAのセンテンス
 * @return true OK
 * @return false ERROR
 */
bool check_sum(std::string sentence)
{
    // チェックサムの計算
    size_t start = sentence.find("$") + 1;
    size_t end = sentence.find("*");
    uint8_t sum = 0;
    for (auto it = sentence.begin() + start, e = sentence.begin() + end; it != e; ++it) {
        sum ^= *it;
    }

    // チェックサムを切り出し
    std::string sum_str = sentence.substr(end + 1, 2);
    if (sum == std::stoi(sum_str, nullptr, 16)) {
        return true;
    }

    return false;
}

/**
 * @brief 数値を出力（-1は表示しない）
 * 
 * @param value 
 * @return std::string 
 */
std::string to_str(int value)
{
    std::string str = "";
    if (value < 0) {
        str = "  ";
    }
    else {
        str = std::to_string(value);
    }

    return str;
}

/**
 * @brief [OK][ERROR]を出力
 * 
 * @param ok 
 */
inline void print_result(bool ok) 
{
    if (ok == true) {
        std::cout << "\033[32m[OK]\033[0m    ";
    }
    else {
        std::cout << "\033[31m[ERROR]\033[0m ";
    }
}

/**
 * @brief UTCを出力
 * 
 * @param gps_utc 
 */
void print_utc(std::string gps_utc)
{
    if (gps_utc == "") {
        std::cout << "Date Time (UTC) \033[31m[Error]\033[0m" << std::endl;
    }
    else {
        std::cout << "Date Time (UTC) " << gps_utc << std::endl;
    }
}

/**
 * @brief GPS座標を出力
 * 
 * @param latitude 
 * @param longitude 
 * @param altitude 
 * @param num_sv 
 */
void print_gps_coordinates(double latitude, double longitude, double altitude, int num_sv)
{
    std::cout << "Latitude = ";
    if (std::isnan(latitude)) {
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(11) << "[ERROR]";
        std::cout << "\033[0m";
    }
    else {
        std::cout << std::right << std::setw(11) << std::fixed << std::setprecision(7) << latitude;
    }
    std::cout << ", ";

    // 経度を表示
    std::cout << "Longitude = ";
    if (std::isnan(longitude)) {
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(11) << "[ERROR]";
        std::cout << "\033[0m";
    }
    else {
        std::cout << std::right << std::setw(11) << std::fixed << std::setprecision(7) << longitude;
    }
    std::cout << ", ";

    // 高度を表示
    std::cout << "Altitude = ";
    if (std::isnan(altitude)) {
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(8) << "[ERROR]";
        std::cout << "\033[0m";
    }
    else {
        std::cout << std::right << std::setw(8) << std::fixed << std::setprecision(1) << altitude;
    }
    std::cout << ", ";

    std::cout << "num_sv = ";
    if (num_sv < 0) {
        std::cout << "\033[31m[ERROR]\033[0m";
    }
    else {
        std::cout << num_sv;
    }


    std::cout << "\033[0K" << std::endl;
}

/**
 * @brief DOP（精度）を出力
 * 
 * @param pdop 
 * @param hdop 
 * @param vdop 
 */
void print_dop(double pdop, double hdop, double vdop)
{
    std::cout << "PDOP = ";
    if (std::isnan(pdop)) {
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(5) << "[ERROR]";
        std::cout << "\033[0m";

    }
    else {
        std::cout << std::right << std::setw(5) << std::fixed << std::setprecision(2) << pdop;
    }
    std::cout << ", ";

    std::cout << "HDOP = ";
    if (std::isnan(hdop)) {
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(5) << "[ERROR]";
        std::cout << "\033[0m";
    }
    else {
        std::cout << std::right << std::setw(5) << std::fixed << std::setprecision(2) << hdop;
    }
    std::cout << ", ";

    std::cout << "VDOP = ";
    if (std::isnan(vdop)){
        std::cout << "\033[31m";
        std::cout << std::left << std::setw(5) << "[ERROR]";
        std::cout << "\033[0m";
    }
    else {
        std::cout << std::right << std::setw(5) << std::fixed << std::setprecision(2) << vdop;
    }
    std::cout << "\033[0K" << std::endl;

}

/**
 * @brief 衛星情報を出力
 * 
 * @param gsa_list 
 * @param gsv_list 
 */
void print_satellite_info(std::vector<nmea_gsa> gsa_list, std::vector<nmea_gsv> gsv_list)
{
    int no = 0;
    std::cout << "No.\tActive\tSat.ID\tEL.\tAZ.\tC/N0\tSignal" << "\033[0K" << std::endl;
    for(auto gsv : gsv_list) {
        std::vector<int> svid_list = gsv.get_svid_list();
        std::string sys_str;
        switch (gsv.get_system_id()) {
        case 1:
            sys_str = "GPS";
            break;
        case 2:
            sys_str = "GLONASS";
            break;
        case 3:
            sys_str = "Galileo";
            break;
        case 4:
            sys_str = "BeiDou";
            break;
        default:
            sys_str = "";
            break;
        }
        for(auto svid : svid_list) {
            std::string active = "";
            for(auto gsa : gsa_list) {
                auto said_list = gsa.get_svid_list();
                for (auto said : said_list) {
                    if (said == svid) {
                        active = "Yes";
                    }
                }
            }
            nmea_gsv::sv_info si = gsv.get_svinfo(svid);
            std::cout << std::right << std::setw(2) << no << "\t";
            std::cout << active << "\t";
            std::cout << std::right << std::setw(2) << si.svid << "\t";
            std::cout << std::right << std::setw(2) << to_str(si.elv) << "\t";
            std::cout << std::right << std::setw(3) << to_str(si.az) << "\t";
            std::cout << std::right << std::setw(2) << to_str(si.cno) << "\t";
            std::cout << std::left << std::setw(8) << sys_str << "\033[0K" << std::endl;
            no++;
        }
    }
    std::cout << "\033[0J" << std::endl;
}

/**
 * @brief メインのループ処理
 * 
 */
static void loop_thread_proc()
{
    ubx ubx;
    bool sum_err = false;
    int sum_err_cnt = 0;
    bool utc_err = false;
    int utc_err_cnt = 0;
    bool timeout = false;
    int timeout_cnt = 0;
    std::chrono::system_clock::time_point  prev_time = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point  curr_time = std::chrono::system_clock::now();
    const double timeout_limit = 2000.0;
    std::string msg;
    std::vector<std::string> nmea;

    std::cout << "\033[2J" << std::endl;
    while(!terminate) {
        curr_time = std::chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - prev_time).count(); 
        if (elapsed >= timeout_limit && timeout == false) {
            timeout = true;
            timeout_cnt++;
        }

        std::vector<uint8_t> buf;
        ubx::status sts = ubx.get_nmea(buf);
        if (sts == ubx::conflict) {
            // コンフリクトした場合はランダムな時間ウェイト
            random_sleep();
            continue;
        };

        if(sts == ubx::ok) {
            msg.insert(msg.end(), buf.begin(), buf.end());
        }

        if(sts == ubx::empty && msg.size() > 0) {
            curr_time = std::chrono::system_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - prev_time).count(); 
            if (elapsed < timeout_limit) {
                timeout = false;
            }
            prev_time = curr_time;
            std::string gps_utc = "";
            double latitude = std::numeric_limits<double>::quiet_NaN();
            double longitude = std::numeric_limits<double>::quiet_NaN();
            double altitude = std::numeric_limits<double>::quiet_NaN();
            int num_sv = 0;
            time_t prev_gps_time_t = (time_t)-1;
            time_t current_gps_time_t = (time_t)-1;
            std::vector<nmea_gsa> gsa;
            std::vector<nmea_gsv> gsv;
            std::vector<nmea_gsv::sv_info> sv_list;

            nmea.clear();
            size_t pos;
            std::string delimiter = "\r\n";
            while ((pos = msg.find(delimiter)) != std::string::npos) {
                std::string sentence = msg.substr(0, pos);
                msg.erase(0, pos + delimiter.length());
                nmea.push_back(sentence);
            }
            msg = "";
            std::cout << "\033[0;0H";
            for (auto s : nmea) {
                std::cout << s;
                if (check_sum(s) == false) {
                    // チェックサムエラー
                    sum_err = true;
                    sum_err_cnt++;
                    print_result(false);
                    continue;
                }
                else {
                    print_result(true);
                }
                std::cout << "\033[0K" << std::endl;

                if (s.find("RMC") != std::string::npos) {
                    nmea_rmc rmc(s);
                    gps_utc = rmc.get_utc_datetime();
                    current_gps_time_t = rmc.get_time_t();
                }
                if (s.find("GGA") != std::string::npos) {
                    nmea_gga gga(s);
                    latitude = gga.get_latitude();
                    longitude = gga.get_longitude();
                    altitude = gga.get_altitude();
                    num_sv = gga.get_num_sv();
                }
                if (s.find("GSA") != std::string::npos) {
                    gsa.push_back(nmea_gsa(s));
                }
                if (s.find("GSV") != std::string::npos) {
                    gsv.push_back(nmea_gsv(s));
                }
            }

            // 時刻チェック
            if (current_gps_time_t <= 0) {
                // エラー
                utc_err = true;
                utc_err_cnt++;
            }
            else {
                if (prev_gps_time_t > 0) {
                    int diff = difftime(current_gps_time_t, prev_gps_time_t);
                    if (diff > 1) {
                        utc_err = true;
                        utc_err_cnt++;
                    }
                    else {
                        utc_err = false;
                    }
                }
                prev_gps_time_t = current_gps_time_t;
            }


            // 表示
            std::cout << "Checksum  ";
            print_result(!sum_err);
            std::cout << "(error count = " << sum_err_cnt << ")\033[0K" << std::endl;
            
            std::cout << "UTC check ";
            print_result(!utc_err);
            std::cout << "(error count = " << utc_err_cnt << ")\033[0K" << std::endl;

            std::cout << "Timeout   ";
            print_result(!timeout);
            std::cout << "(error count = " << timeout_cnt << ")\033[0K" << std::endl;

#if 1
            // 時刻を表示
            print_utc(gps_utc);

            // 緯度を表示
            print_gps_coordinates(latitude, longitude, altitude, num_sv);

            // DOP（精度）を表示
            double pdop = std::numeric_limits<double>::quiet_NaN();
            double hdop = std::numeric_limits<double>::quiet_NaN();
            double vdop = std::numeric_limits<double>::quiet_NaN();
            if(gsa.size() > 0) {
                pdop = gsa[0].get_pdop();
                hdop = gsa[0].get_hdop();
                vdop = gsa[0].get_vdop();
            }
            print_dop(pdop, hdop, vdop);

            // 使用衛星の情報表示
            print_satellite_info(gsa, gsv);
#endif
            std::cout << "\033[K";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "terminate" << std::endl;
}

/**
 * @brief メイン関数
 * 
 * @return int 
 */
int main(void) 
{
    sigset_t ss = {0};
    int signo = 0;

    // 無限ループを回避するためにメインのループを別スレッドにする。
    terminate.store(false);
    std::thread loop_thread([]{loop_thread_proc();}) ;

    // Ctrl+Cとkillを待つようにセット
    sigemptyset(&ss);
    if (sigaddset(&ss, SIGINT) != 0) {
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&ss, SIGKILL) != 0) {
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&ss, SIGTERM) != 0) {
        exit(EXIT_FAILURE);
    }
    if (sigaddset(&ss, SIGHUP) != 0) {
        exit(EXIT_FAILURE);
    }
    if (sigprocmask(SIG_BLOCK, &ss, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

    // シグナル待ち
    if (sigwait(&ss, &signo) == 0) {
        if (signo == SIGINT) {
        }
        else if (signo == SIGKILL) {
        }
        else if (signo == SIGTERM) {
        }
        else if (signo == SIGHUP) {
        }
    }

    // シグナルを受信したらループを終了
    terminate.store(true);
    loop_thread.join();

    return 0;
}


