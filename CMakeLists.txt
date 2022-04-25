cmake_minimum_required(VERSION 3.13)

project(gps_test CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS " -Wall -Wextra ")

find_package(Threads)

add_executable(gps_test
    gps_test.cpp
    ubx.cpp 
    nmea_gga.cpp
    nmea_gsa.cpp
    nmea_gsv.cpp
    nmea_rmc.cpp
)

target_link_libraries(gps_test
    Threads::Threads
)

add_executable(gps_test_org
    gps_test_org.cpp
    ubx.cpp
)

target_link_libraries(gps_test_org
    Threads::Threads
)

