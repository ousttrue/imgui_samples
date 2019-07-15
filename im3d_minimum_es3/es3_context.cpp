#include "es3_context.h"
#include <EGL/egl.h>
#include <assert.h>

class ES3ContextImpl
{
    EGLContext m_context = nullptr;
    EGLDisplay m_display = nullptr;
    EGLSurface m_surface = nullptr;

public:
    ~ES3ContextImpl()
    {
        if (m_context != EGL_NO_CONTEXT)
        {
            eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(m_display, m_context);
            m_context = EGL_NO_CONTEXT;
        }

        if (m_surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(m_display, m_surface);
            m_surface = EGL_NO_SURFACE;
        }

        if (m_display != EGL_NO_DISPLAY)
        {
            eglTerminate(m_display);
            m_display = EGL_NO_DISPLAY;
        }
    }

    bool Create(HWND hwnd)
    {
        // m_display
        m_display = eglGetDisplay(GetDC(hwnd));
        if (!m_display)
        {
            return false;
        }

        EGLint major, minor;
        assert(eglInitialize(m_display, &major, &minor));

        assert(eglBindAPI(EGL_OPENGL_ES_API));

        // config
        EGLint configAttributes[] =
            {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_BUFFER_SIZE, 32,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                EGL_NONE};
        EGLConfig config;
        EGLint num_config;
        assert(eglChooseConfig(m_display, configAttributes, &config, 1, &num_config));

        // context
        EGLint contextAttributes[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE};
        EGLContext context = eglCreateContext(m_display, config, NULL, contextAttributes);
        assert(context);

        // surface
        m_surface = eglCreateWindowSurface(m_display, config, hwnd, NULL);
        assert(m_surface);

        assert(eglMakeCurrent(m_display, m_surface, m_surface, context));
        return true;
    }

    void Present()
    {
        assert(eglSwapBuffers(m_display, m_surface));
    }
};

ES3Context::ES3Context()
    : m_impl(new ES3ContextImpl)
{
}

ES3Context::~ES3Context()
{
    delete m_impl;
}

bool ES3Context::Create(void *hwnd)
{
    return m_impl->Create((HWND)hwnd);
}

void ES3Context::Present()
{
    m_impl->Present();
}
