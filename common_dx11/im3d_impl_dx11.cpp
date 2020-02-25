#include "im3d_impl_dx11.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <im3d.h>
#include <wrl/client.h> // ComPtr
#include <string>
#include <plog/Log.h>

const std::string g_hlsl =
#include "../shaders/im3d.hlsl"
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

class Im3dImplDx11Impl
{
    struct D3DShader
    {
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11GeometryShader> m_gs;
        ComPtr<ID3D11PixelShader> m_ps;
        void Set(ID3D11DeviceContext *ctx, bool useGS = true)
        {
            ctx->VSSetShader(m_vs.Get(), nullptr, 0);
            ctx->PSSetShader(m_ps.Get(), nullptr, 0);
            if (useGS)
            {
                ctx->GSSetShader(m_gs.Get(), nullptr, 0);
            }
        }
    };

    D3DShader g_Im3dShaderPoints;
    D3DShader g_Im3dShaderLines;
    D3DShader g_Im3dShaderTriangles;
    ComPtr<ID3D11InputLayout> g_Im3dInputLayout;
    ComPtr<ID3D11RasterizerState> g_Im3dRasterizerState;
    ComPtr<ID3D11BlendState> g_Im3dBlendState;
    ComPtr<ID3D11DepthStencilState> g_Im3dDepthStencilState;
    ComPtr<ID3D11Buffer> g_Im3dConstantBuffer;
    ComPtr<ID3D11Buffer> g_Im3dVertexBuffer;

public:
    void Draw(ID3D11DeviceContext *ctx, const float *viewProjection)
    {
        auto &ad = Im3d::GetAppData();

        ComPtr<ID3D11Device> d3d;
        ctx->GetDevice(&d3d);

        if (!g_Im3dShaderPoints.m_vs)
        {
            // points shader
            D3D_SHADER_MACRO vsPointsDefs[] =
                {
                    {
                        .Name = "VERTEX_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "POINTS",
                        .Definition = "1",
                    },
                    {0}};
            auto vsBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@points:vs", vsPointsDefs, "vs_4_0");
            if (!vsBlob)
            {
                return;
            }
            if (FAILED(d3d->CreateVertexShader((DWORD *)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_vs)))
            {
                return;
            }

            {
                D3D11_INPUT_ELEMENT_DESC desc[] = {
                    {"POSITION_SIZE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(Im3d::VertexData, m_positionSize), D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(Im3d::VertexData, m_color), D3D11_INPUT_PER_VERTEX_DATA, 0},
                };
                if (FAILED(d3d->CreateInputLayout(desc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_Im3dInputLayout)))
                {
                    return;
                }
            }

            D3D_SHADER_MACRO gsPointsDefs[] =
                {
                    {
                        .Name = "GEOMETRY_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "POINTS",
                        .Definition = "1",
                    },
                    {0}};
            auto gsBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@points:gs", gsPointsDefs, "gs_4_0");
            if (!gsBlob)
            {
                return;
            }
            if (FAILED(d3d->CreateGeometryShader((DWORD *)gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_gs)))
            {
                return;
            }

            D3D_SHADER_MACRO psPointsDefs[] =
                {
                    {
                        .Name = "PIXEL_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "POINTS",
                        .Definition = "1",
                    },
                    {0}};
            auto psBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@points:ps", psPointsDefs, "ps_4_0");
            if (!psBlob)
            {
                return;
            }
            if (FAILED(d3d->CreatePixelShader((DWORD *)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_Im3dShaderPoints.m_ps)))
            {
                return;
            }
        }

        if (!g_Im3dShaderLines.m_vs)
        {
            // lines shader
            D3D_SHADER_MACRO vsLinesDefs[] =
                {
                    {
                        .Name = "VERTEX_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "LINES",
                        .Definition = "1",
                    },
                    {0}};
            auto vsBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@lines:vs", vsLinesDefs, "vs_4_0");
            if (!vsBlob)
            {
                return;
            }
            if (FAILED(d3d->CreateVertexShader((DWORD *)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_vs)))
            {
                return;
            }

            D3D_SHADER_MACRO gsLinesDefs[] =
                {
                    {
                        .Name = "GEOMETRY_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "LINES",
                        .Definition = "1",
                    },
                    {0}};
            auto gsBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@lines:gs", gsLinesDefs, "gs_4_0");
            if (!gsBlob)
            {
                return;
            }
            if (FAILED(d3d->CreateGeometryShader((DWORD *)gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_gs)))
            {
                return;
            }

            D3D_SHADER_MACRO psLinesDefs[] =
                {
                    {
                        .Name = "PIXEL_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "LINES",
                        .Definition = "1",
                    },
                    {0}};
            auto psBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@lines:ps", psLinesDefs, "ps_4_0");
            if (!psBlob)
            {
                return;
            }
            if (FAILED(d3d->CreatePixelShader((DWORD *)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_Im3dShaderLines.m_ps)))
            {
                return;
            }
        }

        if (!g_Im3dShaderTriangles.m_vs)
        {
            // triangles shader
            D3D_SHADER_MACRO vsTrianglesDefs[] =
                {
                    {
                        .Name = "VERTEX_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "TRIANGLES",
                        .Definition = "1",
                    },
                    {0}};
            auto vsBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@triangles:vs", vsTrianglesDefs, "vs_4_0");
            if (!vsBlob)
            {
                return;
            }
            if (FAILED(d3d->CreateVertexShader((DWORD *)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_Im3dShaderTriangles.m_vs)))
            {
                return;
            }

            D3D_SHADER_MACRO psTrianglesDefs[] =
                {
                    {
                        .Name = "PIXEL_SHADER",
                        .Definition = "1",
                    },
                    {
                        .Name = "TRIANGLES",
                        .Definition = "1",
                    },
                    {0}};
            auto psBlob = LoadCompileShader(g_hlsl, "im3d.hlsl@triangles:ps", psTrianglesDefs, "ps_4_0");
            if (!psBlob)
            {
                return;
            }
            if (FAILED(d3d->CreatePixelShader((DWORD *)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_Im3dShaderTriangles.m_ps)))
            {
                return;
            }
        }

        if (!g_Im3dRasterizerState)
        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE; // culling invalid for points/lines (they are view-aligned), valid but optional for triangles
            if (FAILED(d3d->CreateRasterizerState(&desc, &g_Im3dRasterizerState)))
            {
                return;
            }
        }
        ctx->RSSetState(g_Im3dRasterizerState.Get());

        if (!g_Im3dDepthStencilState)
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            if (FAILED(d3d->CreateDepthStencilState(&desc, &g_Im3dDepthStencilState)))
            {
                return;
            }
        }
        ctx->OMSetDepthStencilState(g_Im3dDepthStencilState.Get(), 0);

