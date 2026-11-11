#pragma once

#include <byte-forge-engine/modules/scripting/scripting_api.h>
#include <byte-forge-engine/api/types.h>

struct lua_State;

namespace byteforge::scripting
{
    class BF_SCRIPTING_API scripting_context_t
    {
    public:
        scripting_context_t() : lua_state(nullptr) {}

        bool initialize();
        void dispose();

        bool load_file(const char* file_path);
        bool call(const char* function_name);

        bool push_table(const char* table_name);
        void pop_table();

        bool get_integer(const char* field, i32& out_integer);
        bool get_number(const char* field, f64& out_number);
        bool get_string(const char* field, char* out_buffer, usize buffer_size);

        lua_State* get_lua_state() const { return lua_state; }
        usize get_memory_bytes() const;

    private:
        lua_State* lua_state;
    };
}
