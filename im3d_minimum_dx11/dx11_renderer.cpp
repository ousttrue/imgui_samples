#include "dx11_renderer.h"
#include "shader_source.h"
#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h> // ComPtr
#include <plog/Log.h>

const std::string g_hlsl =
#include "../shaders/im3d.hlsl"
    ;

using namespace Microsoft::WRL;

ComPtr<ID3DBlob> LoadCompileShader(const std::string &target, const std::string &src)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> ret;
    ComPtr<ID3DBlob> err;
    if (FAILED(D3DCompile(src.data(), src.size(), nullptr, nullptr, nullptr, "main", target.c_str(), flags, 0, &ret, &err)))
    {
        LOGE << (char *)err->GetBufferPointer();
        return nullptr;
    }
    return ret;
}

class DX11RendererImpl
{

public:
    bool Create(ID3D11Device *device)
    {
        return true;
    }
};

DX11Renderer::DX11Renderer()
    : m_impl(new DX11RendererImpl)
{
}

DX11Renderer::~DX11Renderer()
{
    delete m_impl;
}

bool DX11Renderer::Create(void *device)
{
    return m_impl->Create((ID3D11Device *)device);
}

void* DX11Renderer::NewFrame(int screenWidth, int screenHeight)
{
    // setup backbuffer
    return nullptr;
}

void DX11Renderer::DrawTeapot(void *deviceContext, const float *viewProjection, const float *world)
{
    static ComPtr<ID3D11VertexShader> s_vs;
    static ComPtr<ID3D11PixelShader> s_ps;
    static ComPtr<ID3D11InputLayout> s_inputLayout;
    static ComPtr<ID3D11Buffer> s_vb;
    static ComPtr<ID3D11Buffer> s_ib;
    static ComPtr<ID3D11Buffer> s_cb;
    static ComPtr<ID3D11RasterizerState> s_rasterizerState;

    auto ctx = (ID3D11DeviceContext *)deviceContext;
    ComPtr<ID3D11Device> d3d;
    ctx->GetDevice(&d3d);
    if (!s_vs)
    {
        auto vs = ShaderSource(g_hlsl);
        vs.Define("VERTEX_SHADER");
        auto vsBlob = LoadCompileShader("vs_4_0", vs.GetSource());
        if (FAILED(d3d->CreateVertexShader((DWORD *)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &s_vs)))
        {
            return;
        }

        auto ps = ShaderSource(g_hlsl);
        ps.Define("PIXEL_SHADER");
        auto psBlob = LoadCompileShader("ps_4_0", ps.GetSource());
        if (FAILED(d3d->CreatePixelShader((DWORD *)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &s_ps)))
        {
            return;
        }

        // s_vb = CreateVertexBuffer(sizeof(s_teapotVertices), D3D11_USAGE_IMMUTABLE, s_teapotVertices);
        // s_ib = CreateIndexBuffer(sizeof(s_teapotIndices), D3D11_USAGE_IMMUTABLE, s_teapotIndices);

        // D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        //     {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        //     {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};
        // dxAssert(d3d->CreateInputLayout(inputDesc, 2, s_vsBlob->GetBufferPointer(), s_vsBlob->GetBufferSize(), &s_inputLayout));

        // s_cb = CreateConstantBuffer(sizeof(Mat4) * 2, D3D11_USAGE_DYNAMIC);

        // D3D11_RASTERIZER_DESC rasterizerDesc = {};
        // rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        // rasterizerDesc.CullMode = D3D11_CULL_BACK;
        // rasterizerDesc.FrontCounterClockwise = true;
        // dxAssert(d3d->CreateRasterizerState(&rasterizerDesc, &s_rasterizerState));
    }

    // Mat4 *cbData = (Mat4 *)MapBuffer(s_cb, D3D11_MAP_WRITE_DISCARD);
    // cbData[0] = _world;
    // cbData[1] = _viewProj;
    // UnmapBuffer(s_cb);

    // unsigned int stride = 4 * 3 * 2;
    // unsigned int offset = 0;
    // ctx->IASetInputLayout(s_inputLayout);
    // ctx->IASetVertexBuffers(0, 1, &s_vb, &stride, &offset);
    // ctx->IASetIndexBuffer(s_ib, DXGI_FORMAT_R32_UINT, 0);
    // ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ctx->VSSetShader(s_vs, nullptr, 0);
    // ctx->VSSetConstantBuffers(0, 1, &s_cb);
    // ctx->PSSetShader(s_ps, nullptr, 0);

    // ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    // ctx->OMSetDepthStencilState(nullptr, 0);
    // ctx->RSSetState(s_rasterizerState);

    // ctx->DrawIndexed(sizeof(s_teapotIndices) / sizeof(unsigned), 0, 0);
}
