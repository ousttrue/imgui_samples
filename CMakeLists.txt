CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(imgui_samples VERSION 0.1.0)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/im3d
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/examples
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/json/single_include
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/plog/include
    ${CMAKE_CURRENT_LIST_DIR}/common
    )
LINK_DIRECTORIES(
    )
ADD_DEFINITIONS(
    -DUNICODE
    -D_UNICODE
    -D_CRT_SECURE_NO_WARNINGS
    -DNOMINMAX
    -DIM3D_VERTEX_ALIGNMENT=16
    )

SUBDIRS(
    screenstate
    im3d
    im3d_minimum_gl3
    # im3d_minimum_es3
    im3d_minimum_dx11
    im3d_in_imgui_view_dx11
    # nodeeditor_dx11
    )
