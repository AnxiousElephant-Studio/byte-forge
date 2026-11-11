#include <byte-forge-engine/modules/scene/scene_script_system.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/platform/input/input_names.h>

#include <lua.hpp>

#include <cstdio>

namespace byteforge::scene
{
    namespace
    {
        const i32 stack_top           = -1;
        const i32 environment_upvalue = 1;
        const i32 system_upvalue      = 1;

        const char* const start_function     = "on_start";
        const char* const update_function    = "on_update";
        const char* const collision_function = "on_collision";

        const char* const global_table_name = "_G";
        const char* const index_metafield   = "__index";

        const char* const load_error_format                 = "Could not load script at path: '%s' for entity ID: %llu. Cause: %s";
        const char* const call_error_format                 = "Could not call script function: '%s' for entity ID: %llu. Cause: %s";
        const char* const stamp_error_format               = "%s";
        const char* const unknown_error_message             = "Unknown error.";
        const char* const missing_stamp_loader_error       = "Could not spawn the stamp. Cause: the application did not set a stamp loader.";
        const char* const script_log_format                 = "[lua] %s";
        const char* const missing_component_error_format    = "Could not access component: '%s' of entity ID: %d. Cause: the entity does not have it.";
        const char* const missing_entity_error_format       = "Could not access entity ID: %d. Cause: the entity does not exist.";
        const char* const unknown_key_error_format          = "Could not find key: '%s'. Cause: unknown key name.";
        const char* const unknown_mouse_button_error_format = "Could not find mouse button: '%s'. Cause: unknown mouse button name.";
        const char* const text_format                       = "%s";
        const char* const default_entity_name               = "Entity";

        const char* const transform_label = "transform_2D";
        const char* const velocity_label  = "velocity_2D";
        const char* const label_label     = "label_2D";
        const char* const audio_label     = "audio_source";
        const char* const sprite_label    = "sprite_2D";
        const char* const camera_label    = "camera_2D";

        scene_script_system_t& get_system(lua_State* lua_state)
        {
            return *static_cast<scene_script_system_t*>(lua_touserdata(lua_state, lua_upvalueindex(system_upvalue)));
        }

        scene_t& get_scene(lua_State* lua_state)
        {
            return get_system(lua_state).get_scene();
        }

        platform::input_state_t& get_input_state(lua_State* lua_state)
        {
            return get_system(lua_state).get_input_state();
        }

        platform::e_key_codes check_key_code(lua_State* lua_state)
        {
            const char* name = luaL_checkstring(lua_state, 1);
            platform::e_key_codes key_code = platform::e_key_codes::Unknown;

            if (!platform::input_names_t::get_key_code(name, key_code))
            {
                luaL_error(lua_state, unknown_key_error_format, name);
            }

            return key_code;
        }

        platform::e_mouse_buttons check_mouse_button(lua_State* lua_state)
        {
            const char* name = luaL_checkstring(lua_state, 1);
            platform::e_mouse_buttons mouse_button = platform::e_mouse_buttons::None;

            if (!platform::input_names_t::get_mouse_button(name, mouse_button))
            {
                luaL_error(lua_state, unknown_mouse_button_error_format, name);
            }

            return mouse_button;
        }

        ecs::entity_t check_entity(lua_State* lua_state, const i32 argument)
        {
            return ecs::entity_t(static_cast<u64>(luaL_checkinteger(lua_state, argument)));
        }

        ecs::entity_t check_existing_entity(lua_State* lua_state, const i32 argument)
        {
            const ecs::entity_t entity = check_entity(lua_state, argument);

            if (!get_scene(lua_state).exists(entity))
            {
                luaL_error(lua_state, missing_entity_error_format, static_cast<i32>(entity.id));
            }

            return entity;
        }

        template <typename component_t>
        component_t* require_component(lua_State* lua_state, component_t* component, const ecs::entity_t entity, const char* component_label)
        {
            if (component == nullptr)
            {
                luaL_error(lua_state, missing_component_error_format, component_label, static_cast<i32>(entity.id));
            }

            return component;
        }

        components::transform_2D_component_t* get_transform(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            return require_component(lua_state, get_scene(lua_state).get_transform_2D(entity), entity, transform_label);
        }

        components::velocity_2D_component_t* get_velocity(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            return require_component(lua_state, get_scene(lua_state).get_velocity_2D(entity), entity, velocity_label);
        }

        components::label_2D_component_t* get_label(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            return require_component(lua_state, get_scene(lua_state).get_label_2D(entity), entity, label_label);
        }

