set(TARGET_NAME glew)
add_library(${TARGET_NAME} SHARED glew-2.1.0/src/glew.c)
target_include_directories(${TARGET_NAME}
                           PUBLIC ${CMAKE_CURRENT_LIST_DIR}/glew-2.1.0/include)
target_compile_definitions(${TARGET_NAME} PRIVATE GLEW_BUILD)
target_link_libraries(${TARGET_NAME} PRIVATE OpenGL32.lib)
