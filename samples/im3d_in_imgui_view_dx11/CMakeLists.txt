set(TARGET_NAME im3d_in_imgui_view_dx11)
add_executable(${TARGET_NAME} main.cpp dx11_view.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE plog screenstate common common_dx11 imgui)
