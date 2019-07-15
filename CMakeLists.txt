CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(imgui_samples VERSION 0.1.0)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/im3d
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/im3d/examples/common
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/json/single_include
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/plog/include
    ${CMAKE_CURRENT_LIST_DIR}/common
    )
LINK_DIRECTORIES(
    )
ADD_DEFINITIONS(
    -DUNICODE
    -D_UNICODE
    -DNOMINMAX
    -DIM3D_VERTEX_ALIGNMENT=16
    -DGLEW_BUILD
    )
ADD_COMPILE_OPTIONS(
    /std:c++latest 
    /EHsc
    )

SUBDIRS(
    common
    glew
    im3d_minimum
    )
