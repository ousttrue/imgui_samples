#include "gl3_renderer.h"
#include <array>
#include <string>

#include "gl_include.h"

#include "teapot.h"

const std::string g_vs =
#include "../shaders/model.vs"
    ;
const std::string g_fs =
#include "../shaders/model.fs"
    ;

GL3Renderer::GL3Renderer()
{
}

GL3Renderer::~GL3Renderer()
{
}

void GL3Renderer::NewFrame(int screenWidth, int screenHeight)
{
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screenWidth, screenHeight);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

static GLuint CompileShader(GLenum stage, const std::string &src)
{
    auto shader = glCreateShader(stage);
    auto data = static_cast<const GLchar *>(src.data());
    auto size = static_cast<GLint>(src.size());
    glShaderSource(shader, 1, &data, &size);

    glCompileShader(shader);
    auto compileStatus = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        // fprintf(stderr, "Error compiling '%s':\n\n", _path);
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char *log = new GLchar[len];
        glGetShaderInfoLog(shader, len, 0, log);
        fprintf(stderr, log);
        delete[] log;

        //fprintf(stderr, "\n\n%s", src.data());
        fprintf(stderr, "\n");
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static bool LinkShaderProgram(GLuint _handle)
{
    glLinkProgram(_handle);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(_handle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        fprintf(stderr, "Error linking program:\n\n");
        GLint len;
        glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &len);
        GLchar *log = new GLchar[len];
        glGetProgramInfoLog(_handle, len, 0, log);
        fprintf(stderr, log);
        fprintf(stderr, "\n");
        delete[] log;

        return false;
    }
    return true;
}

unsigned int CreateShader(const std::string &vsSrc, const std::string &fsSrc)
{
    auto vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
    if (!vs)
    {
        return 0;
    }
    auto fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!fs)
    {
        return 0;
    }

    auto shTeapot = glCreateProgram();
    glAttachShader(shTeapot, vs);
    glAttachShader(shTeapot, fs);
    bool ret = LinkShaderProgram(shTeapot);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ret)
    {
        return 0;
    }
    return shTeapot;
}

void GL3Renderer::DrawTeapot(const float *viewProjection, const float *world)
{
    static GLuint shTeapot;
    static GLuint vbTeapot;
    static GLuint ibTeapot;
    static GLuint vaTeapot;
    if (shTeapot == 0)
    {
        shTeapot = CreateShader(g_vs, g_fs);
        glGenBuffers(1, &vbTeapot);
        glGenBuffers(1, &ibTeapot);
        glGenVertexArrays(1, &vaTeapot);
        glBindVertexArray(vaTeapot);
        glBindBuffer(GL_ARRAY_BUFFER, vbTeapot);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3 * 2, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (GLvoid *)(sizeof(float) * 3));
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_teapotVertices), (GLvoid *)s_teapotVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibTeapot);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_teapotIndices), (GLvoid *)s_teapotIndices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
    glUseProgram(shTeapot);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uWorldMatrix"), 1, false, world);
    glUniformMatrix4fv(glGetUniformLocation(shTeapot, "uViewProjMatrix"), 1, false, viewProjection);
    glBindVertexArray(vaTeapot);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDrawElements(GL_TRIANGLES, sizeof(s_teapotIndices) / sizeof(unsigned), GL_UNSIGNED_INT, (GLvoid *)0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    glUseProgram(0);
}
