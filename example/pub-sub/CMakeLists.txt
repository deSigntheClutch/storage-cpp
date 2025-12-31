# Dependencies are inherited from parent CMakeLists.txt

# Publisher executable
add_executable(publisher publisher.cpp)
target_include_directories(publisher PRIVATE ${HIREDIS_INCLUDE_DIR})
target_link_libraries(publisher PRIVATE ${HIREDIS_LIB} Boost::json)

# Subscriber executable
add_executable(subscriber subscriber.cpp)
target_include_directories(subscriber PRIVATE ${HIREDIS_INCLUDE_DIR})
target_link_libraries(subscriber PRIVATE ${HIREDIS_LIB} Boost::json)