        components::sprite_2D_component_t* get_sprite(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            return require_component(lua_state, get_scene(lua_state).get_sprite_2D(entity), entity, sprite_label);
        }

        components::camera_2D_component_t* get_camera(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            return require_component(lua_state, get_scene(lua_state).get_camera_2D(entity), entity, camera_label);
        }

        i32 push_vector(lua_State* lua_state, const math::vec2_t& vector)
        {
            lua_pushnumber(lua_state, vector.x);
            lua_pushnumber(lua_state, vector.y);
            return 2;
        }

        i32 push_color(lua_State* lua_state, const math::vec4_t& color)
        {
            lua_pushnumber(lua_state, color.x);
            lua_pushnumber(lua_state, color.y);
            lua_pushnumber(lua_state, color.z);
            lua_pushnumber(lua_state, color.w);
            return 4;
        }

        i32 push_entity(lua_State* lua_state, const ecs::entity_t entity)
        {
            lua_pushinteger(lua_state, static_cast<lua_Integer>(entity.id));
            return 1;
        }

        i32 push_entities(lua_State* lua_state, const std::vector<ecs::entity_t>& entities)
        {
            lua_createtable(lua_state, static_cast<i32>(entities.size()), 0);

            for (usize index = 0; index < entities.size(); index++)
            {
                lua_pushinteger(lua_state, static_cast<lua_Integer>(entities[index].id));
                lua_rawseti(lua_state, -2, static_cast<lua_Integer>(index + 1));
            }

            return 1;
        }

        math::vec2_t check_vector(lua_State* lua_state, const i32 first_argument)
        {
            return math::vec2_t(static_cast<f32>(luaL_checknumber(lua_state, first_argument)), static_cast<f32>(luaL_checknumber(lua_state, first_argument + 1)));
        }

        math::vec4_t check_color(lua_State* lua_state, const i32 first_argument)
        {
            return math::vec4_t(
                static_cast<f32>(luaL_checknumber(lua_state, first_argument)),
                static_cast<f32>(luaL_checknumber(lua_state, first_argument + 1)),
                static_cast<f32>(luaL_checknumber(lua_state, first_argument + 2)),
                static_cast<f32>(luaL_optnumber(lua_state, first_argument + 3, 1.0)));
        }

        template <usize size>
        void copy_text(char (&buffer)[size], const char* text)
        {
            std::snprintf(buffer, size, text_format, text);
        }

        i32 transform_get_position(lua_State* lua_state) { return push_vector(lua_state, get_transform(lua_state)->position); }
        i32 transform_set_position(lua_State* lua_state) { get_transform(lua_state)->position = check_vector(lua_state, 2); return 0; }
        i32 transform_get_scale(lua_State* lua_state)    { return push_vector(lua_state, get_transform(lua_state)->scale); }
        i32 transform_set_scale(lua_State* lua_state)    { get_transform(lua_state)->scale = check_vector(lua_state, 2); return 0; }

        i32 transform_get_rotation(lua_State* lua_state)
        {
            lua_pushnumber(lua_state, get_transform(lua_state)->rotation);
            return 1;
        }

        i32 transform_set_rotation(lua_State* lua_state)
        {
            get_transform(lua_state)->rotation = static_cast<f32>(luaL_checknumber(lua_state, 2));
            return 0;
        }

        i32 transform_get_world_position(lua_State* lua_state)
        {
            return push_vector(lua_state, get_scene(lua_state).get_world_transform_2D(check_existing_entity(lua_state, 1)).position);
        }

        i32 velocity_get(lua_State* lua_state) { return push_vector(lua_state, get_velocity(lua_state)->velocity); }
        i32 velocity_set(lua_State* lua_state) { get_velocity(lua_state)->velocity = check_vector(lua_state, 2); return 0; }

        i32 velocity_get_speed(lua_State* lua_state)
        {
            lua_pushnumber(lua_state, get_velocity(lua_state)->speed);
            return 1;
        }

        i32 velocity_set_speed(lua_State* lua_state)
        {
            get_velocity(lua_state)->speed = static_cast<f32>(luaL_checknumber(lua_state, 2));
            return 0;
        }

        i32 label_get_text(lua_State* lua_state)
        {
            lua_pushstring(lua_state, get_label(lua_state)->text);
            return 1;
        }

        i32 label_set_text(lua_State* lua_state)
        {
            copy_text(get_label(lua_state)->text, luaL_checkstring(lua_state, 2));
            return 0;
        }

