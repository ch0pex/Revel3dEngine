/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details)
 ************************************************************************/
/**
 * @file gl_render_layers.hpp
 * @version 1.0
 * @date 10/04/2024
 * @brief Short description
 *
 * Longer description
 */

#pragma once

#include "render/mesh.hpp"
#include "gl_render_info.hpp"

namespace reveal3d::graphics::opengl {

struct Layer {
    u32 shaderId;
};

class RenderLayers  {
public:
    void init();
    void addMesh(render::SubMesh &mesh);
    void draw(std::vector<RenderElement> &render_elments, math::mat4 &pass_constants, u32 layer);

    inline Layer& operator[] (u32 index) { return layers_[index]; }
    inline const Layer& operator[] (u32 index) const { return layers_[index]; }

private:
    static std::string readShader(const char*file_name);
    static u32 createShader(GLenum shader_type, std::string &source, const char*shader_name);
    static u32 createProgram(const char* vs, const char* fs);

    Layer layers_[render::Shader::count];
    std::array<std::vector<render::SubMesh *>, render::Shader::count> sub_meshes_;

};

}
