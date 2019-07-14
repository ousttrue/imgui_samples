CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(imgui_samples VERSION 0.1.0)

INCLUDE_DIRECTORIES(
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
    im3d_minimum
    )