        if (!g_Im3dBlendState)
        {
            D3D11_BLEND_DESC desc = {};
            desc.RenderTarget[0].BlendEnable = true;
            desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            if (FAILED(d3d->CreateBlendState(&desc, &g_Im3dBlendState)))
            {
                return;
            }
        }
        ctx->OMSetBlendState(g_Im3dBlendState.Get(), nullptr, 0xffffffff);

        if (!g_Im3dConstantBuffer)
        {
            D3D11_BUFFER_DESC desc = {0};
            desc.ByteWidth = sizeof(Im3d::Mat4) + sizeof(Im3d::Vec4);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            if (FAILED(d3d->CreateBuffer(&desc, nullptr, &g_Im3dConstantBuffer)))
            {
                return;
            }
        }

        for (Im3d::U32 i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
        {
            auto &drawList = Im3d::GetDrawLists()[i];

            if (drawList.m_layerId == Im3d::MakeId("NamedLayer"))
            {
                // The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
            }

            // upload view-proj matrix/viewport size
            struct Layout
            {
                Im3d::Mat4 m_viewProj;
                Im3d::Vec2 m_viewport;
            };
            Layout layout{
                .m_viewProj = *(const Im3d::Mat4 *)viewProjection,
                .m_viewport = ad.m_viewportSize};
            ctx->UpdateSubresource(g_Im3dConstantBuffer.Get(), 0, nullptr, &layout, 0, 0);

            // upload vertex data
            static Im3d::U32 s_vertexBufferSize = 0;
            if (!g_Im3dVertexBuffer || s_vertexBufferSize < drawList.m_vertexCount)
            {
                if (g_Im3dVertexBuffer)
                {
                    g_Im3dVertexBuffer = nullptr;
                }
                s_vertexBufferSize = drawList.m_vertexCount;

                D3D11_BUFFER_DESC desc = {0};
                desc.ByteWidth = s_vertexBufferSize * sizeof(Im3d::VertexData);
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                if (FAILED(d3d->CreateBuffer(&desc, nullptr, &g_Im3dVertexBuffer)))
                {
                    return;
                }
            }

            D3D11_MAPPED_SUBRESOURCE subRes;
            if (SUCCEEDED(ctx->Map(g_Im3dVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes)))
            {
                memcpy(subRes.pData, drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));
                ctx->Unmap(g_Im3dVertexBuffer.Get(), 0);
            }
            else
            {
                return;
            }

            ID3D11Buffer *constants[] =
                {
                    g_Im3dConstantBuffer.Get()};

            // select shader/primitive topo
            switch (drawList.m_primType)
            {
            case Im3d::DrawPrimitive_Points:
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                ctx->GSSetConstantBuffers(0, _countof(constants), constants);
                g_Im3dShaderPoints.Set(ctx);
                break;
            case Im3d::DrawPrimitive_Lines:
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
                ctx->GSSetConstantBuffers(0, _countof(constants), constants);
                g_Im3dShaderLines.Set(ctx);
                break;
            case Im3d::DrawPrimitive_Triangles:
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                g_Im3dShaderTriangles.Set(ctx, false);
                break;
            default:
                IM3D_ASSERT(false);
                return;
            };

            UINT stride = sizeof(Im3d::VertexData);
            UINT offset = 0;
            ID3D11Buffer *vertexBuffers[] = {
                g_Im3dVertexBuffer.Get(),
            };
            ctx->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, &stride, &offset);
            ctx->IASetInputLayout(g_Im3dInputLayout.Get());
            ctx->VSSetConstantBuffers(0, _countof(constants), constants);
            ctx->Draw(drawList.m_vertexCount, 0);

            ctx->VSSetShader(nullptr, nullptr, 0);
            ctx->GSSetShader(nullptr, nullptr, 0);
            ctx->PSSetShader(nullptr, nullptr, 0);
        }
    }
};

Im3dImplDx11::Im3dImplDx11()
    : m_impl(new Im3dImplDx11Impl)
{
}

Im3dImplDx11::~Im3dImplDx11()
{
    delete m_impl;
}

void Im3dImplDx11::Draw(void *deviceContext, const float *viewProjection)
{
    m_impl->Draw((ID3D11DeviceContext *)deviceContext, viewProjection);
}
