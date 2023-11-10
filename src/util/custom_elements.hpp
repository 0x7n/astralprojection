#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <windows.h>
#include "../../ext/imgui/imgui.h"
#include "../../ext/imgui/imgui_internal.h"

using namespace ImGui;

class c_gui
{
public:
    bool tab(const char* name, bool active, ImVec2 size_arg);
};

