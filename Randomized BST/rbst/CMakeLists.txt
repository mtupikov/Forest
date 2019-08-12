add_library(rbst INTERFACE)

target_sources(rbst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/RBST.h)

target_include_directories(rbst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})