#pragma once

#include <string>

namespace byteforge::rendering
{
    struct sprite_shader_uniform_names_t
    {
    public:
        std::string projection_matrix;
        std::string model_matrix;
        std::string tint;
        std::string use_texture;
        std::string uv_rect;
        std::string texture;

        sprite_shader_uniform_names_t()
            : projection_matrix("u_projection_matrix")
            , model_matrix("u_model_matrix")
            , tint("u_tint")
            , use_texture("u_use_texture")
            , uv_rect("u_uv_rect")
            , texture("u_texture")
        {
        }
    };

    struct sprite_shader_descriptor_t
    {
    public:
        std::string vertex_shader_path;
        std::string fragment_shader_path;
        sprite_shader_uniform_names_t uniform_names;

        sprite_shader_descriptor_t() : vertex_shader_path(), fragment_shader_path(), uniform_names() {}

        sprite_shader_descriptor_t(const char* vertex_shader_path, const char* fragment_shader_path)
            : vertex_shader_path(vertex_shader_path)
            , fragment_shader_path(fragment_shader_path)
            , uniform_names()
        {
        }
    };
}
