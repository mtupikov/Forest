add_library(ebst INTERFACE)

target_sources(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/EBST.h)

target_link_libraries(ebst INTERFACE abst)

target_include_directories(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
