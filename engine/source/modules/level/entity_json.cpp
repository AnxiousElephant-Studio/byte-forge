#include "entity_json.h"

#include <cstdio>
#include <optional>
#include <unordered_map>

namespace byteforge::level
{
    namespace
    {
        const i64 no_parent_index = -1;

        const char* const name_key         = "name";
        const char* const parent_key       = "parent";
        const char* const tags_key         = "tags";
        const char* const transform_key    = "transform_2D";
        const char* const sprite_key       = "sprite_2D";
        const char* const script_key       = "script";
        const char* const velocity_key     = "velocity_2D";
        const char* const box_collider_key = "box_collider_2D";
        const char* const audio_source_key = "audio_source";
        const char* const label_key        = "label_2D";
        const char* const camera_key       = "camera_2D";
        const char* const point_light_key  = "point_light_2D";

        struct entity_data_t
        {
            std::string name;
            i64 parent_index = no_parent_index;
            std::optional<components::tags_component_t> tags;
            std::optional<components::transform_2D_component_t> transform;
            std::optional<components::sprite_2D_component_t> sprite;
            std::optional<components::script_component_t> script;
            std::optional<components::velocity_2D_component_t> velocity;
            std::optional<components::box_collider_2D_component_t> box_collider;
            std::optional<components::audio_source_component_t> audio_source;
            std::optional<components::label_2D_component_t> label;
            std::optional<components::camera_2D_component_t> camera;
            std::optional<components::point_light_2D_component_t> point_light;
        };

        json_t to_json(const math::vec2_t& vector)
        {
            return json_t::array({ vector.x, vector.y });
        }

        json_t to_json(const math::vec4_t& vector)
        {
            return json_t::array({ vector.x, vector.y, vector.z, vector.w });
        }

        math::vec2_t read_vec2(const json_t& json)
        {
            return math::vec2_t(json.at(0).get<f32>(), json.at(1).get<f32>());
        }

        math::vec4_t read_vec4(const json_t& json)
        {
            return math::vec4_t(json.at(0).get<f32>(), json.at(1).get<f32>(), json.at(2).get<f32>(), json.at(3).get<f32>());
        }

        template <usize size>
        void read_text(const json_t& json, const char* key, char (&buffer)[size])
        {
            std::snprintf(buffer, size, "%s", json.at(key).get<std::string>().c_str());
        }

        json_t entity_to_json(scene::scene_t& scene, const ecs::entity_t entity, const std::unordered_map<u64, i64>& entity_indices)
        {
            json_t json = json_t::object();

            if (const components::name_component_t* name = scene.get_name(entity))
            {
                json[name_key] = name->name;
            }

            if (scene.has_parent(entity))
            {
                const std::unordered_map<u64, i64>::const_iterator parent = entity_indices.find(scene.get_parent(entity).id);

                if (parent != entity_indices.end())
                {
                    json[parent_key] = parent->second;
                }
            }

            if (const components::tags_component_t* tags = scene.get_tags(entity))
            {
                json[tags_key] = tags->tags;
            }

            if (const components::transform_2D_component_t* transform = scene.get_transform_2D(entity))
            {
                json[transform_key] = { { "position", to_json(transform->position) }, { "scale", to_json(transform->scale) }, { "rotation", transform->rotation } };
            }

            if (const components::sprite_2D_component_t* sprite = scene.get_sprite_2D(entity))
            {
                json[sprite_key] = { { "texture_path", sprite->texture_path }, { "size", to_json(sprite->size) }, { "tint", to_json(sprite->tint) }, { "material_path", sprite->material_path }, { "is_lit", sprite->is_lit } };
            }

            if (const components::script_component_t* script = scene.get_script(entity))
            {
                json[script_key] = { { "script_path", script->script_path } };
            }

            if (const components::velocity_2D_component_t* velocity = scene.get_velocity_2D(entity))
            {
                json[velocity_key] = { { "velocity", to_json(velocity->velocity) }, { "speed", velocity->speed } };
            }

            if (const components::box_collider_2D_component_t* box_collider = scene.get_box_collider_2D(entity))
            {
                json[box_collider_key] = { { "offset", to_json(box_collider->offset) }, { "size", to_json(box_collider->size) } };
            }

            if (const components::audio_source_component_t* audio_source = scene.get_audio_source(entity))
            {
                json[audio_source_key] = { { "clip_path", audio_source->clip_path }, { "volume", audio_source->volume }, { "loop", audio_source->loop }, { "play_on_start", audio_source->play_on_start } };
            }

            if (const components::label_2D_component_t* label = scene.get_label_2D(entity))
            {
                json[label_key] = { { "text", label->text }, { "font_path", label->font_path }, { "font_size", label->font_size }, { "color", to_json(label->color) } };
            }

            if (const components::camera_2D_component_t* camera = scene.get_camera_2D(entity))
            {
                json[camera_key] = { { "zoom", camera->zoom }, { "is_primary", camera->is_primary } };
            }

            if (const components::point_light_2D_component_t* point_light = scene.get_point_light_2D(entity))
            {
                json[point_light_key] = { { "color", to_json(point_light->color) }, { "radius", point_light->radius }, { "intensity", point_light->intensity } };
            }

            return json;
        }

