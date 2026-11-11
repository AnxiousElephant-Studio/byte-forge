#include <byte-forge-editor/ui/components/status_panel_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <imgui.h>

#include <cfloat>
#include <cstdio>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title     = "Status";
        const char* const frame_header     = "Frame";
        const char* const phases_header    = "Phases (ms)";
        const char* const scene_header     = "Scene";
        const char* const resources_header = "Resources";
        const char* const controls_header  = "Runtime Controls";

        const char* const frame_profile_name = "frame";
        const char* const phases_table_id    = "phases_table";
        const char* const history_plot_id    = "##frame_history";
        const char* const phase_column       = "Phase";
        const char* const last_column        = "Last";
        const char* const average_column     = "Average";

        const char* const stopped_label = "Stopped";
        const char* const playing_label = "Playing";
        const char* const paused_label  = "Paused";

        const char* const pause_label         = "Pause";
        const char* const resume_label        = "Resume";
        const char* const step_label          = "Step";
        const char* const time_scale_label    = "Time Scale";
        const char* const reset_label         = "Reset";
        const char* const ambient_light_label = "Ambient Light";
        const char* const overlay_format      = "%.1f ms";

        const char* const fps_format          = "FPS: %.0f  (target %u)";
        const char* const frame_time_format   = "Frame time: %.2f ms";
        const char* const work_format         = "Work: %.2f ms  |  waiting: %.2f ms";
        const char* const state_format        = "State: %s";
        const char* const entities_format     = "Entities alive: %zu  (pending spawn %zu, pending kill %zu)";
        const char* const types_format        = "Component types: %zu / %zu";
        const char* const systems_format      = "Systems: %zu";
        const char* const ecs_memory_format   = "ECS memory (estimate): %.1f KB";
        const char* const textures_format     = "Textures cached: %zu  (failed to load: %zu)";
        const char* const fonts_format        = "Fonts cached: %zu  (failed to load: %zu)";
        const char* const sounds_format       = "Sounds active: %zu";
        const char* const environments_format = "Script environments: %zu";
        const char* const lua_memory_format   = "Lua memory: %.1f KB";
        const char* const windows_format      = "Windows: %zu";
        const char* const phase_value_format  = "%.3f";

        const usize history_size        = 120;
        const usize overlay_buffer_size = 32;
        const i32   phases_column_count = 3;

        const f32 plot_height                     = 60.0f;
        const f32 time_scale_min                  = 0.0f;
        const f32 time_scale_max                  = 4.0f;
        const char* const profiler_enabled_label  = "Enable profiler";
        const char* const profiler_disabled_label = "The profiler is disabled. Enable it to measure frame and system times.";

        const f32 bytes_per_kilobyte = 1024.0f;
        const f64 milliseconds_per_second = 1000.0;
    }

    void status_panel_component_t::draw(editor_application_context_t& context)
    {
        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        bool is_profiler_enabled = context.profiler.is_enabled();

        if (ImGui::Checkbox(profiler_enabled_label, &is_profiler_enabled))
        {
            context.profiler.set_enabled(is_profiler_enabled);
            frame_history.clear();
            history_offset = 0;
        }

        if (ImGui::CollapsingHeader(frame_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            draw_frame(context);
        }

        if (ImGui::CollapsingHeader(phases_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            draw_phases(context);
        }

        if (ImGui::CollapsingHeader(scene_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            draw_scene(context);
        }

        if (ImGui::CollapsingHeader(resources_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            draw_resources(context);
        }

        if (ImGui::CollapsingHeader(controls_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            draw_controls(context);
        }
    }

    void status_panel_component_t::draw_frame(editor_application_context_t& context)
    {
        if (!context.profiler.is_enabled())
        {
            ImGui::TextUnformatted(profiler_disabled_label);
            return;
        }

        const time::profiler_sample_t* frame_sample = context.profiler.find(frame_profile_name);

        if (frame_sample == nullptr || frame_sample->last_ms <= 0.0)
        {
            return;
        }

        if (frame_history.size() < history_size)
        {
            frame_history.push_back(static_cast<f32>(frame_sample->last_ms));
        }
        else
        {
            frame_history[history_offset] = static_cast<f32>(frame_sample->last_ms);
            history_offset = (history_offset + 1) % history_size;
        }

        f64 work_ms = 0.0;

        for (usize index = 0; index < context.profiler.get_sample_count(); index++)
        {
            const time::profiler_sample_t& sample = context.profiler.get_sample(index);

            if (sample.name != frame_profile_name)
            {
                work_ms += sample.last_ms;
            }
        }

        ImGui::Text(fps_format, milliseconds_per_second / frame_sample->average_ms, static_cast<u32>(context.engine_runtime->frame_settings.target_fps));
        ImGui::Text(frame_time_format, frame_sample->last_ms);
        ImGui::Text(work_format, work_ms, frame_sample->last_ms - work_ms);

        char overlay[overlay_buffer_size];
        std::snprintf(overlay, sizeof(overlay), overlay_format, frame_sample->last_ms);

        ImGui::PlotLines(history_plot_id, frame_history.data(), static_cast<i32>(frame_history.size()), static_cast<i32>(history_offset), overlay, 0.0f, FLT_MAX, ImVec2(0.0f, plot_height));
    }

    void status_panel_component_t::draw_phases(editor_application_context_t& context)
    {
        if (!ImGui::BeginTable(phases_table_id, phases_column_count, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            return;
        }

        ImGui::TableSetupColumn(phase_column);
        ImGui::TableSetupColumn(last_column);
        ImGui::TableSetupColumn(average_column);
        ImGui::TableHeadersRow();

        for (usize index = 0; index < context.profiler.get_sample_count(); index++)
        {
            const time::profiler_sample_t& sample = context.profiler.get_sample(index);

            if (sample.name == frame_profile_name)
            {
                continue;
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(sample.name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(phase_value_format, sample.last_ms);
            ImGui::TableNextColumn();
            ImGui::Text(phase_value_format, sample.average_ms);
        }

        ImGui::EndTable();
    }

    void status_panel_component_t::draw_scene(editor_application_context_t& context)
    {
        const ecs::registry_t& registry = context.scene_context.scene.registry;

        const scene::scene_player_t& scene_player = context.scene_context.scene_player;

        const char* state = stopped_label;

        if (scene_player.is_running())
        {
            state = scene_player.is_paused() ? paused_label : playing_label;
        }

        ImGui::Text(state_format, state);
        ImGui::Text(entities_format, registry.entities.size(), registry.entities_to_add.size(), registry.entities_to_kill.size());
        ImGui::Text(types_format, registry.component_types.size(), ecs::MAX_COMPONENT_TYPES);
        ImGui::Text(systems_format, registry.systems.size());
        ImGui::Text(ecs_memory_format, static_cast<f32>(registry.get_memory_bytes()) / bytes_per_kilobyte);
    }

    void status_panel_component_t::draw_resources(editor_application_context_t& context)
    {
        const runtime::engine_runtime_t& engine_runtime = *context.engine_runtime;
        const assets::asset_manager_t& asset_manager    = engine_runtime.asset_manager;

        ImGui::Text(textures_format, asset_manager.texture_cache.get_count(), asset_manager.texture_cache.get_failed_count());
        ImGui::Text(fonts_format, asset_manager.font_cache.get_count(), asset_manager.font_cache.get_failed_count());
        ImGui::Text(sounds_format, engine_runtime.audio.get_active_count());
        ImGui::Text(environments_format, context.scene_context.scene_player.script_system.get_environment_count());
        ImGui::Text(lua_memory_format, static_cast<f32>(engine_runtime.scripting_context.get_memory_bytes()) / bytes_per_kilobyte);
        ImGui::Text(windows_format, engine_runtime.platform.window_manager.get_window_count());
    }

    void status_panel_component_t::draw_controls(editor_application_context_t& context)
    {
        scene::scene_player_t& player = context.scene_context.scene_player;

        ImGui::BeginDisabled(!player.is_running());

        if (ImGui::Button(player.is_paused() ? resume_label : pause_label))
        {
            player.set_paused(!player.is_paused());
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!player.is_paused());

        if (ImGui::Button(step_label))
        {
            player.request_step();
        }

        ImGui::EndDisabled();
        ImGui::EndDisabled();

        ImGui::SliderFloat(time_scale_label, &player.time_scale, time_scale_min, time_scale_max);
        ImGui::SameLine();

        if (ImGui::Button(reset_label))
        {
            player.time_scale = 1.0f;
        }

        ImGui::ColorEdit3(ambient_light_label, &context.scene_context.render_context.ambient_light_color.x);
    }
}
