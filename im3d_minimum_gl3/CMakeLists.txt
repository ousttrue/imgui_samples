SET(TARGET_NAME im3d_minimum_gl3)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${TARGET_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common/GL/glew.c
    #
    ${CMAKE_CURRENT_LIST_DIR}/../common/shader_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common_gl/gl3_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common_gl/im3d_impl_gl3.cpp
    )
TARGET_COMPILE_DEFINITIONS(${TARGET_NAME} PUBLIC
    GLEW_BUILD
    RENDERER_GLEW
    )
TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # GL/glew.h
    ${CMAKE_CURRENT_LIST_DIR}/../common_gl
    )
TARGET_LINK_LIBRARIES(${TARGET_NAME}
    OPENGL32
    winmm
    screenstate
    )
set_property(TARGET ${TARGET_NAME} 
    PROPERTY CXX_STANDARD 20
    )
