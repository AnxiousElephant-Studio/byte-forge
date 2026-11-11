#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/math/rects/recti.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/platform/display/window/window_descriptor.h>
#include <byte-forge-engine/modules/platform/display/window/window_event.h>
#include <byte-forge-engine/modules/platform/platform.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light.h>
#include <byte-forge-engine/modules/rendering/material/material.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>
#include <byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h>
#include <byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h>
#include <byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>
#include <byte-forge-engine/modules/rendering/window/renderer.h>
#include <byte-forge-engine/modules/time/frame_manager.h>

#include <cstdlib>

using namespace byteforge;

namespace
{
    const char* const window_title = "ByteForge - Sandbox";
    const math::recti_t window_bounds(0, 0, 1280, 720);
    const u8 target_fps = 60;

    const char* const startup_error_format = "Could not start the sandbox. Cause: %s";

    const char* const quad_material_path = "assets/materials/striped_quad.glsl";

    const char* const light_vertex_shader_path   = "assets/shaders/light_vertex.glsl";
    const char* const light_fragment_shader_path = "assets/shaders/light_fragment.glsl";

    const char* const screen_vertex_shader_path   = "assets/shaders/screen_vertex.glsl";
    const char* const screen_fragment_shader_path = "assets/shaders/screen_fragment.glsl";

    const u32 scene_texture_slot = 0;
    const u32 light_texture_slot = 1;

    const math::vec4_t clear_color(0.08f, 0.08f, 0.10f, 1.0f);

    const math::vec4_t ambient_light_color(0.35f, 0.35f, 0.50f, 1.0f);
    const math::vec4_t quad_line_color(0.0f, 1.0f, 1.0f, 1.0f);
    const f32 quad_pan_speed           = 0.1f;
    const f32 quad_size_in_pixels      = 300.0f;
    const f32 rotation_degrees_per_sec = 45.0f;

    const rendering::opengl_framebuffer_descriptor_t scene_framebuffer_descriptor(true, false);
    const rendering::opengl_framebuffer_descriptor_t light_framebuffer_descriptor(false, false);
}

