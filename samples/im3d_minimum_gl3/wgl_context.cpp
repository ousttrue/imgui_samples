#include "wgl_context.h"
#include <GL/glew.h>
#include <GL/wglew.h>
#include <stdio.h>
#include <assert.h>

static const char *GlGetString(GLenum _name)
{
    const char *ret;
    assert(ret = (const char *)glGetString(_name));
    return ret ? ret : "";
}

class WGLContextImpl
{
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;

public:
    ~WGLContextImpl()
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(m_hglrc);
        ReleaseDC(m_hwnd, m_hdc);
    }

    bool Initialize(HWND hwnd, int major, int minor)
    {
        m_hwnd = hwnd;
        m_hdc = GetDC(hwnd);

        // set the window pixel format
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 24;
        pfd.dwDamageMask = 8;
        int pformat = 0;
        pformat = ChoosePixelFormat(m_hdc, &pfd);
        SetPixelFormat(m_hdc, pformat, &pfd);

        static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;
        {
            // create dummy context to load wgl extensions
            HGLRC hglrc = 0;
            hglrc = wglCreateContext(m_hdc);
            wglMakeCurrent(m_hdc, hglrc);

            // check the platform supports the requested GL version
            GLint platformVMaj, platformVMin;
            glGetIntegerv(GL_MAJOR_VERSION, &platformVMaj);
            glGetIntegerv(GL_MINOR_VERSION, &platformVMin);
            major = major < 0 ? platformVMaj : major;
            minor = minor < 0 ? platformVMin : minor;
            if (platformVMaj < major || (platformVMaj >= major && platformVMin < minor))
            {
                fprintf(stderr, "OpenGL version %d.%d is not available (available version is %d.%d).", major, minor, platformVMaj, platformVMin);
                fprintf(stderr, "This error may occur if the platform has an integrated GPU.");
                return false;
            }

            // load wgl extensions for true context creation
            wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

            // delete the dummy context
            wglMakeCurrent(0, 0);
            wglDeleteContext(hglrc);
        }

        // create true context
        int profileBit = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        //profileBit = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        int attr[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            WGL_CONTEXT_PROFILE_MASK_ARB, profileBit,
            0};

        m_hglrc = wglCreateContextAttribs(m_hdc, 0, attr);

        // load extensions
        if (!wglMakeCurrent(m_hdc, m_hglrc))
        {
            fprintf(stderr, "wglMakeCurrent failed");
            return false;
        }
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        assert(err == GLEW_OK);
        glGetError(); // clear any errors caused by glewInit()

        wglSwapIntervalEXT(0); // example uses FPS as a rough perf measure, hence disable vsync

        fprintf(stdout, "OpenGL context:\n\tVersion: %s\n\tGLSL Version: %s\n\tVendor: %s\n\tRenderer: %s\n",
                GlGetString(GL_VERSION),
                GlGetString(GL_SHADING_LANGUAGE_VERSION),
                GlGetString(GL_VENDOR),
                GlGetString(GL_RENDERER));

        if (major == 3 && minor == 1)
        {
            // check that the uniform blocks size is at least 64kb
            GLint maxUniformBlockSize;
            glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
            if (maxUniformBlockSize < (64 * 1024))
            {
                fprintf(stderr, "GL_MAX_UNIFORM_BLOCK_SIZE is less than 64kb (%dkb)", maxUniformBlockSize / 1024);
                return false;
            }
        }

        return true;
    }

    void Present()
    {
        SwapBuffers(m_hdc);
    }
};

WGLContext::WGLContext()
    : m_impl(new WGLContextImpl)
{
}

WGLContext::~WGLContext()
{
    delete m_impl;
}

bool WGLContext::Create(void* hwnd, int major, int minor)
{
    m_hwnd = hwnd;
    return m_impl->Initialize((HWND)hwnd, major, minor);
}

void WGLContext::Present()
{
    ValidateRect((HWND)m_hwnd, 0); // suppress WM_PAINT
    m_impl->Present();
}
