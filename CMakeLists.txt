cmake_minimum_required(VERSION 2.8)
project(project)


include_directories(.)

add_executable(project
        8080emu.cpp
        8080emuCPP.h
        gtuos.cpp
        gtuos.h
        main.cpp
        memory.cpp
        memory.h
        memoryBase.h)
