SET(SUB_NAME im3d_in_imgui_view_dx11)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/../common/orbit_camera.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/win32_window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/shader_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl_dx11.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/dx11_context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/dx11_renderer.cpp
    )
TARGET_COMPILE_DEFINITIONS(${SUB_NAME} PUBLIC
    )
TARGET_INCLUDE_DIRECTORIES(${SUB_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # teapot.h
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    winmm
    d3d11
    d3dcompiler
    )
