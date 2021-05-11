#include "im3d_impl_gl3.h"
#include <im3d.h>
#include <im3d_math.h>
#include <plog/Log.h>

#include "gl_include.h"
#include "camera_state.h"
#include "screenstate.h"
#include "gl3_renderer.h"
#include "shader_source.h"

const std::string g_glsl =
#include "../shaders/im3d.glsl"
    ;

class Im3dImplGL3Impl
{
    GLuint g_Im3dVertexArray;
    GLuint g_Im3dVertexBuffer;
    GLuint g_Im3dUniformBuffer;
    GLuint g_Im3dShaderPoints;
    GLuint g_Im3dShaderLines;
    GLuint g_Im3dShaderTriangles;

public:
    Im3dImplGL3Impl(const std::string &version)
    {
        static_assert(sizeof(Im3d::VertexData) % 16 == 0);

        // glGetv
        //

        GLint value;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
        LOGI << "GL_MAX_UNIFORM_BLOCK_SIZE: " << value;

        {
            auto vs = ShaderSource(g_glsl, version);
            vs.Define("VERTEX_SHADER");
            vs.Define("POINTS");
            if (version == "#version 300 es")
            {
                vs.Replace("noperspective", "");
            }

            auto fs = ShaderSource(g_glsl, version);
            fs.Define("FRAGMENT_SHADER");
            fs.Define("POINTS");
            if (version == "#version 300 es")
            {
                fs.Insert("precision mediump float;\n");
                fs.Replace("noperspective", "");
            }
            g_Im3dShaderPoints = CreateShader("im3d_point", vs.GetSource(), fs.GetSource());
            auto blockIndex = glGetUniformBlockIndex(g_Im3dShaderPoints, "VertexDataBlock");
            glUniformBlockBinding(g_Im3dShaderPoints, blockIndex, 0);
        }
        {
            auto vs = ShaderSource(g_glsl, version);
            vs.Define("VERTEX_SHADER");
            vs.Define("LINES");
            if (version == "#version 300 es")
            {
                vs.Replace("noperspective", "");
            }

            auto fs = ShaderSource(g_glsl, version);
            fs.Define("FRAGMENT_SHADER");
            fs.Define("LINES");
            if (version == "#version 300 es")
            {
                fs.Insert("precision mediump float;\n");
                fs.Replace("noperspective", "");
            }
            g_Im3dShaderLines = CreateShader("im3d_line", vs.GetSource(), fs.GetSource());
            auto blockIndex = glGetUniformBlockIndex(g_Im3dShaderLines, "VertexDataBlock");
            glUniformBlockBinding(g_Im3dShaderLines, blockIndex, 0);
        }
        {
            auto vs = ShaderSource(g_glsl, version);
            vs.Define("VERTEX_SHADER");
            vs.Define("TRIANGLES");
            if (version == "#version 300 es")
            {
                vs.Replace("noperspective", "");
            }

            auto fs = ShaderSource(g_glsl, version);
            fs.Define("FRAGMENT_SHADER");
            fs.Define("TRIANGLES");
            if (version == "#version 300 es")
            {
                fs.Insert("precision mediump float;\n");
                fs.Replace("noperspective", "");
            }

            g_Im3dShaderTriangles = CreateShader("im3d_triangle", vs.GetSource(), fs.GetSource());
            auto blockIndex = glGetUniformBlockIndex(g_Im3dShaderTriangles, "VertexDataBlock");
            glUniformBlockBinding(g_Im3dShaderTriangles, blockIndex, 0);
        }

        // in this example we're using a static buffer as the vertex source with a uniform buffer to provide
        // the shader with the Im3d vertex data
        Im3d::Vec4 vertexData[] = {
            Im3d::Vec4(-1.0f, -1.0f, 0.0f, 1.0f),
            Im3d::Vec4(1.0f, -1.0f, 0.0f, 1.0f),
            Im3d::Vec4(-1.0f, 1.0f, 0.0f, 1.0f),
            Im3d::Vec4(1.0f, 1.0f, 0.0f, 1.0f)};
        glGenBuffers(1, &g_Im3dVertexBuffer);
        ;
        glGenVertexArrays(1, &g_Im3dVertexArray);
        glBindVertexArray(g_Im3dVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), (GLvoid *)vertexData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec4), (GLvoid *)0);
        // glVertexAttribDivisor(0, 0);
        glBindVertexArray(0);

        glGenBuffers(1, &g_Im3dUniformBuffer);
    }

    ~Im3dImplGL3Impl()
    {
        glDeleteVertexArrays(1, &g_Im3dVertexArray);
        glDeleteBuffers(1, &g_Im3dUniformBuffer);
        glDeleteBuffers(1, &g_Im3dVertexBuffer);
        glDeleteProgram(g_Im3dShaderPoints);
        glDeleteProgram(g_Im3dShaderLines);
        glDeleteProgram(g_Im3dShaderTriangles);
    }

    void Draw(const float *viewProj)
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (Im3d::U32 i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
        {
            auto &drawList = Im3d::GetDrawLists()[i];

            if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
            {
                // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
            }

            GLenum prim;
            GLuint sh;
            int primVertexCount;
            switch (drawList.m_primType)
            {
            case Im3d::DrawPrimitive_Points:
                prim = GL_TRIANGLE_STRIP;
                primVertexCount = 1;
                sh = g_Im3dShaderPoints;
                glDisable(GL_CULL_FACE); // points are view-aligned
                break;
            case Im3d::DrawPrimitive_Lines:
                prim = GL_TRIANGLE_STRIP;
                primVertexCount = 2;
                sh = g_Im3dShaderLines;
                glDisable(GL_CULL_FACE); // lines are view-aligned
                break;
            case Im3d::DrawPrimitive_Triangles:
                prim = GL_TRIANGLES;
                primVertexCount = 3;
                sh = g_Im3dShaderTriangles;
                //glEnable(GL_CULL_FACE); // culling valid for triangles, but optional
                break;
            default:
                IM3D_ASSERT(false);
                return;
            };

            glBindVertexArray(g_Im3dVertexArray);
            glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer);

            glUseProgram(sh);
            auto &ad = Im3d::GetAppData();
            auto loc = glGetUniformLocation(sh, "uViewport");
            glUniform2f(loc, ad.m_viewportSize.x, ad.m_viewportSize.y);
            loc = glGetUniformLocation(sh, "uViewProjMatrix");
            glUniformMatrix4fv(loc, 1, false, viewProj);

            // Uniform buffers have a size limit; split the vertex data into several passes.
            const int kMaxBufferSize = 64 * 1024; // assuming 64kb here but the application should check the implementation limit
            const int kPrimsPerPass = kMaxBufferSize / (sizeof(Im3d::VertexData) * primVertexCount);

            int remainingPrimCount = drawList.m_vertexCount / primVertexCount;
            const Im3d::VertexData *vertexData = drawList.m_vertexData;
            while (remainingPrimCount > 0)
            {
                int passPrimCount = remainingPrimCount < kPrimsPerPass ? remainingPrimCount : kPrimsPerPass;
                int passVertexCount = passPrimCount * primVertexCount;

                glBindBuffer(GL_UNIFORM_BUFFER, g_Im3dUniformBuffer);
                glBufferData(GL_UNIFORM_BUFFER, (GLsizeiptr)passVertexCount * sizeof(Im3d::VertexData), (GLvoid *)vertexData, GL_DYNAMIC_DRAW);

                // instanced draw call, 1 instance per prim
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_Im3dUniformBuffer);
                glDrawArraysInstanced(prim, 0, prim == GL_TRIANGLES ? 3 : 4, passPrimCount); // for triangles just use the first 3 verts of the strip

                vertexData += passVertexCount;
                remainingPrimCount -= passPrimCount;
            }
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glDisable(GL_BLEND);
    }
};

//////////////////////////////////////////////////////////////////////////////
Im3dImplGL3::Im3dImplGL3(const std::string &version)
    : m_impl(new Im3dImplGL3Impl(version))
{
}

Im3dImplGL3::~Im3dImplGL3()
{
    delete m_impl;
}

void Im3dImplGL3::Draw(const float *viewProjection)
{
    m_impl->Draw(viewProjection);
}
