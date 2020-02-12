add_library(ebst INTERFACE
	#EBST.h
	#Operand.h
	#Operator.h
	#Operator.cpp
)

target_sources(rbst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Operand.h)

target_link_libraries(ebst INTERFACE abst)

target_include_directories(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
