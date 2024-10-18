/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details) 
 ************************************************************************/
/**
 * @file win_32.hpp
 * @version 1.0
 * @date 27/02/2024
 * @brief Short description
 *
 * Win32 backend manager class
 */

#pragma once

#include "render/renderer.hpp"
#include "window/window_info.hpp"
#include "input/input.hpp"

#include <WindowsX.h>
#include <backends/imgui_impl_win32.h>


namespace reveal3d::window {

class Win32 {
public:
    using WCallback = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);

    Win32(Config &info);

    template<graphics::HRI Gfx> void Create(render::Renderer<Gfx> &renderer);
    template<graphics::HRI Gfx> void Update(render::Renderer<Gfx> &renderer);
    void Show();
    void CloseWindow(input::action act, input::type type);
    bool ShouldClose();

    [[nodiscard]] inline Resolution& GetRes() { return info_.res; }
    [[nodiscard]] inline WHandle GetHandle() const { return info_.handle; }
private:
    template<graphics::HRI Gfx> void ClipMouse(render::Renderer<Gfx> &renderer);
    template<graphics::HRI Gfx>
    static LRESULT DefaultProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    Config info_;
    MSG msg_ {};
    WCallback callback_ { nullptr };
    bool isRunning_ { false };
};

template<graphics::HRI Gfx>
void Win32::Update(render::Renderer<Gfx> &renderer) {
    while (PeekMessage(&msg_, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg_);
        DispatchMessage(&msg_);
        isRunning_ &= (msg_.message != WM_QUIT);
    }
    ClipMouse(renderer);
}

template<graphics::HRI Gfx>
LRESULT Win32::DefaultProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

#ifdef IMGUI
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
        return true;
#endif
    auto* renderer = reinterpret_cast<render::Renderer<Gfx>*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (message) {
        case WM_CREATE:
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return 0;
        }
        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE:
        case WM_SIZE:
        {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            const window::Resolution res(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            renderer->Resize(res);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        {
#ifdef IMGUI
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            if (io.WantCaptureMouse)
                return 0;
#endif
            input::KeyDown(wParam);
            return 0;
        }
        case WM_KEYUP:
            input::KeyUp(wParam);
            return 0;
        case WM_MBUTTONDOWN:
        {
            input::Cursor::shouldClip = true;
            input::Cursor::lastUnclipedPos = {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)};
            SetCapture(hwnd);
            SetCursor(NULL);
            input::KeyDown(input::code::mouse_middle);
            return 0;
        }
        case WM_MBUTTONUP:
        {
            input::Cursor::shouldClip = false;
            input::KeyUp(input::code::mouse_middle);
            RECT window_rect;
            GetWindowRect(hwnd, &window_rect);
            SetCursorPos(input::Cursor::lastUnclipedPos.x + window_rect.left, input::Cursor::lastUnclipedPos.y + window_rect.top);
            ReleaseCapture();
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            input::Cursor::pos = {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)};
            input::MouseMove(wParam, input::Cursor::pos);
            return 0;
        }
        case WM_RBUTTONDOWN:
            input::KeyDown(input::code::mouse_right, {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)});
            return 0;
        case WM_RBUTTONUP:
            input::KeyUp(input::code::mouse_right, {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)});
            return 0;
        case WM_LBUTTONDOWN:
            input::KeyDown(input::code::mouse_left, {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)});
            return 0;
        case WM_LBUTTONUP:
            input::KeyUp(input::code::mouse_left, {(f32)GET_X_LPARAM(lParam), (f32)GET_Y_LPARAM(lParam)});
            return 0;

    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

template<graphics::HRI Gfx>
void Win32::Create(render::Renderer<Gfx> &renderer) {
    WNDCLASSEXW windowClass = {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = callback_ ? callback_ : DefaultProc<Gfx>,
            .hInstance = GetModuleHandle(NULL),
            .hCursor = LoadCursor(NULL, IDC_ARROW),
            .lpszClassName = L"Reveal3dClass"
    };
    RegisterClassExW(&windowClass);

    RECT windowRect = {0, 0, static_cast<LONG>(info_.res.width), static_cast<LONG>(info_.res.height)};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    info_.handle.hwnd = CreateWindowExW(
            0,
            windowClass.lpszClassName,
            info_.name,
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            &renderer);

}

template<graphics::HRI Gfx>
void Win32::ClipMouse(render::Renderer<Gfx> &renderer) {
    if (!input::Cursor::shouldClip) return;

    if (input::Cursor::pos.x < 2) {
        input::Cursor::pos.x = info_.res.width - 3;
        renderer.CameraResetMouse();
    }
    else if (input::Cursor::pos.x >= info_.res.width - 2) {
        input::Cursor::pos.x = 3;
        renderer.CameraResetMouse();
    }
    if (input::Cursor::pos.y < 2) {
        input::Cursor::pos.y = info_.res.height - 3;
        renderer.CameraResetMouse();
    }
    else if (input::Cursor::pos.y >= info_.res.height - 2) {
        input::Cursor::pos.y = 3;
        renderer.CameraResetMouse();
    }

    POINT pt = {static_cast<LONG>(input::Cursor::pos.x), static_cast<LONG>(input::Cursor::pos.y)};
    ClientToScreen(info_.handle.hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
}


}