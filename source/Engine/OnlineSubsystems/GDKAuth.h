
#pragma once


#ifdef __GDK__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <XGameRuntime.h>
#include <XUser.h>
#include <xsapi-c/services_c.h>
#include <XGameSave.h>

struct GDKSession
{
    XUserHandle         user = nullptr;
    XTaskQueueHandle    queue = nullptr;
};

GDKSession  GDKAuth_Init(const char* scid);
void GDKAuth_Shutdown(GDKSession& session);

#endif