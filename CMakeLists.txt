CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(imgui_samples VERSION 0.1.0)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/im3d
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/im3d/examples/common
    )
LINK_DIRECTORIES(
    )
ADD_DEFINITIONS(
    -DNOMINMAX
    )
ADD_COMPILE_OPTIONS(
    /std:c++latest 
    /EHsc
    )

SUBDIRS(
    glew
    im3d
    im3d_minimum
    )
