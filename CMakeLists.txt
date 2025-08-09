cmake_minimum_required(VERSION 3.28)
project(Crawler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(crawler
    src/main.cpp
)

target_include_directories(crawler
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
