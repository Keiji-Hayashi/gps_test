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
#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <limits>
#include <random>
#include <thread>
#include <signal.h>

static std::atomic<bool> terminate(false);  //! スレッド終了フラグ

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
 * @brief スレッド処理
 * 
 */
static void loop_thread_proc()
{
    ubx ubx;

    std::string msg;
    while (!terminate.load()) {
        std::vector<uint8_t> buf;
        std::vector<std::string> nmea;
        ubx::status sts = ubx.get_nmea(buf);
        if(sts == ubx::conflict) {
            random_sleep();
            continue;
        }
        if(sts == ubx::ok) {
            msg.insert(msg.end(), buf.begin(), buf.end());
        }

        // OKならNMEAを表示
        if(sts == ubx::empty && msg.size() > 0) {
            nmea.clear();
            size_t pos;
            std::string delimiter = "\r\n";
            while ((pos = msg.find(delimiter)) != std::string::npos) {
                std::string sentence = msg.substr(0, pos);
                msg.erase(0, pos + delimiter.length());
                nmea.push_back(sentence);
            }
            msg = "";
            for (auto s : nmea) {
                std::cout << s << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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

    // 無限ループを回避するためにメインのループを別スレッドで動かす。
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

    // シグナルを受信したらスレッドを終了
    terminate.store(true);
    loop_thread.join();

    return 0;
}