        entity_data_t read_entity(const json_t& json)
        {
            entity_data_t entity;

            entity.name         = json.at(name_key).get<std::string>();
            entity.parent_index = json.value(parent_key, no_parent_index);

            if (json.contains(tags_key))
            {
                components::tags_component_t tags = {};
                read_text(json, tags_key, tags.tags);
                entity.tags = tags;
            }

            if (json.contains(transform_key))
            {
                const json_t& data = json.at(transform_key);

                components::transform_2D_component_t transform = {};
                transform.position = read_vec2(data.at("position"));
                transform.scale    = read_vec2(data.at("scale"));
                transform.rotation = data.at("rotation").get<f32>();
                entity.transform   = transform;
            }

            if (json.contains(sprite_key))
            {
                const json_t& data = json.at(sprite_key);

                components::sprite_2D_component_t sprite = {};
                read_text(data, "texture_path", sprite.texture_path);
                sprite.size = read_vec2(data.at("size"));
                sprite.tint = read_vec4(data.at("tint"));

                if (data.contains("material_path"))
                {
                    read_text(data, "material_path", sprite.material_path);
                }

                sprite.is_lit = data.value("is_lit", false);
                entity.sprite = sprite;
            }

            if (json.contains(script_key))
            {
                components::script_component_t script = {};
                read_text(json.at(script_key), "script_path", script.script_path);
                entity.script = script;
            }

            if (json.contains(velocity_key))
            {
                const json_t& data = json.at(velocity_key);

                components::velocity_2D_component_t velocity = {};
                velocity.velocity = read_vec2(data.at("velocity"));
                velocity.speed    = data.at("speed").get<f32>();
                entity.velocity   = velocity;
            }

            if (json.contains(box_collider_key))
            {
                const json_t& data = json.at(box_collider_key);

                components::box_collider_2D_component_t box_collider = {};
                box_collider.offset = read_vec2(data.at("offset"));
                box_collider.size   = read_vec2(data.at("size"));
                entity.box_collider = box_collider;
            }

            if (json.contains(audio_source_key))
            {
                const json_t& data = json.at(audio_source_key);

                components::audio_source_component_t audio_source = {};
                read_text(data, "clip_path", audio_source.clip_path);
                audio_source.volume        = data.at("volume").get<f32>();
                audio_source.loop          = data.at("loop").get<bool>();
                audio_source.play_on_start = data.at("play_on_start").get<bool>();
                entity.audio_source        = audio_source;
            }

            if (json.contains(label_key))
            {
                const json_t& data = json.at(label_key);

                components::label_2D_component_t label = {};
                read_text(data, "text", label.text);
                read_text(data, "font_path", label.font_path);
                label.font_size = data.at("font_size").get<f32>();
                label.color     = read_vec4(data.at("color"));
                entity.label    = label;
            }

            if (json.contains(camera_key))
            {
                const json_t& data = json.at(camera_key);

                components::camera_2D_component_t camera = {};
                camera.zoom       = data.at("zoom").get<f32>();
                camera.is_primary = data.at("is_primary").get<bool>();
                entity.camera     = camera;
            }

            if (json.contains(point_light_key))
            {
                const json_t& data = json.at(point_light_key);

                components::point_light_2D_component_t point_light = {};
                point_light.color     = read_vec4(data.at("color"));
                point_light.radius    = data.at("radius").get<f32>();
                point_light.intensity = data.at("intensity").get<f32>();
                entity.point_light    = point_light;
            }

            return entity;
        }

