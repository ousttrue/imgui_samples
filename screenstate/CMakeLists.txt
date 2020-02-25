set(TARGET_NAME screenstate)
add_library(${TARGET_NAME}
    Win32Window.cpp
    orbit_camera.cpp
    )
set_property(TARGET ${TARGET_NAME} 
    PROPERTY CXX_STANDARD 20
    )
target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )
target_link_libraries(${TARGET_NAME} PUBLIC
    )
