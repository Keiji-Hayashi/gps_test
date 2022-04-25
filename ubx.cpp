/**
 * @file ubx.cpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief UBX
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "ubx.hpp"
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <array>
#include <sstream>
#include <iomanip>

/**
 * @brief Construct a new ubx::ubx object
 * 
 */
ubx::ubx()
{

}

/**
 * @brief I2Cスレーブデバイスからデータを読み込む.
 * 
 * @param dev_addr デバイスアドレス
 * @param reg_addr レジスタアドレス
 * @param data 読み込むデータの格納場所を指すポインタ
 * @param length 読み込むデータの長さ
 * @return int8_t 
 */
int8_t ubx::i2c_read(int32_t fd, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t length)
{
    /* I2C-Readメッセージを作成する. */
    struct i2c_msg messages[] = {
        { dev_addr, 0, 1, &reg_addr },         /* レジスタアドレスをセット. */
        { dev_addr, I2C_M_RD, length, data },  /* dataにlengthバイト読み込む. */
    };
    struct i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

    /* I2C-Readを行う. */
    if (ioctl(fd, I2C_RDWR, &ioctl_data) != 2) {
        std::cerr << "i2c_read: failed to ioctl: " << std::strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

/**
 * @brief I2Cスレーブデバイスにデータを書き込む
 * 
 * @param dev_addr デバイスアドレス
 * @param reg_addr レジスタアドレス
 * @param data 書き込むデータの格納場所を指すポインタ
 * @param length 書き込むデータの長さ
 * @return int8_t 
 */
int8_t ubx::i2c_write(int32_t fd, uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length)
{
    /* I2C-Write用のバッファを準備する. */
    std::vector<uint8_t> buffer(static_cast<size_t>(length + 1));
    buffer[0] = reg_addr;              /* 1バイト目にレジスタアドレスをセット. */
    for (int i = 0; i < length; i++) {
        buffer[i + 1] = data[i];
    }
    
    /* I2C-Writeメッセージを作成する. */
    struct i2c_msg message = { dev_addr, 0, ((ushort)(length + 1)), buffer.data() };
    struct i2c_rdwr_ioctl_data ioctl_data = { &message, 1 };

    /* I2C-Writeを行う. */
    if (ioctl(fd, I2C_RDWR, &ioctl_data) != 1) {
        std::cerr << "i2c_write: failed to ioctl: " << std::strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

/**
 * @brief Get the nmea object
 * 
 * @param nmea NMEA文字列配列
 * @return true データ有効
 * @return false データ無効（コンフリクト発生）
 */
ubx::status ubx::get_nmea(std::vector<uint8_t> &buf)
{
#if 0
    std::string msg;
    static time_t pt = time(nullptr);
    time_t ct = time(nullptr);
    if (ct == pt) {
        return ubx::empty;
    }
    pt = ct;
    struct tm *ptm = gmtime(&ct);
    std::array<char, 200> date;
    std::array<char, 200> time;
    strftime(date.data(), date.size(), "%d%m%y", ptm);
    strftime(time.data(), time.size(), "%H%M%S.00", ptm);
    std::stringstream ss;
    ss << "$GNRMC," << std::string(time.data()) << ",A,3540.23799,N,13922.23373,E,0.407,," << std::string(date.data()) << ",,,A,V*";
    uint8_t sum = 0;
    for (auto c : ss.str()){
        if (c == '$' || c == '*') {
            continue;
        }
        sum ^= static_cast<uint8_t>(c);
    }
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sum) << "\r\n";
    std::string s = ss.str();
    msg += s;
    // msg += "$GNRMC,085505.00,A,3540.23799,N,13922.23373,E,0.407,,110422,,,A,V*17\r\n";
    msg += "$GNVTG,,T,,M,0.407,N,0.754,K,A*38\r\n";
    msg += "$GNGGA,085505.00,3540.23799,N,13922.23373,E,1,10,0.99,148.0,M,38.9,M,,*48\r\n";
    msg += "$GNGSA,A,3,19,04,03,17,14,01,06,,,,,,1.79,0.99,1.49,1*09\r\n";
    msg += "$GNGSA,A,3,71,88,,,,,,,,,,,1.79,0.99,1.49,2*07\r\n";
    msg += "$GNGSA,A,3,33,,,,,,,,,,,,1.79,0.99,1.49,3*00\r\n";
    msg += "$GNGSA,A,3,,,,,,,,,,,,,1.79,0.99,1.49,4*07\r\n";
    msg += "$GPGSV,3,1,10,01,27,064,23,03,56,049,25,04,27,118,24,06,33,284,26,1*69\r\n";
    msg += "$GPGSV,3,2,10,09,18,154,,14,40,213,24,17,69,334,22,19,48,320,27,1*6E\r\n";
    msg += "$GPGSV,3,3,10,21,07,079,,28,,,25,1*52\r\n";
    msg += "$GLGSV,3,1,11,65,17,182,,70,04,024,,71,45,054,26,72,53,139,,1*7B\r\n";
    msg += "$GLGSV,3,2,11,76,06,213,17,77,22,258,,78,15,315,,85,03,098,,1*74\r\n";
    msg += "$GLGSV,3,3,11,86,39,068,,87,44,336,22,88,11,302,20,1*48\r\n";
    msg += "$GAGSV,1,1,01,33,63,351,25,7*47\r\n";
    msg += "$GBGSV,1,1,00,1*76\r\n";
    msg += "$GNGLL,3540.23799,N,13922.23373,E,085505.00,A,A*73\r\n";

    buf = std::vector<uint8_t>(msg.begin(), msg.end());
    return ubx::ok;

#else
    const char *dev_name = "/dev/i2c-1";
    const uint8_t dev_addr = 0x42u;
    const uint8_t size_reg = 0xfdu;
    const uint8_t stream_reg = 0xffu;
    bool conflict = false;

    // OPEN
    int32_t fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        std::cerr << "get_nmea: failed to open: " << std::strerror(errno) << std::endl;
        return ubx::dev_error;
    }

    try {
        // データサイズ取得
        std::vector<uint8_t> lenbuf(2, 0);
        int8_t ret = i2c_read(fd, dev_addr, size_reg, lenbuf.data(), lenbuf.size());
        if (ret != 0) {
            return ubx::dev_error;
        }
        int len = ((int)lenbuf[0] << 8) | ((int)lenbuf[1] << 0);

        if (len > 0) {
            // データ取得
            buf.resize(len);
            ret = i2c_read(fd, dev_addr, stream_reg, buf.data(), buf.size());

            for (auto c : buf) {
                if (c == 0xff) {
                    // コンフリクト
                    conflict = true;
                    break;
                }
            }
        }
    }
    catch (std::exception ex) {
        // 例外発生
        std::cerr << "get_nmea: exception: " << ex.what() << std::endl;
    }

    // CLOSE
    if (close(fd) < 0) {
        std::cerr << "get_nmea: failed to close: " << std::strerror(errno) << std::endl;
    }

    if (conflict == true) {

        return ubx::conflict;
    }
    if (buf.size() == 0) {
        return ubx::empty;
    }

    return ubx::ok;
#endif
}