        i32 sprite_get_texture(lua_State* lua_state)
        {
            lua_pushstring(lua_state, get_sprite(lua_state)->texture_path);
            return 1;
        }

        i32 sprite_set_texture(lua_State* lua_state)
        {
            copy_text(get_sprite(lua_state)->texture_path, luaL_checkstring(lua_state, 2));
            return 0;
        }

        i32 sprite_get_tint(lua_State* lua_state) { return push_color(lua_state, get_sprite(lua_state)->tint); }
        i32 sprite_set_tint(lua_State* lua_state) { get_sprite(lua_state)->tint = check_color(lua_state, 2); return 0; }
        i32 sprite_get_size(lua_State* lua_state) { return push_vector(lua_state, get_sprite(lua_state)->size); }
        i32 sprite_set_size(lua_State* lua_state) { get_sprite(lua_state)->size = check_vector(lua_state, 2); return 0; }

        i32 camera_get_zoom(lua_State* lua_state)
        {
            lua_pushnumber(lua_state, get_camera(lua_state)->zoom);
            return 1;
        }

        i32 camera_set_zoom(lua_State* lua_state)
        {
            get_camera(lua_state)->zoom = static_cast<f32>(luaL_checknumber(lua_state, 2));
            return 0;
        }

        i32 audio_play(lua_State* lua_state)
        {
            const ecs::entity_t entity = check_entity(lua_state, 1);
            const components::audio_source_component_t* audio_source = require_component(lua_state, get_scene(lua_state).get_audio_source(entity), entity, audio_label);

            lua_pushinteger(lua_state, get_system(lua_state).get_audio_engine().play(audio_source->clip_path, audio_source->volume, audio_source->loop));
            return 1;
        }

        i32 audio_stop(lua_State* lua_state)
        {
            get_system(lua_state).get_audio_engine().stop(static_cast<u32>(luaL_checkinteger(lua_state, 1)));
            return 0;
        }

        i32 input_is_key_down(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_key_down(check_key_code(lua_state)));
            return 1;
        }

