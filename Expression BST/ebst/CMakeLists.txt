add_library(ebst STATIC
	#EBST.h
	ExpressionNode.h
	ExpressionNode.cpp
)

target_link_libraries(ebst INTERFACE abst)

target_include_directories(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
