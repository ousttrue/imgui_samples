#include "im3d_gui.h"
#include <im3d.h>
#include <im3d_math.h>
#include <GL/glew.h>
#include "gl3_renderer.h"
#include "camera_state.h"
#include "mouse_state.h"

const std::string g_points_vs =
#include "im3d_points.vs"
    ;
const std::string g_points_fs =
#include "im3d_points.fs"
    ;

const std::string g_lines_vs =
#include "im3d_lines.vs"
    ;
const std::string g_lines_fs =
#include "im3d_lines.fs"
    ;

const std::string g_triangles_vs =
#include "im3d_triangles.vs"
    ;
const std::string g_triangles_fs =
#include "im3d_triangles.fs"
    ;

class Im3dGuiImpl
{
    GLuint g_Im3dVertexArray;
    GLuint g_Im3dVertexBuffer;
    GLuint g_Im3dUniformBuffer;
    GLuint g_Im3dShaderPoints;
    GLuint g_Im3dShaderLines;
    GLuint g_Im3dShaderTriangles;

public:
    Im3dGuiImpl()
    {
        static_assert(sizeof(Im3d::VertexData) % 16 == 0);

        {
            g_Im3dShaderPoints = CreateShader(g_points_vs, g_points_fs);
            auto blockIndex = glGetUniformBlockIndex(g_Im3dShaderPoints, "VertexDataBlock");
            glUniformBlockBinding(g_Im3dShaderPoints, blockIndex, 0);
        }
        {
            g_Im3dShaderLines = CreateShader(g_lines_vs, g_lines_fs);
            auto blockIndex = glGetUniformBlockIndex(g_Im3dShaderLines, "VertexDataBlock");
            glUniformBlockBinding(g_Im3dShaderLines, blockIndex, 0);
        }
        {
            g_Im3dShaderTriangles = CreateShader(g_triangles_vs, g_triangles_fs);
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
        glCreateBuffers(1, &g_Im3dVertexBuffer);
        ;
        glCreateVertexArrays(1, &g_Im3dVertexArray);
        glBindVertexArray(g_Im3dVertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, g_Im3dVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), (GLvoid *)vertexData, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Im3d::Vec4), (GLvoid *)0);
        glBindVertexArray(0);

        glCreateBuffers(1, &g_Im3dUniformBuffer);
    }

    ~Im3dGuiImpl()
    {
        glDeleteVertexArrays(1, &g_Im3dVertexArray);
        glDeleteBuffers(1, &g_Im3dUniformBuffer);
        glDeleteBuffers(1, &g_Im3dVertexBuffer);
        glDeleteProgram(g_Im3dShaderPoints);
        glDeleteProgram(g_Im3dShaderLines);
        glDeleteProgram(g_Im3dShaderTriangles);
    }

    void Draw(const float *viewProj, int w, int h)
    {
        glViewport(0, 0, (GLsizei)w, (GLsizei)h);
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
            glUniform2f(glGetUniformLocation(sh, "uViewport"), ad.m_viewportSize.x, ad.m_viewportSize.y);
            glUniformMatrix4fv(glGetUniformLocation(sh, "uViewProjMatrix"), 1, false, viewProj);

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
    }
};

Im3dGui::Im3dGui()
    : m_impl(new Im3dGuiImpl)
{
}

Im3dGui::~Im3dGui()
{
    delete m_impl;
}

void Im3dGui::NewFrame(const camera::CameraState *c, const MouseState *mouse, float deltaTime)
{
    auto &ad = Im3d::GetAppData();

    ad.m_deltaTime = deltaTime;
    ad.m_viewportSize = Im3d::Vec2(c->viewportWidth, c->viewportHeight);

    auto &inv = c->viewInverse;
    ad.m_viewOrigin = Im3d::Vec3(inv[12], inv[13], inv[14]); // for VR use the head position
    ad.m_viewDirection = Im3d::Vec3(-inv[8], -inv[9], -inv[10]);
    ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
    ad.m_projOrtho = false;

    // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
    ad.m_projScaleY = tanf(c->fovYRadians * 0.5f) * 2.0f;

    // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
    Im3d::Vec2 cursorPos((float)mouse->X, (float)mouse->Y);
    cursorPos = (cursorPos / ad.m_viewportSize) * 2.0f - 1.0f;
    cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
    Im3d::Vec3 rayOrigin, rayDirection;
    {
        rayOrigin = ad.m_viewOrigin;
        rayDirection.x = cursorPos.x / c->projection[0];
        rayDirection.y = cursorPos.y / c->projection[5];
        rayDirection.z = -1.0f;
        Im3d::Mat4 camWorld(
            c->viewInverse[0], c->viewInverse[4], c->viewInverse[8], c->viewInverse[12],
            c->viewInverse[1], c->viewInverse[5], c->viewInverse[9], c->viewInverse[13],
            c->viewInverse[2], c->viewInverse[6], c->viewInverse[10], c->viewInverse[14],
            c->viewInverse[3], c->viewInverse[7], c->viewInverse[11], c->viewInverse[15]);
        rayDirection = camWorld * Im3d::Vec4(Im3d::Normalize(rayDirection), 0.0f);
    }
    ad.m_cursorRayOrigin = rayOrigin;
    ad.m_cursorRayDirection = rayDirection;

    // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via im3d_config.h, or if any of the IsVisible() functions are called.
    Im3d::Mat4 viewProj(
        c->viewProjection[0], c->viewProjection[1], c->viewProjection[2], c->viewProjection[3],
        c->viewProjection[4], c->viewProjection[5], c->viewProjection[6], c->viewProjection[7],
        c->viewProjection[8], c->viewProjection[9], c->viewProjection[10], c->viewProjection[11],
        c->viewProjection[12], c->viewProjection[13], c->viewProjection[14], c->viewProjection[15]);
    ad.setCullFrustum(viewProj, true);

    // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
    // All key states have an equivalent (and more descriptive) 'Action_' enum.
    //ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Mouse_Left /*Im3d::Action_Select*/] = mouse->IsDown(ButtonFlags::Left);

#if 0
    // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl key as an additional predicate.
    bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_L /*Action_GizmoLocal*/] = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_T /*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_R /*Action_GizmoRotation*/] = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
    ad.m_keyDown[Im3d::Key_S /*Action_GizmoScale*/] = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

    // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
    ad.m_snapTranslation = 0.0f;
    ad.m_snapRotation = 0.0f;
    ad.m_snapScale = 0.0f;
#endif

    Im3d::NewFrame();
}

void Im3dGui::Manipulate(float world[16])
{
    Im3d::Gizmo("GizmoUnified", world);
}

void Im3dGui::Draw(const float *viewProjection, int w, int h)
{
    Im3d::EndFrame();

    m_impl->Draw(viewProjection, w, h);

    glDisable(GL_BLEND);
}
