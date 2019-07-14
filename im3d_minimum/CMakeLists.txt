SET(SUB_NAME im3d_minimum)

FILE(GLOB SRC
        *.cpp
        # GL/glew.c
    ) 
ADD_EXECUTABLE(${SUB_NAME}
    ${SRC}
    )
TARGET_LINK_LIBRARIES(${SUB_NAME}
    # OPENGL32
    )
