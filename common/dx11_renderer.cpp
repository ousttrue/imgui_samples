#include "dx11_renderer.h"
#include "dx11_rendertarget.h"
#include "teapot.h"
#include <string>
#include <array>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h> // ComPtr
#include <plog/Log.h>

const std::string g_hlsl =
#include "../shaders/model.hlsl"
    ;

using namespace Microsoft::WRL;

static ComPtr<ID3DBlob> LoadCompileShader(const std::string &src, const char *name, const D3D_SHADER_MACRO *define, const char *target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

    ComPtr<ID3DBlob> ret;
    ComPtr<ID3DBlob> err;
    if (FAILED(D3DCompile(src.data(), src.size(), name, define, nullptr, "main", target, flags, 0, &ret, &err)))
    {
        auto error = (char *)err->GetBufferPointer();
        LOGE << name << ": " << error;
        LOGE << src;
        return nullptr;
    }
    return ret;
}

class DX11RendererImpl
{
    int m_width = 0;
    int m_height = 0;
    Dx11RenderTarget m_rt;

public:
    void *NewFrameToRenderTarget(ID3D11DeviceContext *deviceContext, int width, int height, const float *clear)
    {
        if (!m_rt.m_rtv || width != m_width || height != m_height)
        {
            m_rt = Dx11RenderTarget();
            m_width = width;
            m_height = height;

            ComPtr<ID3D11Device> device;
            deviceContext->GetDevice(&device);

            // create depthbuffer
            D3D11_TEXTURE2D_DESC desc = {0};
            desc.Width = m_width;
            desc.Height = m_height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            ComPtr<ID3D11Texture2D> texture;
            if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
            {
                return nullptr;
            }

            if (!m_rt.Initialize(device.Get(), texture, true))
            {
                return nullptr;
            }
        }

        m_rt.ClearAndSet(deviceContext, clear, 1.0f, 0xff, m_width, m_height);

        return m_rt.m_srv.Get();
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

void *DX11Renderer::NewFrameToRenderTarget(void *deviceContext, int width, int height, const float *clear)
{
    return m_impl->NewFrameToRenderTarget((ID3D11DeviceContext *)deviceContext, width, height, clear);
}

struct ConstantBuffer
{
    std::array<float, 16> World;
    std::array<float, 16> ViewProjection;
};
static_assert(sizeof(ConstantBuffer) == sizeof(float) * 16 * 2);

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
        D3D_SHADER_MACRO vsMacro[] =
            {
                {
                    .Name = "VERTEX_SHADER",
                    .Definition = "1",
                },
                {0}};
        auto vsBlob = LoadCompileShader(g_hlsl, "model.hlsl@vs", vsMacro, "vs_4_0");
        if (!vsBlob)
        {
            return;
        }
        if (FAILED(d3d->CreateVertexShader((DWORD *)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &s_vs)))
        {
            return;
        }

        // create layout for vs
        D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};
        if (FAILED(d3d->CreateInputLayout(inputDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &s_inputLayout)))
        {
            return;
        }

        D3D_SHADER_MACRO psMacro[] =
            {
                {
                    .Name = "PIXEL_SHADER",
                    .Definition = "1",
                },
                {0}};
        auto psBlob = LoadCompileShader(g_hlsl, "model.hlsl@ps", psMacro, "ps_4_0");
        if (!psBlob)
        {
            return;
        }
        if (FAILED(d3d->CreatePixelShader((DWORD *)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &s_ps)))
        {
            return;
        }

        {
            D3D11_BUFFER_DESC desc = {0};
            desc.ByteWidth = sizeof(s_teapotVertices);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA subRes = {0};
            subRes.pSysMem = s_teapotVertices;
            if (FAILED(d3d->CreateBuffer(&desc, &subRes, &s_vb)))
            {
                return;
            }
        }

        {
            D3D11_BUFFER_DESC desc = {0};
            desc.ByteWidth = sizeof(s_teapotIndices);
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA subRes = {0};
            subRes.pSysMem = s_teapotIndices;
            if (FAILED(d3d->CreateBuffer(&desc, &subRes, &s_ib)))
            {
                return;
            }
        }

        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(float) * 16 * 2;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            // desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            if (FAILED(d3d->CreateBuffer(&desc, nullptr, &s_cb)))
            {
                return;
            }
        }

        D3D11_RASTERIZER_DESC rasterizerDesc = {0};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = true;
        if (FAILED(d3d->CreateRasterizerState(&rasterizerDesc, &s_rasterizerState)))
        {
            return;
        }
    }

    static std::array<float, 16> identity = {
        1.0f, 0, 0, 0,
        0, 1.0f, 0, 0,
        0, 0, 1.0f, 0,
        0, 0, 0, 1.0f};

    ConstantBuffer data{
        .World = *(const std::array<float, 16> *)world,
        // .World = identity,
        .ViewProjection = *(const std::array<float, 16> *)viewProjection,
        // .ViewProjection = identity,
    };
    ctx->UpdateSubresource(s_cb.Get(), 0, nullptr, &data, 0, 0);

    unsigned int stride = 4 * 3 * 2;
    unsigned int offset = 0;
    ctx->IASetInputLayout(s_inputLayout.Get());
    ID3D11Buffer *vb_list[] =
        {
            s_vb.Get()};
    ctx->IASetVertexBuffers(0, 1, vb_list, &stride, &offset);
    ctx->IASetIndexBuffer(s_ib.Get(), DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(s_vs.Get(), nullptr, 0);
    ID3D11Buffer *cb_list[] =
        {
            s_cb.Get()};
    ctx->VSSetConstantBuffers(0, 1, cb_list);
    ctx->PSSetShader(s_ps.Get(), nullptr, 0);

    ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    ctx->OMSetDepthStencilState(nullptr, 0);
    ctx->RSSetState(s_rasterizerState.Get());

    ctx->DrawIndexed(sizeof(s_teapotIndices) / sizeof(unsigned), 0, 0);
}
