class DX11RendererImpl;
class DX11Renderer
{
    DX11RendererImpl *m_impl = nullptr;

public:
    DX11Renderer();
    ~DX11Renderer();
    void NewFrame(int screenWidth, int screenHeight);
    void DrawTeapot(const float *viewProjection, const float *world);
};
