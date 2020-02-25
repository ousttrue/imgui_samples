#pragma once

class Im3dImplDx11Impl;
class Im3dImplDx11
{
    Im3dImplDx11Impl *m_impl = nullptr;

public:
    Im3dImplDx11();
    ~Im3dImplDx11();
    void Draw(void *deviceContext, const float *viewProjection);
};
