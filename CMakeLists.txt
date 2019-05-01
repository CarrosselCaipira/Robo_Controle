cmake_minimum_required(VERSION 3.10.2)
project(Robo_Controle)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Core REQUIRED)

include_directories(.)

add_executable(Robo_Controle
    joystick.cc
    joystick.hh
    radio.cpp
    radio.hpp
    robo.cpp
    robo.hpp
    robos_joystick.cpp
    tipoEstruturas.hpp)

qt5_use_modules(Robo_Controle Core SerialPort)

target_link_libraries(Robo_Controle
        pthread)
