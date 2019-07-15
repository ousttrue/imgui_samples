#include "gl3_renderer.h"
#include <array>
#include <sstream>
#include <fstream>

#include "gl_include.h"

#include "teapot.h"

const std::string g_glsl =
#include "../shaders/model.glsl"
    ;

///
/// GL3Renderer
///
GL3Renderer::GL3Renderer(const std::string &version)
    : m_version(version)
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

static GLuint CompileShader(const std::string &debugName, GLenum stage, const std::string &src)
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
        fprintf(stderr, "#########\n");
        fprintf(stderr, "Error compiling '%s':\n\n", debugName.c_str());
        GLint len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char *log = new GLchar[len];
        glGetShaderInfoLog(shader, len, 0, log);
        fprintf(stderr, log);
        delete[] log;

        fprintf(stderr, "\n\n%s", src.data());
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

unsigned int CreateShader(const std::string &debugName, const std::string &vsSrc, const std::string &fsSrc)
{
    auto vs = CompileShader(debugName + "@vs", GL_VERTEX_SHADER, vsSrc);
    if (!vs)
    {
        return 0;
    }
    auto fs = CompileShader(debugName + "@fs", GL_FRAGMENT_SHADER, fsSrc);
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
    static GLuint shTeapot = 0;
    static GLuint vbTeapot = 0;
    static GLuint ibTeapot = 0;
    static GLuint vaTeapot = 0;
    if (shTeapot == 0)
    {
        auto vs = GL3ShaderSource(g_glsl, m_version);
        vs.Define("VERTEX_SHADER");
        vs.Replace("noperspective", "");

        auto fs = GL3ShaderSource(g_glsl, m_version);
        fs.Define("FRAGMENT_SHADER");
        fs.Insert("precision mediump float;\n");
        fs.Replace("noperspective", "");

        shTeapot = CreateShader("model.glsl", vs.GetSource(), fs.GetSource());
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
