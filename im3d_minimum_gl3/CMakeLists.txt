SET(SUB_NAME im3d_minimum_gl3)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common/GL/glew.c
    #
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_gl3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gl3_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/orbit_camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/win32_window.cpp
    )
TARGET_COMPILE_DEFINITIONS(${SUB_NAME} PUBLIC
    GLEW_BUILD
    RENDERER_GLEW
    )
TARGET_INCLUDE_DIRECTORIES(${SUB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # GL/glew.h
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    OPENGL32
    winmm
    )
