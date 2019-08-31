add_library(abst INTERFACE)

target_sources(abst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/AbstractBST.h)

target_include_directories(abst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})