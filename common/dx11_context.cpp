#include "dx11_context.h"
#include <d3d11.h>
#include <stdio.h>
#include <wrl/client.h> // ComPtr
#include <plog/Log.h>

using namespace Microsoft::WRL;

class DX11ContextImpl
{
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapchain;

    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11DepthStencilView> m_dsv;

public:
    ID3D11DeviceContext *GetDeviceContext()
    {
        return m_context.Get();
    }

    // create device and swapchain
    ID3D11Device *Create(HWND hWnd)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        swapChainDesc.SampleDesc.Count = 1;

        UINT createDeviceFlags = 0;
#if _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[] = {
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
                &m_swapchain,
                &m_device,
                &featureLevel,
                &m_context)))
        {
            LOGE << "Error initializing DirectX";
            return nullptr;
        }

        DXGI_SWAP_CHAIN_DESC desc;
        m_swapchain->GetDesc(&desc);
        m_width = desc.BufferDesc.Width;
        m_height = desc.BufferDesc.Height;

        return m_device.Get();
    }

    int m_width = 0;
    int m_height = 0;

    ID3D11DeviceContext *NewFrame(int width, int height)
    {
        if (width != m_width || height != m_height)
        {
            m_rtv.Reset();
            m_dsv.Reset();
            // clear
            m_context->OMSetRenderTargets(0, nullptr, nullptr);
            // resize swapchain
            DXGI_SWAP_CHAIN_DESC desc;
            m_swapchain->GetDesc(&desc);
            m_swapchain->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);

            m_width = width;
            m_height = height;
        }

        if (!m_rtv)
        {
            // get backbuffer
            ComPtr<ID3D11Texture2D> backBuffer;
            if (FAILED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
            {
                // fatal
                LOGE << "fail to get backbuffer";
                return nullptr;
            }
            D3D11_TEXTURE2D_DESC backBufferDesc;
            backBuffer->GetDesc(&backBufferDesc);

            // create RtV
            if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv)))
            {
                return false;
            }

            // create depthbuffer
            D3D11_TEXTURE2D_DESC depthDesc = {0};
            depthDesc.Width = backBufferDesc.Width;
            depthDesc.Height = backBufferDesc.Height;
            depthDesc.MipLevels = 1;
            depthDesc.ArraySize = 1;
            depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.Usage = D3D11_USAGE_DEFAULT;
            depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            ComPtr<ID3D11Texture2D> depthBuffer;
            if (FAILED(m_device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer)))
            {
                return false;
            }

            // create dsv
            if (FAILED(m_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &m_dsv)))
            {
                return false;
            }
        }

        // clear
        float clear[] = {0.3f, 0.3f, 0.3f, 0.0f};
        m_context->ClearRenderTargetView(m_rtv.Get(), clear);
        m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0xff);

        // set backbuffer & depthbuffer
        ID3D11RenderTargetView *rtv_list[] = {
            m_rtv.Get()};
        m_context->OMSetRenderTargets(1, rtv_list, m_dsv.Get());
        D3D11_VIEWPORT viewports[] =
            {
                {
                    .TopLeftX = 0,
                    .TopLeftY = 0,
                    .Width = (float)m_width,
                    .Height = (float)m_height,
                    .MinDepth = 0,
                    .MaxDepth = 1.0f,
                },
            };
        m_context->RSSetViewports(_countof(viewports), viewports);

        return m_context.Get();
    }

    void Present()
    {
        m_swapchain->Present(0, 0);
    }
};

//////////////////////////////////////////////////////////////////////////////
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

void *DX11Context::NewFrame(int width, int height)
{
    return m_impl->NewFrame(width, height);
}

void DX11Context::Present()
{
    m_impl->Present();
}

void *DX11Context::GetDeviceContext()
{
    return m_impl->GetDeviceContext();
}
