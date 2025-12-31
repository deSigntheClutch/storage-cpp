# Dependencies are inherited from parent CMakeLists.txt

add_executable(sorted-set main.cpp)
target_include_directories(sorted-set PRIVATE ${HIREDIS_INCLUDE_DIR})
target_link_libraries(sorted-set PRIVATE ${HIREDIS_LIB} Boost::json)
