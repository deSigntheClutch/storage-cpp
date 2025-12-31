cmake_minimum_required(VERSION 3.10)
project(RedisExamples CXX)

set(CMAKE_CXX_STANDARD 17)

# Common dependencies for all examples
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)
find_package(Boost REQUIRED COMPONENTS json)

# Add subdirectories
add_subdirectory(pub-sub)
add_subdirectory(sorted-set)
