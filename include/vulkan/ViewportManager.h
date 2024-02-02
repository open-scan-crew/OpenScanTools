#ifndef VIEWPORT_MANAGER_H
#define VIEWPORT_MANAGER_H

#include <cstdint>

class ViewportManager
{
public:
    ViewportManager();
    ~ViewportManager();

    void registerViewport(uint64_t winId);
};

#endif