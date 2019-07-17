#pragma once
#include <string>

class Im3dImplGL3Impl;
class Im3dImplGL3
{
    Im3dImplGL3Impl *m_impl = nullptr;

public:
    Im3dImplGL3(const std::string &version);
    ~Im3dImplGL3();
    void Draw(const float *viewProjection);
};
