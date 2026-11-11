#pragma once

#include <string>

namespace byteforge::rendering
{
    struct collider_shader_uniform_names_t
    {
    public:
        std::string projection_matrix;
        std::string model_matrix;

        collider_shader_uniform_names_t()
            : projection_matrix("u_projection_matrix")
            , model_matrix("u_model_matrix")
        {
        }
    };

    struct collider_shader_descriptor_t
    {
    public:
        std::string vertex_shader_path;
        std::string fragment_shader_path;
        collider_shader_uniform_names_t uniform_names;

        collider_shader_descriptor_t() : vertex_shader_path(), fragment_shader_path(), uniform_names() {}

        collider_shader_descriptor_t(const char* vertex_shader_path, const char* fragment_shader_path)
            : vertex_shader_path(vertex_shader_path)
            , fragment_shader_path(fragment_shader_path)
            , uniform_names()
        {
        }
    };
}
