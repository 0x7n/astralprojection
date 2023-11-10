#include "custom_elements.hpp"

bool c_gui::tab(const char* name, bool active, ImVec2 size_arg) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(name);
    const ImVec2 label_size = CalcTextSize(name, NULL, true);
    DWORD flags = ImGuiWindowFlags_None;


    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
    if (pressed)
        MarkItemEdited(id);

    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

    ImColor color = ImColor(121, 121, 121, 255);
    if (active) {
        window->DrawList->AddRectFilled(bb.Min, bb.Max, col); // ImColor(37, 37, 51, 255)
        window->DrawList->AddRectFilled(bb.Min, bb.Max - ImVec2(142, 0), ImColor(7, 129, 227, 255));
        color = ImColor(255, 255, 255, 255);
    }
    
    window->DrawList->AddText(bb.Min + ImVec2(15, 7), ImColor(0, 0, 0, 70), name);
    window->DrawList->AddText(bb.Min + ImVec2(14, 6), color, name);


    return pressed;
}
