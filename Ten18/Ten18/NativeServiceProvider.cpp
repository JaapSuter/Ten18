#pragma once

#include "Ten18/NativeServiceProvider.Generated.h"
#include "Ten18/Renderer.Generated.h"

::Ten18::Renderer* __stdcall ::Ten18::NativeServiceProvider::get_Renderer()
{
    return new ::Ten18::Renderer();
}
