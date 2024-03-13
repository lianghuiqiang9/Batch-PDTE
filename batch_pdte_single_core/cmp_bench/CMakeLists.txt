cmake_minimum_required(VERSION 3.10)
project(cmp)
find_package(SEAL)

add_executable(tecmp tecmp.cpp ../src/cmp.cpp)
target_include_directories(tecmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tecmp SEAL::seal)

add_executable(tecmp_norm tecmp_norm.cpp ../src/cmp.cpp)
target_include_directories(tecmp_norm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tecmp_norm SEAL::seal)

add_executable(rdcmp rdcmp.cpp ../src/cmp.cpp)
target_include_directories(rdcmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(rdcmp SEAL::seal)

add_executable(cdcmp cdcmp.cpp ../src/cmp.cpp)
target_include_directories(cdcmp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(cdcmp SEAL::seal)

add_executable(time_test time_test.cpp)
target_link_libraries(time_test SEAL::seal)

add_executable(depth_test depth_test.cpp)
target_link_libraries(depth_test SEAL::seal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")