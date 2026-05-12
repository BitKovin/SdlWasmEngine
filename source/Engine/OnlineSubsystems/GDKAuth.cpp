#include "GDKAuth.h"

#ifdef __GDK__

#include <Logger.hpp>
#include <windows.h> 

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------

// XblCleanupAsync needs a queue to dispatch its completion on.
// This helper creates a temporary one when the session queue isn't ready yet
// (i.e. during early-init failure paths).
static void CleanupXbl(XTaskQueueHandle queue)
{
    XTaskQueueHandle tempQueue = nullptr;
    bool ownsTempQueue = false;

    if (!queue)
    {
        HRESULT hr = XTaskQueueCreate(
            XTaskQueueDispatchMode::ThreadPool,
            XTaskQueueDispatchMode::ThreadPool,
            &tempQueue);

        if (FAILED(hr))
        {
            // Nothing we can do — best effort
            Logger::Error("[GDKAuth] CleanupXbl: failed to create temp queue: %08X\n", hr);
            return;
        }
        queue = tempQueue;
        ownsTempQueue = true;
    }

    XAsyncBlock async{};
    async.queue = queue;

    HRESULT hr = XblCleanupAsync(&async);
    if (SUCCEEDED(hr))
    {
        XAsyncGetStatus(&async, /*wait=*/true);
    }

    if (ownsTempQueue)
    {
        XTaskQueueCloseHandle(tempQueue);
    }
}

static bool CreateQueue(XTaskQueueHandle& outQueue)
{
    HRESULT hr = XTaskQueueCreate(
        XTaskQueueDispatchMode::ThreadPool,
        XTaskQueueDispatchMode::ThreadPool,
        &outQueue);

    if (FAILED(hr))
    {
        Logger::Error("[GDKAuth] XTaskQueueCreate failed: %08X\n", hr);
        return false;
    }
    return true;
}

static bool SignInUser(XTaskQueueHandle queue, XUserHandle& outUser)
{
    HANDLE userEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (!userEvent)
    {
        Logger::Error("[GDKAuth] CreateEventEx failed\n");
        return false;
    }

    XAsyncBlock* async = new XAsyncBlock{};
    async->queue = queue;
    async->context = userEvent;
    async->callback = [](XAsyncBlock* a)
        {
            SetEvent(reinterpret_cast<HANDLE>(a->context));
        };

    HRESULT hr = XUserAddAsync(XUserAddOptions::AddDefaultUserAllowingUI, async);
    if (FAILED(hr))
    {
        Logger::Error("[GDKAuth] XUserAddAsync failed: %08X\n", hr);
        CloseHandle(userEvent);
        delete async;
        return false;
    }

    MSG msg{};
    while (WaitForSingleObjectEx(userEvent, 0, TRUE) == WAIT_TIMEOUT)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    hr = XUserAddResult(async, &outUser);
    CloseHandle(userEvent);
    delete async;

    if (FAILED(hr))
    {
        Logger::Error("[GDKAuth] XUserAddResult failed: %08X\n", hr);
        return false;
    }

    char gamertag[XUserGamertagComponentClassicMaxBytes]{};
    XUserGetGamertag(outUser, XUserGamertagComponent::Classic,
        sizeof(gamertag), gamertag, nullptr);
    Logger::Info("[GDKAuth] Signed in as: %s\n", gamertag);

    return true;
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------

GDKSession GDKAuth_Init(const char* scid)
{
    GDKSession session{};

    HRESULT hr = XGameRuntimeInitialize();
    if (FAILED(hr))
    {
        Logger::Error("[GDKAuth] XGameRuntimeInitialize failed: %08X\n", hr);
        return session;
    }

    XblInitArgs xblArgs{};
    xblArgs.scid = scid;
    hr = XblInitialize(&xblArgs);
    if (FAILED(hr))
    {
        Logger::Error("[GDKAuth] XblInitialize failed: %08X\n", hr);
        XGameRuntimeUninitialize();
        return session;
    }

    // Queue not ready yet — CleanupXbl creates a temp queue internally
    if (!CreateQueue(session.queue))
    {
        CleanupXbl(nullptr);
        XGameRuntimeUninitialize();
        return session;
    }

    if (!SignInUser(session.queue, session.user))
    {
        CleanupXbl(session.queue);
        XTaskQueueCloseHandle(session.queue);
        session.queue = nullptr;
        XGameRuntimeUninitialize();
        return session;
    }

    Logger::Info("[GDKAuth] GDK session ready.\n");
    return session;
}

void GDKAuth_Shutdown(GDKSession& session)
{
    if (session.user)
    {
        XUserCloseHandle(session.user);
        session.user = nullptr;
    }

    // CleanupXbl must fully finish before the queue is closed
    CleanupXbl(session.queue);

    if (session.queue)
    {
        XTaskQueueCloseHandle(session.queue);
        session.queue = nullptr;
    }

    XGameRuntimeUninitialize();

    Logger::Info("[GDKAuth] GDK session shut down.\n");
}

#endif