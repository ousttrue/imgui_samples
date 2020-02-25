#pragma once
#include <d3d11.h>
#include <wrl/client.h>

struct Dx11RenderTarget
{
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

    bool Initialize(ID3D11Device *device, const Microsoft::WRL::ComPtr<ID3D11Texture2D> &texture, bool useSRV)
    {
        // create RTV
        if (FAILED(device->CreateRenderTargetView(texture.Get(), nullptr, &m_rtv)))
        {
            return false;
        }

        // create SRV
        if (useSRV)
        {
            if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, &m_srv)))
            {
                return false;
            }
        }

        D3D11_TEXTURE2D_DESC textureDesc;
        texture->GetDesc(&textureDesc);

        // create depthbuffer
        D3D11_TEXTURE2D_DESC depthDesc = {0};
        depthDesc.Width = textureDesc.Width;
        depthDesc.Height = textureDesc.Height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBuffer;
        if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer)))
        {
            return false;
        }

        // create dsv
        if (FAILED(device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &m_dsv)))
        {
            return false;
        }

        return true;
    }

    void ClearAndSet(ID3D11DeviceContext *context, const float *clear, float depth, UINT8 stencil,
                     int width, int height)
    {
        if (m_rtv)
        {
            context->ClearRenderTargetView(m_rtv.Get(), clear);
        }
        if (m_dsv)
        {
            context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, depth, stencil);
        }

        // set backbuffer & depthbuffer
        ID3D11RenderTargetView *rtv_list[] = {
            m_rtv.Get()};
        context->OMSetRenderTargets(1, rtv_list, m_dsv.Get());
        D3D11_VIEWPORT viewports[] =
            {
                {
                    .TopLeftX = 0,
                    .TopLeftY = 0,
                    .Width = (float)width,
                    .Height = (float)height,
                    .MinDepth = 0,
                    .MaxDepth = 1.0f,
                },
            };
        context->RSSetViewports(_countof(viewports), viewports);
    }
};