        ecs::entity_t create_entity(scene::scene_t& scene, const entity_data_t& data)
        {
            const ecs::entity_t entity = scene.create_empty_entity(data.name.c_str());

            if (data.tags.has_value())
            {
                scene.add_tags(entity);
                *scene.get_tags(entity) = data.tags.value();
            }

            if (data.transform.has_value())
            {
                scene.add_transform_2D(entity);
                *scene.get_transform_2D(entity) = data.transform.value();
            }

            if (data.sprite.has_value())
            {
                scene.add_sprite_2D(entity);
                *scene.get_sprite_2D(entity) = data.sprite.value();
            }

            if (data.script.has_value())
            {
                scene.add_script(entity);
                *scene.get_script(entity) = data.script.value();
            }

            if (data.velocity.has_value())
            {
                scene.add_velocity_2D(entity);
                *scene.get_velocity_2D(entity) = data.velocity.value();
            }

            if (data.box_collider.has_value())
            {
                scene.add_box_collider_2D(entity);
                *scene.get_box_collider_2D(entity) = data.box_collider.value();
            }

            if (data.audio_source.has_value())
            {
                scene.add_audio_source(entity);
                *scene.get_audio_source(entity) = data.audio_source.value();
            }

            if (data.label.has_value())
            {
                scene.add_label_2D(entity);
                *scene.get_label_2D(entity) = data.label.value();
            }

            if (data.camera.has_value())
            {
                scene.add_camera_2D(entity);
                *scene.get_camera_2D(entity) = data.camera.value();
            }

            if (data.point_light.has_value())
            {
                scene.add_point_light_2D(entity);
                *scene.get_point_light_2D(entity) = data.point_light.value();
            }

            return entity;
        }
    }

    json_t entity_json_t::serialize(scene::scene_t& scene, const std::vector<ecs::entity_t>& entities)
    {
        std::unordered_map<u64, i64> entity_indices;

        for (usize index = 0; index < entities.size(); index++)
        {
            entity_indices[entities[index].id] = static_cast<i64>(index);
        }

        json_t entities_json = json_t::array();

        for (const ecs::entity_t entity : entities)
        {
            entities_json.push_back(entity_to_json(scene, entity, entity_indices));
        }

        return entities_json;
    }

    bool entity_json_t::deserialize(scene::scene_t& scene, const json_t& entities_json, std::vector<ecs::entity_t>& out_created_entities, std::string& out_error)
    {
        std::vector<entity_data_t> entities;

        try
        {
            for (const json_t& entity_json : entities_json)
            {
                entities.push_back(read_entity(entity_json));
            }
        }
        catch (const nlohmann::json::exception& exception)
        {
            out_error = exception.what();
            return false;
        }

        const usize first_created = out_created_entities.size();

        for (const entity_data_t& entity : entities)
        {
            out_created_entities.push_back(create_entity(scene, entity));
        }

        for (usize index = 0; index < entities.size(); index++)
        {
            const i64 parent_index = entities[index].parent_index;

            if (parent_index >= 0 && static_cast<usize>(parent_index) < entities.size())
            {
                scene.attach_to_parent(out_created_entities[first_created + index], out_created_entities[first_created + static_cast<usize>(parent_index)]);
            }
        }

        return true;
    }
}
