/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details) 
 ************************************************************************/
/**
 * @file renderer.hpp
 * @version 1.0
 * @date 25/02/2024
 * @brief Render class 
 *
 * Renderer template class, abstracts at a very high level the HRI (Hardware render interface)
 * 
 */

#pragma once

#include "camera.hpp"
#include "core/scene.hpp"
#include "graphics/gfx.hpp"

#include "IMGUI/backends/imgui_impl_win32.h"
#include "IMGUI/backends/imgui_impl_dx12.h"
#include <functional>

namespace reveal3d::render {

template<graphics::HRI Gfx>
class Renderer {
public:
    Renderer(window::Resolution *res, Timer &timer);
    void Init(WHandle wHandle);
    void Update();
    void Render();
    void Destroy();
    void Resize(const window::Resolution &res);

    Gfx& Graphics() { return graphics_; }

    INLINE f32 DeltaTime() const { return timer_.DeltaTime(); }
    INLINE  void CameraResetMouse() { camera_.ResetMouse(); }
    INLINE Timer& Time() { return timer_; }

private:
    Gfx graphics_;
    Camera camera_;
    Timer& timer_;
};

template<graphics::HRI Gfx>
Renderer<Gfx>::Renderer(window::Resolution *res, Timer &timer)
        : camera_(*res),
          graphics_(res),
          timer_(timer)
{

}

template<graphics::HRI Gfx>
void Renderer<Gfx>::Init(WHandle wHandle) {
    f32 time = timer_.TotalTime();
    graphics_.SetWindow(wHandle);
    graphics_.LoadPipeline();
    logger(logDEBUG) << "Initializing Pipeline...[" << timer_.Diff(time) * 1000 <<"ms]";
    time = timer_.TotalTime();
    graphics_.LoadAssets();
    logger(logDEBUG) << "Loading assets...[" << timer_.Diff(time) * 1000 <<"ms]";
}

template<graphics::HRI Gfx>
void Renderer<Gfx>::Update() {
    camera_.Update(timer_);
    graphics_.Update(camera_);
}

template<graphics::HRI Gfx>
void Renderer<Gfx>::Render() {
    graphics_.RenderSurface();
}

template<graphics::HRI Gfx>
void Renderer<Gfx>::Destroy() {
    logger(logDEBUG) << "Cleaning pipeline...[" << timer_.TotalTime()  <<"]";
    graphics_.Terminate();
}

template<graphics::HRI Gfx>
void Renderer<Gfx>::Resize(const window::Resolution &res) {
    camera_.Resize(res);
    graphics_.Resize(res);
}
}