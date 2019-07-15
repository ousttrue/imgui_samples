#include "dx11_renderer.h"
#include <d3d11.h>

class DX11RendererImpl
{
    struct D3DShader
    {
        ID3DBlob *m_vsBlob = nullptr;
        ID3D11VertexShader *m_vs = nullptr;
        ID3DBlob *m_gsBlob = nullptr;
        ID3D11GeometryShader *m_gs = nullptr;
        ID3DBlob *m_psBlob = nullptr;
        ID3D11PixelShader *m_ps = nullptr;

        void Release()
        {
            if (m_vsBlob)
                m_vsBlob->Release();
            if (m_vs)
                m_vs->Release();
            if (m_gsBlob)
                m_gsBlob->Release();
            if (m_gs)
                m_gs->Release();
            if (m_psBlob)
                m_psBlob->Release();
            if (m_ps)
                m_ps->Release();
        }
    };

    D3DShader g_Im3dShaderPoints;
    D3DShader g_Im3dShaderLines;
    D3DShader g_Im3dShaderTriangles;
    ID3D11InputLayout *g_Im3dInputLayout = nullptr;
    ID3D11RasterizerState *g_Im3dRasterizerState = nullptr;
    ID3D11BlendState *g_Im3dBlendState = nullptr;
    ID3D11DepthStencilState *g_Im3dDepthStencilState = nullptr;
    ID3D11Buffer *g_Im3dConstantBuffer = nullptr;
    ID3D11Buffer *g_Im3dVertexBuffer = nullptr;

};

DX11Renderer::DX11Renderer()
: m_impl(new DX11RendererImpl)
{

}

DX11Renderer::~DX11Renderer()
{
    delete m_impl;
}

void DX11Renderer::NewFrame(int screenWidth, int screenHeight)
{

}

void DX11Renderer::DrawTeapot(const float *viewProjection, const float *world)
{

}
