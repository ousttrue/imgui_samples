set(TARGET_NAME common)
add_library(${TARGET_NAME} im3d_impl.cpp shader_source.cpp)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${TARGET_NAME} PUBLIC screenstate im3d)
