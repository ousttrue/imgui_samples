#pragma once

class DX11ViewImpl;
namespace screenstate
{
    struct ScreenState;
}
class DX11View
{
    DX11ViewImpl *m_impl = nullptr;

public:
    DX11View();
    ~DX11View();
    void *Draw(void *deviceContext, const screenstate::ScreenState &viewState);
};
