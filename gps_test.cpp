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
#include <fstream>

static std::atomic<bool> terminate(false);
double MinimumLatitude = -90;
double MaximumLatitude = 90;
double MinimumLongitude = -180;
double MaximumLongitude = 180;
double MinimumAltitude = -1000;
double MaximumAltitude = 10000;
bool PositionError = false;
bool LatitudeError = false;
bool LongitudeError = false;
bool AltitudeError = false;
int PositionErrorCount = 0;
int LatitudeErrorCount = 0;
int LongitudeErrorCount = 0;
int AltitudeErrorCount = 0;

struct gps_test_param {
    bool print_nmea;
    bool print_sv;
    gps_test_param() {
        print_nmea = false;
        print_sv = false;
    }
};

void read_conf();

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
inline std::string print_result(bool ok) 
{
    if (ok == true) {
        return "\033[32m[OK]   \033[0m";
    }
    else {
        return "\033[31m[ERROR]\033[0m";
    }
}

/**
 * @brief UTCを出力
 * 
 * @param gps_utc 
 */
void print_utc(std::string gps_utc)
{
    std::cout << "\033[0K";
    if (gps_utc == "") {
        std::cout << "DateTime(UTC) \033[31m[ERROR]\033[0m";
    }
    else {
        std::cout << "DateTime(UTC) " << gps_utc;
    }
    std::cout << std::endl;
}

void position_check(double latitude, double longitude, double altitude)
{
    if (!std::isnan(latitude) && latitude >= MinimumLatitude && latitude <= MaximumLatitude) {
        LatitudeError = false;
    }
    else {
        LatitudeError = true;
        LatitudeErrorCount++;
    }

    if (!std::isnan(longitude) && longitude >= MinimumLongitude && longitude <= MaximumLongitude) {
        LongitudeError = false;
    }
    else {
        LongitudeError = true;
        LongitudeErrorCount++;
    }

    if (!std::isnan(altitude) && altitude >= MinimumAltitude && altitude <= MaximumAltitude) {
        AltitudeError = false;
    }
    else {
        AltitudeError = true;
        AltitudeErrorCount++;
    }

    if(LatitudeError == false && LongitudeError == false && AltitudeError == false) {
        PositionError = false;
    }
    else {
        PositionError = true;
        PositionErrorCount++;
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
void print_gps_coordinates(double latitude, double longitude, double altitude)
{
    std::cout << "\033[0K";
    std::cout << "LAT=";
    if (std::isnan(latitude)) {
        std::cout << "-----------";
    }
    else {
        std::cout << std::right << std::setw(11) << std::fixed << std::setprecision(7) << latitude;
    }
    std::cout << print_result(!LatitudeError) << ", ";

    // 経度を表示
    std::cout << "LON=";
    if (std::isnan(longitude)) {
        std::cout << "-----------";
    }
    else {
        std::cout << std::right << std::setw(11) << std::fixed << std::setprecision(7) << longitude;
    }
    std::cout << print_result(!LongitudeError) << ", ";

    // 高度を表示
    std::cout << "ALT=";
    if (std::isnan(altitude)) {
        std::cout << "------";
    }
    else {
        std::cout << std::setw(6) << std::fixed << std::setprecision(1) << altitude;
    }
    std::cout << print_result(!AltitudeError) << std::endl;
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
    std::cout << "\033[0K";
    std::cout << "PDOP=";
    if (std::isnan(pdop)) {
        std::cout << "\033[31m[ERROR]\033[0m";
    }
    else {
        std::cout << std::fixed << std::setprecision(2) << pdop;
    }
    std::cout << ", ";

    std::cout << "HDOP=";
    if (std::isnan(hdop)) {
        std::cout << "\033[31m[ERROR]\033[0m";
    }
    else {
        std::cout << std::fixed << std::setprecision(2) << hdop;
    }
    std::cout << ", ";

    std::cout << "VDOP=";
    if (std::isnan(vdop)){
        std::cout << "\033[31m[ERROR]\033[0m";
    }
    else {
        std::cout << std::fixed << std::setprecision(2) << vdop;
    }
    std::cout << std::endl;

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
    std::cout << "\033[0K";
    std::cout << "No.\tActive\tSat.ID\tEL.\tAZ.\tC/N0\tGNSS\t" << std::endl;
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
            std::cout << "\033[0K";
            std::cout << std::right << std::setw(2) << no << "\t";
            std::cout << active << "\t";
            std::cout << std::right << std::setw(2) << si.svid << "\t";
            std::cout << std::right << std::setw(2) << to_str(si.elv) << "\t";
            std::cout << std::right << std::setw(3) << to_str(si.az) << "\t";
            std::cout << std::right << std::setw(2) << to_str(si.cno) << "\t";
            std::cout << std::left << std::setw(8) << sys_str << std::endl;
            no++;
        }
    }
}

/**
 * @brief メインのループ処理
 * 
 */
