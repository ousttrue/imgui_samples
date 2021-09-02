set(TARGET_NAME json)
add_library(${TARGET_NAME} INTERFACE)
target_link_directories(${TARGET_NAME} INTERFACE
                        ${CMAKE_CURRENT_LIST_DIR}/json/include)
