SET(SUB_NAME im3d_minimum)

FILE(GLOB SRC
        *.cpp
        # GL/glew.c
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    )
TARGET_COMPILE_DEFINITIONS(${SUB_NAME} PUBLIC
    UNICODE
    _UNICODE
    )
TARGET_INCLUDE_DIRECTORIES(${SUB_NAME} PUBLIC
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    # OPENGL32
    winmm
    )
