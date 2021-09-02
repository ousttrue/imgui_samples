set(TARGET_NAME im3d)
add_library(${TARGET_NAME} im3d.cpp im3d_types.cpp im3d_context.cpp)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
