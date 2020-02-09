cmake_minimum_required(VERSION 3.12)

project(rbst_test LANGUAGES CXX)

set(application_source)
set(application_headers)

#add_subdirectory(src)

#add_library(libSource STATIC ${application_headers} ${application_source})

#target_include_directories(libSource PUBLIC
#	"${CMAKE_CURRENT_SOURCE_DIR}/src"
#)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} rbst)

#target_link_libraries(${PROJECT_NAME} libSource rbst)
