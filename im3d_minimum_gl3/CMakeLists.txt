SET(SUB_NAME im3d_minimum_gl3)

FILE(GLOB SRC
    *.cpp
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    ${CMAKE_CURRENT_LIST_DIR}/../dependencies/im3d/im3d.cpp
    )
TARGET_COMPILE_DEFINITIONS(${SUB_NAME} PUBLIC
    )
TARGET_INCLUDE_DIRECTORIES(${SUB_NAME} PUBLIC
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    common
    OPENGL32
    glew
    winmm
    )
