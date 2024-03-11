cmake_minimum_required(VERSION 3.10)
project(main)
find_package(SEAL)

add_executable(tecmp_pdte tecmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(tecmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tecmp_pdte SEAL::seal)

add_executable(tecmp_base_pdte tecmp_base_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(tecmp_base_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tecmp_base_pdte SEAL::seal)

add_executable(lrcmp_pdte lrcmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(lrcmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lrcmp_pdte SEAL::seal)

add_executable(rrcmp_pdte rrcmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp)
target_include_directories(rrcmp_pdte PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rrcmp_pdte SEAL::seal)

add_executable(new_tree new_tree.cpp src/utils.cpp src/cmp.cpp src/node.cpp)
target_include_directories(new_tree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(new_tree SEAL::seal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")