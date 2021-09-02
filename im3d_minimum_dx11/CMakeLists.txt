set(TARGET_NAME im3d_minimum_dx11)

file(GLOB SRC *.cpp)
add_executable(
  ${TARGET_NAME}
  ${SRC}
  #
  ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../common/shader_source.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../common/im3d_impl.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/im3d_impl_dx11.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/dx11_context.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../common_dx11/dx11_renderer.cpp)
target_compile_definitions(${TARGET_NAME} PRIVATE)
target_include_directories(
  ${TARGET_NAME}
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/examples/common # teapot.h
    ${CMAKE_CURRENT_LIST_DIR}/../common_dx11)
target_link_libraries(
  ${TARGET_NAME} screenstate im3d
  #
  winmm d3d11 d3dcompiler)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