        i32 input_is_key_pressed(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_key_pressed(check_key_code(lua_state)));
            return 1;
        }

        i32 input_is_key_released(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_key_released(check_key_code(lua_state)));
            return 1;
        }

        i32 input_is_mouse_down(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_mouse_button_down(check_mouse_button(lua_state)));
            return 1;
        }

        i32 input_is_mouse_pressed(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_mouse_button_pressed(check_mouse_button(lua_state)));
            return 1;
        }

        i32 input_is_mouse_released(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_input_state(lua_state).is_mouse_button_released(check_mouse_button(lua_state)));
            return 1;
        }

        i32 input_get_mouse_position(lua_State* lua_state) { return push_vector(lua_state, get_input_state(lua_state).mouse_position); }
        i32 input_get_mouse_wheel(lua_State* lua_state)    { return push_vector(lua_state, get_input_state(lua_state).mouse_wheel); }

        i32 scene_create_entity(lua_State* lua_state)
        {
            return push_entity(lua_state, get_scene(lua_state).create_entity(luaL_optstring(lua_state, 1, default_entity_name)));
        }

        i32 scene_destroy(lua_State* lua_state)
        {
            get_scene(lua_state).queue_destroy(check_entity(lua_state, 1));
            return 0;
        }

        i32 scene_exists(lua_State* lua_state)
        {
            lua_pushboolean(lua_state, get_scene(lua_state).exists(check_entity(lua_state, 1)));
            return 1;
        }

        i32 scene_find(lua_State* lua_state)
        {
            ecs::entity_t entity;

            if (!get_scene(lua_state).find_by_name(luaL_checkstring(lua_state, 1), entity))
            {
                lua_pushnil(lua_state);
                return 1;
            }

            return push_entity(lua_state, entity);
        }

        i32 scene_find_with_tag(lua_State* lua_state)
        {
            std::vector<ecs::entity_t> entities;
            get_scene(lua_state).find_with_tag(luaL_checkstring(lua_state, 1), entities);

            return push_entities(lua_state, entities);
        }

        i32 scene_has_tag(lua_State* lua_state)
        {
            const components::tags_component_t* tags = get_scene(lua_state).get_tags(check_entity(lua_state, 1));

            lua_pushboolean(lua_state, tags != nullptr && tags->has_tag(luaL_checkstring(lua_state, 2)));
            return 1;
        }

        i32 scene_get_name(lua_State* lua_state)
        {
            const components::name_component_t* name = get_scene(lua_state).get_name(check_existing_entity(lua_state, 1));

            lua_pushstring(lua_state, name->name);
            return 1;
        }

        i32 scene_get_parent(lua_State* lua_state)
        {
            scene_t& scene             = get_scene(lua_state);
            const ecs::entity_t entity = check_existing_entity(lua_state, 1);

            if (!scene.has_parent(entity))
            {
                lua_pushnil(lua_state);
                return 1;
            }

            return push_entity(lua_state, scene.get_parent(entity));
        }

        i32 scene_set_parent(lua_State* lua_state)
        {
            scene_t& scene             = get_scene(lua_state);
            const ecs::entity_t entity = check_existing_entity(lua_state, 1);

            if (lua_isnoneornil(lua_state, 2))
            {
                scene.clear_parent(entity);
                lua_pushboolean(lua_state, true);
                return 1;
            }

            lua_pushboolean(lua_state, scene.set_parent(entity, check_existing_entity(lua_state, 2)));
            return 1;
        }

        i32 scene_get_children(lua_State* lua_state)
        {
            std::vector<ecs::entity_t> children;
            get_scene(lua_state).get_children(check_existing_entity(lua_state, 1), children);

            return push_entities(lua_state, children);
        }

        i32 scene_spawn_stamp(lua_State* lua_state)
        {
            scene_t& scene = get_scene(lua_state);
            ecs::entity_t root;
            std::string error;

            const stamp_loader_t& stamp_loader = get_system(lua_state).stamp_loader;

            if (!stamp_loader)
            {
                LOG_ERROR(stamp_error_format, missing_stamp_loader_error);
                lua_pushnil(lua_state);
                return 1;
            }

            if (!stamp_loader(scene, luaL_checkstring(lua_state, 1), root, error))
            {
                LOG_ERROR(stamp_error_format, error.c_str());
                lua_pushnil(lua_state);
                return 1;
            }

            if (!lua_isnoneornil(lua_state, 2))
            {
                if (components::transform_2D_component_t* transform = scene.get_transform_2D(root))
                {
                    transform->position = check_vector(lua_state, 2);
                }
            }

            return push_entity(lua_state, root);
        }

        i32 scene_load_level(lua_State* lua_state)
        {
            get_system(lua_state).request_level(luaL_checkstring(lua_state, 1));
            return 0;
        }

        i32 time_get_elapsed(lua_State* lua_state)
        {
            lua_pushnumber(lua_state, get_system(lua_state).get_elapsed_time());
            return 1;
        }

        i32 time_get_delta(lua_State* lua_state)
        {
            lua_pushnumber(lua_state, get_system(lua_state).get_delta_time());
            return 1;
        }

        i32 log_info(lua_State* lua_state)
        {
            LOG_INFO(script_log_format, luaL_checkstring(lua_state, 1));
            return 0;
        }

        i32 log_warning(lua_State* lua_state)
        {
            LOG_WARNING(script_log_format, luaL_checkstring(lua_state, 1));
            return 0;
        }

        i32 log_error(lua_State* lua_state)
        {
            LOG_ERROR(script_log_format, luaL_checkstring(lua_state, 1));
            return 0;
        }

        const luaL_Reg transform_functions[] =
        {
            { "get_position",       transform_get_position       },
            { "set_position",       transform_set_position       },
            { "get_world_position", transform_get_world_position },
            { "get_rotation",       transform_get_rotation       },
            { "set_rotation",       transform_set_rotation       },
            { "get_scale",          transform_get_scale          },
            { "set_scale",          transform_set_scale          },
            { nullptr,              nullptr                      }
        };

        const luaL_Reg velocity_functions[] =
        {
            { "get",       velocity_get       },
            { "set",       velocity_set       },
            { "get_speed", velocity_get_speed },
            { "set_speed", velocity_set_speed },
            { nullptr,     nullptr            }
        };

        const luaL_Reg label_functions[] =
        {
            { "get_text", label_get_text },
            { "set_text", label_set_text },
            { nullptr,    nullptr        }
        };

        const luaL_Reg sprite_functions[] =
        {
            { "get_texture", sprite_get_texture },
            { "set_texture", sprite_set_texture },
            { "get_tint",    sprite_get_tint    },
            { "set_tint",    sprite_set_tint    },
            { "get_size",    sprite_get_size    },
            { "set_size",    sprite_set_size    },
            { nullptr,       nullptr            }
        };

        const luaL_Reg camera_functions[] =
        {
            { "get_zoom", camera_get_zoom },
            { "set_zoom", camera_set_zoom },
            { nullptr,    nullptr         }
        };

        const luaL_Reg audio_functions[] =
        {
            { "play",  audio_play },
            { "stop",  audio_stop },
            { nullptr, nullptr    }
        };

        const luaL_Reg input_functions[] =
        {
            { "is_key_down",        input_is_key_down        },
            { "is_key_pressed",     input_is_key_pressed     },
            { "is_key_released",    input_is_key_released    },
            { "is_mouse_down",      input_is_mouse_down      },
            { "is_mouse_pressed",   input_is_mouse_pressed   },
            { "is_mouse_released",  input_is_mouse_released  },
            { "get_mouse_position", input_get_mouse_position },
            { "get_mouse_wheel",    input_get_mouse_wheel    },
            { nullptr,              nullptr                  }
        };

        const luaL_Reg scene_functions[] =
        {
            { "create_entity", scene_create_entity },
            { "destroy",       scene_destroy       },
            { "exists",        scene_exists        },
            { "find",          scene_find          },
            { "find_with_tag", scene_find_with_tag },
            { "has_tag",       scene_has_tag       },
            { "get_name",      scene_get_name      },
            { "get_parent",    scene_get_parent    },
            { "set_parent",    scene_set_parent    },
            { "get_children",  scene_get_children  },
            { "spawn_stamp",  scene_spawn_stamp  },
            { "load_level",    scene_load_level    },
            { nullptr,         nullptr             }
        };

        const luaL_Reg time_functions[] =
        {
            { "get_elapsed", time_get_elapsed },
            { "get_delta",   time_get_delta   },
            { nullptr,       nullptr          }
        };

        const luaL_Reg log_functions[] =
        {
            { "info",    log_info    },
            { "warning", log_warning },
            { "error",   log_error   },
            { nullptr,   nullptr     }
        };

        const char* get_error_message(lua_State* lua_state)
        {
            const char* message = lua_tostring(lua_state, stack_top);
            return message != nullptr ? message : unknown_error_message;
        }

        void register_table(lua_State* lua_state, const char* table_name, const luaL_Reg* functions, scene_script_system_t* system)
        {
            lua_newtable(lua_state);
            lua_pushlightuserdata(lua_state, system);
            luaL_setfuncs(lua_state, functions, 1);
            lua_setglobal(lua_state, table_name);
        }
    }

    bool scene_script_system_t::initialize(scripting::scripting_context_t& scripting_context, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state)
    {
        this->lua_state    = scripting_context.get_lua_state();
        this->scene        = &scene;
        this->audio_engine = &audio_engine;
        this->input_state  = &input_state;

        register_api();

        return true;
    }

    void scene_script_system_t::start(const std::vector<ecs::entity_t>& entities)
    {
        stop();

        running = true;

        for (usize index = 0; index < entities.size(); index++)
        {
            start_entity(entities[index]);
        }
    }

    void scene_script_system_t::update(const std::vector<ecs::entity_t>& entities, const f64 frame_delta_time)
    {
        if (!running)
        {
            return;
        }

        delta_time    = frame_delta_time;
        elapsed_time += frame_delta_time;

        for (usize index = 0; index < entities.size(); index++)
        {
            const ecs::entity_t entity = entities[index];

            if (entity_environments.find(entity.id) == entity_environments.end() && !start_entity(entity))
            {
                continue;
            }

            if (push_function(entity, update_function))
            {
                lua_pushinteger(lua_state, static_cast<lua_Integer>(entity.id));
                lua_pushnumber(lua_state, frame_delta_time);
                invoke(entity, update_function, 2);
            }
        }
    }

    void scene_script_system_t::notify_collision(const ecs::entity_t entity, const ecs::entity_t other)
    {
        if (running && push_function(entity, collision_function))
        {
            lua_pushinteger(lua_state, static_cast<lua_Integer>(entity.id));
            lua_pushinteger(lua_state, static_cast<lua_Integer>(other.id));
            invoke(entity, collision_function, 2);
        }
    }

    void scene_script_system_t::release_entities(const std::vector<ecs::entity_t>& entities)
    {
        for (const ecs::entity_t entity : entities)
        {
            std::unordered_map<u64, i32>::iterator iterator = entity_environments.find(entity.id);

            if (iterator != entity_environments.end())
            {
                luaL_unref(lua_state, LUA_REGISTRYINDEX, iterator->second);
                entity_environments.erase(iterator);
            }

            failed_entities.erase(entity.id);
        }
    }

    void scene_script_system_t::request_level(const char* level_path)
    {
        requested_level_path = level_path;
    }

    bool scene_script_system_t::take_level_request(std::string& out_level_path)
    {
        if (requested_level_path.empty())
        {
            return false;
        }

        out_level_path = requested_level_path;
        requested_level_path.clear();

        return true;
    }

    void scene_script_system_t::stop()
    {
        for (std::pair<const u64, i32>& entry : entity_environments)
        {
            luaL_unref(lua_state, LUA_REGISTRYINDEX, entry.second);
        }

        entity_environments.clear();
        failed_entities.clear();
        requested_level_path.clear();

        running      = false;
        elapsed_time = 0.0;
        delta_time   = 0.0;
    }

    void scene_script_system_t::dispose()
    {
        if (lua_state != nullptr)
        {
            stop();
        }

        lua_state    = nullptr;
        scene        = nullptr;
        audio_engine = nullptr;
        input_state  = nullptr;
    }

    bool scene_script_system_t::start_entity(const ecs::entity_t entity)
    {
        if (failed_entities.find(entity.id) != failed_entities.end())
        {
            return false;
        }

        const components::script_component_t* script = scene->get_script(entity);

        if (script == nullptr || script->script_path[0] == '\0' || !load_script(entity, script->script_path))
        {
            failed_entities.insert(entity.id);
            return false;
        }

        if (push_function(entity, start_function))
        {
            lua_pushinteger(lua_state, static_cast<lua_Integer>(entity.id));
            invoke(entity, start_function, 1);
        }

        return true;
    }

    bool scene_script_system_t::load_script(const ecs::entity_t entity, const char* script_path)
    {
        if (luaL_loadfile(lua_state, script_path) != LUA_OK)
        {
            LOG_ERROR(load_error_format, script_path, static_cast<unsigned long long>(entity.id), get_error_message(lua_state));
            lua_pop(lua_state, 1);
            return false;
        }

        lua_newtable(lua_state);
        lua_newtable(lua_state);
        lua_getglobal(lua_state, global_table_name);
        lua_setfield(lua_state, -2, index_metafield);
        lua_setmetatable(lua_state, -2);

        lua_pushvalue(lua_state, -1);
        const i32 environment_reference = luaL_ref(lua_state, LUA_REGISTRYINDEX);

        lua_setupvalue(lua_state, -2, environment_upvalue);

        if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
        {
            LOG_ERROR(load_error_format, script_path, static_cast<unsigned long long>(entity.id), get_error_message(lua_state));
            lua_pop(lua_state, 1);
            luaL_unref(lua_state, LUA_REGISTRYINDEX, environment_reference);
            return false;
        }

        entity_environments[entity.id] = environment_reference;

        return true;
    }

    bool scene_script_system_t::push_function(const ecs::entity_t entity, const char* function_name)
    {
        std::unordered_map<u64, i32>::iterator iterator = entity_environments.find(entity.id);

        if (iterator == entity_environments.end())
        {
            return false;
        }

        lua_rawgeti(lua_state, LUA_REGISTRYINDEX, iterator->second);
        lua_getfield(lua_state, -1, function_name);

        if (!lua_isfunction(lua_state, stack_top))
        {
            lua_pop(lua_state, 2);
            return false;
        }

        return true;
    }

    void scene_script_system_t::invoke(const ecs::entity_t entity, const char* function_name, const i32 argument_count)
    {
        if (lua_pcall(lua_state, argument_count, 0, 0) != LUA_OK)
        {
            LOG_ERROR(call_error_format, function_name, static_cast<unsigned long long>(entity.id), get_error_message(lua_state));
            lua_pop(lua_state, 1);
        }

        lua_pop(lua_state, 1);
    }

    void scene_script_system_t::register_api()
    {
        register_table(lua_state, "transform", transform_functions, this);
        register_table(lua_state, "velocity", velocity_functions, this);
        register_table(lua_state, "label", label_functions, this);
        register_table(lua_state, "sprite", sprite_functions, this);
        register_table(lua_state, "camera", camera_functions, this);
        register_table(lua_state, "audio", audio_functions, this);
        register_table(lua_state, "input", input_functions, this);
        register_table(lua_state, "scene", scene_functions, this);
        register_table(lua_state, "time", time_functions, this);
        register_table(lua_state, "log", log_functions, this);
    }
}
