set(TARGET_NAME im3d_minimum_dx11)
add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE plog common common_dx11)
