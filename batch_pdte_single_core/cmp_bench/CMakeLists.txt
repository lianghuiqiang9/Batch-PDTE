cmake_minimum_required(VERSION 3.10)
project(cmp)
find_package(SEAL)

add_executable(tecmp tecmp.cpp ../src/cmp.cpp)
target_include_directories(tecmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tecmp SEAL::seal)

add_executable(tecmp_base tecmp_base.cpp ../src/cmp.cpp)
target_include_directories(tecmp_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tecmp_base SEAL::seal)

add_executable(lrcmp lrcmp.cpp ../src/cmp.cpp)
target_include_directories(lrcmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(lrcmp SEAL::seal)

add_executable(rrcmp rrcmp.cpp ../src/cmp.cpp)
target_include_directories(rrcmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(rrcmp SEAL::seal)

add_executable(time_test time_test.cpp)
target_link_libraries(time_test SEAL::seal)

add_executable(depth_test depth_test.cpp)
target_link_libraries(depth_test SEAL::seal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")