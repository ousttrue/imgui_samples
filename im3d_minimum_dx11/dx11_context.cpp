#include "dx11_context.h"
#include <d3d11.h>
#include <stdio.h>
#include <wrl/client.h> // ComPtr

using namespace Microsoft::WRL;

class DX11ContextImpl
{
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11DeviceContext> m_d3dDeviceCtx;
    ComPtr<IDXGISwapChain> m_dxgiSwapChain;
    ComPtr<ID3D11RenderTargetView> m_d3dRenderTarget;
    ComPtr<ID3D11DepthStencilView> m_d3dDepthStencil;

public:
    ID3D11Device *Create(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swapChainDesc.SampleDesc.Count = 1;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[1] = {
            D3D_FEATURE_LEVEL_11_0,
        };
        if (FAILED(D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                createDeviceFlags,
                featureLevelArray,
                1,
                D3D11_SDK_VERSION,
                &swapChainDesc,
                &m_dxgiSwapChain,
                &m_d3dDevice,
                &featureLevel,
                &m_d3dDeviceCtx)))
        {
            fprintf(stderr, "Error initializing DirectX");
            return nullptr;
        }

        return m_d3dDevice.Get();
    }

#if 0
        ID3D11Texture2D *backBuffer;
        if (FAILED(m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBuffer)))
        {
            return false;
        }
        D3D11_TEXTURE2D_DESC backBufferDesc;
        backBuffer->GetDesc(&backBufferDesc);

        if (FAILED(m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &m_d3dRenderTarget)))
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC txDesc = {0};
        txDesc.Width = backBufferDesc.Width;
        txDesc.Height = backBufferDesc.Height;
        txDesc.MipLevels = 1;
        txDesc.ArraySize = 1;
        txDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        txDesc.SampleDesc.Count = 1;
        txDesc.Usage = D3D11_USAGE_DEFAULT;
        txDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        ID3D11Texture2D *tx = nullptr;
        if (FAILED(m_d3dDevice->CreateTexture2D(&txDesc, nullptr, &tx)))
        {
            return false;
        }

        ID3D11DepthStencilView *ret;
        if (FAILED(m_d3dDevice->CreateDepthStencilView(tx, nullptr, &ret)))
        {
            return false;
        }

        m_d3dDeviceCtx->OMSetRenderTargets(1, &m_d3dRenderTarget, m_d3dDepthStencil);
        backBuffer->Release();

        return true;
    }
#endif

    void Present()
    {
        m_dxgiSwapChain->Present(0, 0);
    }
};

DX11Context::DX11Context()
    : m_impl(new DX11ContextImpl)
{
}

DX11Context::~DX11Context()
{
    delete m_impl;
}

void *DX11Context::Create(void *hwnd)
{
    return m_impl->Create((HWND)hwnd);
}

void DX11Context::Present()
{
    m_impl->Present();
}
