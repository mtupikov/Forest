add_library(ebst STATIC
	EBST.h
	EBST.cpp
	ExpressionNode.h
	ExpressionNode.cpp
)

target_link_libraries(ebst PUBLIC abst)

target_include_directories(ebst INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
