cmake_minimum_required(VERSION 3.10.2)
project(Robo_Controle)

set(CMAKE_CXX_STANDARD 11)

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

target_link_libraries(Robo_Controle
        pthread)
