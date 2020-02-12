cmake_minimum_required(VERSION 3.12)

project(ebst_test LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ebst)

#target_link_libraries(${PROJECT_NAME} libSource ebst)
