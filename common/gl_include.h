#pragma once

#if defined(RENDERER_GLEW)
#include <GL/glew.h>
#elif defined(RENDERER_ANGLE)
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#else
#error "no renderer specified"
#endif
