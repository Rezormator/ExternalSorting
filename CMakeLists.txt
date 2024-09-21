cmake_minimum_required(VERSION 3.28)
project(externalSorting)

set(CMAKE_CXX_STANDARD 20)

add_executable(externalSorting main.cpp
        ExternalSorting.h
        ExternalSorting.cpp
        Generator.h
        Generator.cpp
        FilesActions.h
        FilesActions.cpp
        Input.cpp
        Input.h
)
