/**
 * @file ubx.hpp
 * @author Keiji Hayashi (keiji.hayashi@konicaminolta.com)
 * @brief UBX
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef UBX_HPP
#define UBX_HPP

#include <vector>
#include <string>

/**
 * @brief UBX
 * 
 */
class ubx
{
public:
    ubx();
    enum status {
        empty,
        conflict,
        dev_error,
        ok
    };
    status get_nmea(std::vector<uint8_t> &buf);

private:
    int8_t i2c_read(int32_t fd, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint16_t length);
    int8_t i2c_write(int32_t fd, uint8_t dev_addr, uint8_t reg_addr, const uint8_t* data, uint16_t length);
};


#endif