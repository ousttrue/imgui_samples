set(TARGET_NAME im3d)
add_library(${TARGET_NAME} im3d/im3d.cpp)
target_include_directories(
  ${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/im3d
                        ${CMAKE_CURRENT_LIST_DIR}/im3d/examples/common)
target_compile_definitions(${TARGET_NAME} PUBLIC IM3D_VERTEX_ALIGNMENT=16)
