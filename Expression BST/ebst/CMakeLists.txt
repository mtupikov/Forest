add_library(ebst STATIC
	#EBST.h
	Operand.h
	Operator.h
	Operator.cpp
)

target_link_libraries(ebst INTERFACE abst)

target_include_directories(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
