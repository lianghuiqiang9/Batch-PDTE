cmake_minimum_required(VERSION 3.10)
project(main)
find_package(SEAL)

add_executable(tecmp_pdte tecmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(tecmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tecmp_pdte SEAL::seal)

add_executable(tecmp_norm_pdte tecmp_norm_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(tecmp_norm_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tecmp_norm_pdte SEAL::seal)

add_executable(rdcmp_pdte rdcmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(rdcmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rdcmp_pdte SEAL::seal)

add_executable(cdcmp_pdte cdcmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(cdcmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cdcmp_pdte SEAL::seal)

add_executable(new_tree new_tree.cpp src/utils.cpp src/cmp.cpp src/node.cpp)
target_include_directories(new_tree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(new_tree SEAL::seal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")