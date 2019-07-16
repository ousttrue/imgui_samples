#pragma once

class DX11Im3dImpl;
class DX11Im3d
{
    DX11Im3dImpl *m_impl = nullptr;

public:
    DX11Im3d();
    ~DX11Im3d();
    void Draw(void *deviceContext, const float *viewProjection);
};