static void loop_thread_proc(gps_test_param param)
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
    bool update = false;

    std::cout << "\033[2J" << std::endl;
    while(!terminate) {
        curr_time = std::chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - prev_time).count(); 
        if (elapsed >= timeout_limit && timeout == false) {
            timeout = true;
            timeout_cnt++;
            update = true;
            prev_time = curr_time;
        }
        else {
            timeout = false;
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
            prev_time = curr_time;

            nmea.clear();
            size_t pos;
            std::string delimiter = "\r\n";
            while ((pos = msg.find(delimiter)) != std::string::npos) {
                std::string sentence = msg.substr(0, pos);
                msg.erase(0, pos + delimiter.length());
                nmea.push_back(sentence);
            }
            msg = "";
            update = true;
        }
        if (update == true) {
            update = false;
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
            std::cout << "\033[0;0H";
            for (auto s : nmea) {
                std::stringstream ss;
                ss << s << " ";
                if (check_sum(s) == false) {
                    // チェックサムエラー
                    sum_err = true;
                    sum_err_cnt++;
                    ss << print_result(false);
                    continue;
                }
                else {
                    ss << print_result(true);
                }
                ss << "\033[0K" << std::endl;
                // NMEA表示
                if (param.print_nmea) {
                    std::cout << ss.str();
                }

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

            // GPS座標をチェック
            position_check(latitude, longitude, altitude);

            // 表示
            std::cout << "Checksum  " << print_result(!sum_err);
            std::cout << "(error count = " << sum_err_cnt << ")\033[0K" << std::endl;
            
            std::cout << "UTC check " << print_result(!utc_err);
            std::cout << "(error count = " << utc_err_cnt << ")\033[0K" << std::endl;

            std::cout << "Position  " << print_result(!PositionError);
            std::cout << "(error count = " << PositionErrorCount << ")\033[0K" << std::endl;

            std::cout << "Timeout   " << print_result(!timeout);
            std::cout << "(error count = " << timeout_cnt << ")\033[0K" << std::endl;


            // 時刻を表示
            print_utc(gps_utc);

            // 緯度を表示
            print_gps_coordinates(latitude, longitude, altitude);

            // DOP（精度）を表示
            double pdop = std::numeric_limits<double>::quiet_NaN();
            double hdop = std::numeric_limits<double>::quiet_NaN();
            double vdop = std::numeric_limits<double>::quiet_NaN();
            if(gsa.size() > 0) {
                pdop = gsa[0].get_pdop();
                hdop = gsa[0].get_hdop();
                vdop = gsa[0].get_vdop();
            }
            std::cout << "num_sv=" << num_sv << ", ";

            print_dop(pdop, hdop, vdop);

            if (param.print_sv) {
                // 衛星の情報表示
                print_satellite_info(gsa, gsv);
            }
            std::cout << "\033[0J";

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
int main(int argc, char *argv[]) 
{
    sigset_t ss = {0};
    int signo = 0;
    gps_test_param param;
    
    for (int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if (argv[i][1] == 'n') {
                param.print_nmea = true;
            }
            if (argv[i][1] == 's') {
                param.print_sv = true;
            }
        }
    }

    read_conf();

    // 無限ループを回避するためにメインのループを別スレッドにする。
    terminate.store(false);
    std::thread loop_thread([param]{loop_thread_proc(param);}) ;

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

/**
 * @brief 空白文字削除
 * 
 * @param str 文字列
 * @param trim_character_list 削除対象の空白文字
 * @return std::string 結果
 */
inline std::string trim(const std::string& str, const char* trim_character_list=" \t\v\r\n")
{
    std::string result = "";
    // 左から最初の空白文字以外の文字を検索
    auto left = str.find_first_not_of(trim_character_list);
    if (left != std::string::npos) {
        // すべて空白文字でなければ右から最後の空白文字以外の文字を検索 
        auto right = str.find_last_not_of(trim_character_list);
        // 左右の空白文字を除いた文字列を切り出す
        result = str.substr(left, right - left + 1);
    }
    return result;
}

/**
 * @brief 設定ファイル読み込み
 * 
 */
void read_conf()
{
    std::ifstream ifs;
    std::string filename = "gps_test.conf";
    ifs.open(filename, std::ios::in);
    std::string line;
    while (std::getline(ifs, line)) {
        // #以降はコメントなので削除
        auto comment_pos = line.find("#");
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        // '='があればKeyとValueへ分割
        auto pos = line.find("=");
        if (pos != std::string::npos) {
            auto key = trim(line.substr(0, pos));
            auto value = trim(line.substr(pos + 1));
            if (key == "MinimumLatitude") {
                MinimumLatitude = std::stod(value);
            }
            else if (key == "MaximumLatitude") {
                MaximumLatitude = std::stod(value);
            }
            else if (key == "MinimumLongitude") {
                MinimumLongitude = std::stod(value);
            }
            else if (key == "MaximumLongitude") {
                MaximumLongitude = std::stod(value);
            }
            else if (key == "MinimumAltitude") {
                MinimumAltitude = std::stod(value);
            }
            else if (key == "MaximumAltitude") {
                MaximumAltitude = std::stod(value);
            }
        }
    }

}
