set(TARGET_NAME im3d_minimum_gl3)
add_executable(${TARGET_NAME} main.cpp wgl_context.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE common common_gl OPENGL32.lib glew)
