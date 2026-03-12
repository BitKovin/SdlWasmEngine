#pragma once

#include <atomic>

#include <bgfx/bgfx.h>
#include <vector>
#include <mutex>
#include <cassert>

class ViewIdManager
{
public:
    static void Reset()
    {
        s_nextViewId = 1;
        s_currentViewId = 0;
    }

    static bgfx::ViewId GiveNextId()
    {
        uint16_t id = s_nextViewId++;
        assert(id < 256 && "Exceeded maximum bgfx view count");
        s_currentViewId = id;
        return static_cast<bgfx::ViewId>(id);
    }

    static bgfx::ViewId GetCurrentId()
    {
        return s_currentViewId;
    }

private:
    static inline uint16_t s_nextViewId = 1;
    static inline bgfx::ViewId s_currentViewId = 0;
};

