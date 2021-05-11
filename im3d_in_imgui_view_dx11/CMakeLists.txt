SET(TARGET_NAME im3d_in_imgui_view_dx11)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${TARGET_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/examples/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/imgui/examples/imgui_impl_dx11.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/../common/shader_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/im3d_impl_dx11.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/dx11_context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/dx11_renderer.cpp
    )
TARGET_COMPILE_DEFINITIONS(${TARGET_NAME} PUBLIC
    )
TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # teapot.h
    ${CMAKE_CURRENT_LIST_DIR}/../common_dx11
    )
TARGET_LINK_LIBRARIES(${TARGET_NAME}
    winmm
    d3d11
    d3dcompiler
    screenstate
    )
set_property(TARGET ${TARGET_NAME} 
    PROPERTY CXX_STANDARD 20
    )
