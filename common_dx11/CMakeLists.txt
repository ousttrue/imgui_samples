set(TARGET_NAME common_dx11)
add_library(${TARGET_NAME} dx11_context.cpp dx11_renderer.cpp
                           im3d_impl_dx11.cpp)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE plog
  PUBLIC im3d d3d11.lib d3dcompiler.lib)
