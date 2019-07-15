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

void DX11Renderer::NewFrame(int screenWidth, int screenHeight)
{
}

void DX11Renderer::DrawTeapot(const float *viewProjection, const float *world)
{
}
