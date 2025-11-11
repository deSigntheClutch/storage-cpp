cmake_minimum_required(VERSION 3.10)
project(RedisDemo CXX)

set(CMAKE_CXX_STANDARD 17)

find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)

# Find Boost
find_package(Boost REQUIRED COMPONENTS json)

add_executable(redis_demo main.cpp)
target_include_directories(redis_demo PRIVATE ${HIREDIS_INCLUDE_DIR})
target_link_libraries(redis_demo PRIVATE ${HIREDIS_LIB} Boost::json)
