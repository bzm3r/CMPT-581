cmake_minimum_required(VERSION 3.17)
project(asn1_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(q1 q1.cpp)
add_executable(q2 q2.cpp)
add_executable(q3 q3.cpp)
add_executable(q4 q4.cpp)

add_subdirectory(date)

target_include_directories(q2 INTERFACE
        "${PROJECT_SOURCE_DIR}/date"
        )
