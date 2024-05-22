#!/bin/bash
g++ -g -o  ./linux/client.out -I/usr/include/SDL2 imgui/*.cpp PigeonClient/PigeonClientGUIInfo.h PigeonClient/*.cpp PigeonClient/*.h Utils/Utils.h PigeonClient/PigeonPacket.h *.cpp -lssl -ljsoncpp -lcrypto -lSDL2 -lGLEW -lGL -ldl -std=c++17