int main()
{
    bool should_close = false;

    platform::platform_t platform(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), &should_close);
    rendering::renderer_t renderer;
    time::frame_manager_t frame_manager;

    if (!platform.initialize() || !renderer.initialize(platform.display_descriptor.graphic_backend))
    {
        LOG_FATAL(startup_error_format, "the platform or the renderer failed to initialize.");
        return EXIT_FAILURE;
    }

    const platform::window_descriptor_t descriptor(window_title, window_bounds, platform::e_window_flags::Resizable, true);
    platform::window_t* window = platform.window_manager.create_window(descriptor, platform.display_descriptor);

    if (window == nullptr || !renderer.attach_window(*window) || !renderer.begin_frame(*window))
    {
        LOG_FATAL(startup_error_format, "the window or its OpenGL context could not be created.");
        renderer.dispose();
        platform.dispose();
        return EXIT_FAILURE;
    }

    rendering::opengl_mesh_t quad;
    rendering::material_t quad_material;
    rendering::opengl_shader_t light_shader;
    rendering::opengl_shader_t screen_shader;

    if (!quad.create_quad()
        || !quad_material.load(quad_material_path, rendering::e_material_shading_models::Unlit)
        || !light_shader.create(light_vertex_shader_path, light_fragment_shader_path)
        || !screen_shader.create(screen_vertex_shader_path, screen_fragment_shader_path))
    {
        LOG_FATAL(startup_error_format, "the quad or one of its shaders could not be created.");
        screen_shader.dispose();
        light_shader.dispose();
        quad_material.dispose();
        quad.dispose();
        renderer.dispose();
        platform.dispose();
        return EXIT_FAILURE;
    }

    const i32 light_projection_location = light_shader.get_uniform_location("u_projection");
    const i32 light_model_location      = light_shader.get_uniform_location("u_model");
    const i32 light_position_location   = light_shader.get_uniform_location("u_light_position");
    const i32 light_radius_location     = light_shader.get_uniform_location("u_light_radius");
    const i32 light_color_location      = light_shader.get_uniform_location("u_light_color");
    const i32 light_intensity_location  = light_shader.get_uniform_location("u_light_intensity");

    const i32 screen_scene_location = screen_shader.get_uniform_location("u_scene");
    const i32 screen_light_location = screen_shader.get_uniform_location("u_light");

    platform::input_event_callback_t input_event_callback;
    platform::window_event_callback_t window_event_callback;

    platform::input_state_t& input_state = platform.context.input_state;

    input_event_callback.callback = [&input_state](const platform::input_event_t& event)
    {
        input_state.process(event);
    };

    window_event_callback.callback = [&platform](const platform::window_event_t& event)
    {
        if (event.type == platform::e_window_event_types::CloseRequested)
        {
            platform.request_close();
        }
    };

    frame_manager.reset(time::frame_settings_t(target_fps));

    rendering::point_light_t point_light;

    rendering::opengl_framebuffer_t scene_framebuffer(scene_framebuffer_descriptor);
    rendering::opengl_framebuffer_t light_framebuffer(light_framebuffer_descriptor);

    f32 rotation_degrees = 0.0f;

    while (!should_close)
    {
        frame_manager.start_frame();

        input_state.begin_frame();
        platform.poll_events(input_event_callback, window_event_callback);

        point_light.position = input_state.mouse_position;

        if (!renderer.begin_frame(*window))
        {
            frame_manager.end_frame();
            continue;
        }

        const math::vec2i_t window_size = window->get_size();
        const f32 window_width          = static_cast<f32>(window_size.x);
        const f32 window_height         = static_cast<f32>(window_size.y);

        if (!window_size.is_positive() || !scene_framebuffer.resize(window_size) || !light_framebuffer.resize(window_size))
        {
            renderer.end_frame(*window);
            frame_manager.end_frame();
            continue;
        }

        const math::mat4_t projection = math::mat4_t::orthographic(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);

        rendering::opengl_render_pass_t::begin(scene_framebuffer, clear_color);

        rendering::opengl_render_state_t::enable_blending();
        rendering::opengl_render_state_t::use_alpha_blending();

        rotation_degrees += rotation_degrees_per_sec * static_cast<f32>(frame_manager.delta_time);

        const math::mat4_t quad_model = math::mat4_t::identity()
            .translate(window_width * 0.5f, window_height * 0.5f, 0.0f)
            .rotate_z(rotation_degrees)
            .scale(quad_size_in_pixels, quad_size_in_pixels, 1.0f);

        quad_material.bind();

        quad_material.set_projection(projection);
        quad_material.set_model(quad_model);
        quad_material.set_vector_4("u_line_color", quad_line_color);
        quad_material.set_float("u_pan_speed", quad_pan_speed);
        quad_material.set_time(static_cast<f32>(frame_manager.get_elapsed_time()));

        quad.draw();

        rendering::opengl_render_pass_t::begin(light_framebuffer, ambient_light_color);

        rendering::opengl_render_state_t::enable_blending();
        rendering::opengl_render_state_t::use_additive_blending();

        const f32 light_diameter = point_light.get_diameter();

        const math::mat4_t light_model = math::mat4_t::identity()
            .translate(point_light.position.x, point_light.position.y, 0.0f)
            .scale(light_diameter, light_diameter, 1.0f);

        light_shader.bind();

        light_shader.set_matrix_4(light_projection_location, projection);
        light_shader.set_matrix_4(light_model_location, light_model);
        light_shader.set_vector_2(light_position_location, point_light.position);
        light_shader.set_float(light_radius_location, point_light.radius);
        light_shader.set_vector_4(light_color_location, point_light.color);
        light_shader.set_float(light_intensity_location, point_light.intensity);

        quad.draw();

        rendering::opengl_render_pass_t::begin_window(window_size);

        rendering::opengl_render_state_t::disable_blending();

        screen_shader.bind();

        scene_framebuffer.get_color_texture().bind(scene_texture_slot);
        screen_shader.set_int(screen_scene_location, static_cast<i32>(scene_texture_slot));

        light_framebuffer.get_color_texture().bind(light_texture_slot);
        screen_shader.set_int(screen_light_location, static_cast<i32>(light_texture_slot));

        quad.draw();

        renderer.end_frame(*window);
        frame_manager.end_frame();
    }

    light_framebuffer.dispose();
    scene_framebuffer.dispose();
    screen_shader.dispose();
    light_shader.dispose();
    quad_material.dispose();
    quad.dispose();
    renderer.dispose();
    platform.dispose();

    return EXIT_SUCCESS;
}
