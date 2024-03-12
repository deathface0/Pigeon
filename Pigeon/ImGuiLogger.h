#pragma once

#include <iostream>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

//ImGui logger with multiple args support
struct ImGuiLog
{
    ImGuiTextBuffer Buf;
    bool ScrollToBottom;


    void Clear() { Buf.clear(); }

    void AddLog(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        Buf.append(fmt);
        va_end(args);
        ScrollToBottom = true;
    }

    void Draw(const char* title)
    {
        ImGui::TextUnformatted(Buf.begin());
        if (ScrollToBottom)
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
    }
};