#!/usr/bin/env bash
g++ -c joystick.cc -std=c++11
g++ -c robo.cpp -std=c++11
g++ -c radio.cpp -std=c++11
g++ -o Robos_Joystick robos_joystick.cpp joystick.o robo.o radio.o -std=c++11
