#pragma once

#include <atomic>

#include <bgfx/bgfx.h>
#include <vector>
#include <mutex>
#include <cassert>

class ViewIdManager
{
public:

    static inline bgfx::ViewId allocateViewId()
    {
        {
            std::lock_guard<std::mutex> lock(s_viewIdMutex);
            if (!s_freeViewIds.empty())
            {
                uint16_t id = s_freeViewIds.back();
                s_freeViewIds.pop_back();
                return static_cast<bgfx::ViewId>(id);
            }
        }

        uint16_t id = s_nextViewId.fetch_add(1, std::memory_order_relaxed);
        assert(id < 256 && "Exceeded maximum bgfx view count");
        return static_cast<bgfx::ViewId>(id);
    }

    static inline void deallocateViewId(bgfx::ViewId id)
    {
        assert(id > 0 && id < 256);

        std::lock_guard<std::mutex> lock(s_viewIdMutex);
        s_freeViewIds.push_back(id);
    }

    static inline bgfx::ViewId getCurrentViewId() {
        return currentViewId;
	}

    static inline void setCurrentViewId(bgfx::ViewId id) {
        currentViewId = id;
	}

private:

    static inline std::atomic<uint16_t> s_nextViewId{ 10 }; // 0-9 reserved
    static inline std::vector<uint16_t> s_freeViewIds;
    static inline std::mutex s_viewIdMutex;

	static inline bgfx::ViewId currentViewId = 0;

};

