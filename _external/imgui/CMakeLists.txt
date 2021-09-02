set(TARGET_NAME imgui)
add_library(
  ${TARGET_NAME}
  imgui/imgui.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/backends/imgui_impl_win32.cpp
  imgui/backends/imgui_impl_dx11.cpp
  imgui/imgui_demo.cpp)
target_include_directories(
  ${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui
                        ${CMAKE_CURRENT_LIST_DIR}/imgui/backends)
