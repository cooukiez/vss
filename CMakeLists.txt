cmake_minimum_required(VERSION 3.8)

project(vss)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test src/test.cpp)

find_package(glm REQUIRED)
target_link_libraries(test glm::glm)