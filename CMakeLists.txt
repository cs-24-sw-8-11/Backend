cmake_minimum_required(VERSION 3.15)
project(cs-24-sw-8-11-Backend)
set(CMAKE_CXX_STANDARD 23)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

enable_testing()

include_directories(include include/Routes ./lib/asio/asio/include)

add_subdirectory(src)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/SQLiteCpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/argparse)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/json)

set(ASIO_INCLUDE_DIR ./lib/asio/asio/include)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/Crow)