cmake_minimum_required(VERSION 3.16)
project(NepConvert CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(nepconvert
    src/main.cpp
    src/save_convert.cpp
)
