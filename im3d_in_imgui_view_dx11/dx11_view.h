#pragma once

class DX11ViewImpl;
class DX11View
{
    DX11ViewImpl *m_impl = nullptr;

public:
    DX11View();
    ~DX11View();
    void *Draw(void *deviceContext, const struct WindowState &viewState);
};
