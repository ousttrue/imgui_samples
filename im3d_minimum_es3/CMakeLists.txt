SET(SUB_NAME im3d_minimum_es3)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/../common/shader_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/gl3_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/orbit_camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/win32_window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl_gl3.cpp
    )
TARGET_COMPILE_DEFINITIONS(${SUB_NAME} PUBLIC
    RENDERER_ANGLE
    )
TARGET_INCLUDE_DIRECTORIES(${SUB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # teapot.h
    $ENV{VCPKG_DIR}/installed/x64-windows/include # EGL
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    $ENV{VCPKG_DIR}/installed/x64-windows/lib/libegl.lib # EGL
    $ENV{VCPKG_DIR}/installed/x64-windows/lib/libGLESv2.lib # EGL
    winmm
    )
