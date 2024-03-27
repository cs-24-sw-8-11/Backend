cmake_minimum_required(VERSION 3.15)
project(cs-24-sw-8-11-Backend)
set(CMAKE_CXX_STANDARD 20)

enable_testing()

include_directories(include)

add_subdirectory(src)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/SQLiteCpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/argparse)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/json)
