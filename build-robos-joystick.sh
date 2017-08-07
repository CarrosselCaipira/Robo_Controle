g++ -c joystick.cc
g++ -c robo.cpp
g++ -c radio.cpp
g++ -o Robos_Joystick robos_joystick.cpp joystick.o robo.o radio.o 