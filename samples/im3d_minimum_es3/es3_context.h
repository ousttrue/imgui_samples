#pragma once

class ES3ContextImpl;
class ES3Context
{
    ES3ContextImpl *m_impl = nullptr;

public:
    ES3Context();
    ~ES3Context();
    bool Create(void *hwnd);
    void Present();
};
