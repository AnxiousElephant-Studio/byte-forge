#include <byte-forge-engine/modules/scripting/scripting_context.h>

#include <byte-forge-engine/modules/logging/log_macros.h>

#include <lua.hpp>

#include <cstdio>

namespace byteforge::scripting
{
    namespace
    {
        const i32 stack_top = -1;

        const usize kilobyte_size = 1024;

        const char* const unknown_error_message         = "unknown error";
        const char* const state_creation_error_format   = "Could not create the Lua state. Cause: out of memory.";
        const char* const file_error_format             = "Could not run script at path: '%s'. Cause: %s";
        const char* const missing_function_error_format = "Could not find Lua function: '%s'. Cause: the function does not exist.";
        const char* const call_error_format             = "Could not call Lua function: '%s'. Cause: %s";

        const char* get_error_message(lua_State* lua_state)
        {
            const char* message = lua_tostring(lua_state, stack_top);

            return message != nullptr ? message : unknown_error_message;
        }
    }

    bool scripting_context_t::initialize()
    {
        lua_state = luaL_newstate();

        if (lua_state == nullptr)
        {
            LOG_FATAL(state_creation_error_format);
            return false;
        }

        luaL_openlibs(lua_state);

        return true;
    }

    bool scripting_context_t::load_file(const char* file_path)
    {
        if (luaL_dofile(lua_state, file_path) != LUA_OK)
        {
            LOG_ERROR(file_error_format, file_path, get_error_message(lua_state));
            lua_pop(lua_state, 1);

            return false;
        }

        return true;
    }

    bool scripting_context_t::call(const char* function_name)
    {
        lua_getglobal(lua_state, function_name);

        if (!lua_isfunction(lua_state, stack_top))
        {
            LOG_ERROR(missing_function_error_format, function_name);
            lua_pop(lua_state, 1);

            return false;
        }

        if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
        {
            LOG_ERROR(call_error_format, function_name, get_error_message(lua_state));
            lua_pop(lua_state, 1);

            return false;
        }

        return true;
    }

    bool scripting_context_t::push_table(const char* table_name)
    {
        lua_getglobal(lua_state, table_name);

        if (!lua_istable(lua_state, stack_top))
        {
            lua_pop(lua_state, 1);
            return false;
        }

        return true;
    }

    void scripting_context_t::pop_table()
    {
        if (lua_istable(lua_state, stack_top))
        {
            lua_pop(lua_state, 1);
        }
    }

    bool scripting_context_t::get_integer(const char* field, i32& out_integer)
    {
        lua_getfield(lua_state, stack_top, field);

        bool found = lua_isinteger(lua_state, stack_top) != 0;

        if (found)
        {
            out_integer = static_cast<i32>(lua_tointeger(lua_state, stack_top));
        }

        lua_pop(lua_state, 1);

        return found;
    }

    bool scripting_context_t::get_number(const char* field, f64& out_number)
    {
        lua_getfield(lua_state, stack_top, field);

        bool found = lua_isnumber(lua_state, stack_top) != 0;

        if (found)
        {
            out_number = lua_tonumber(lua_state, stack_top);
        }

        lua_pop(lua_state, 1);

        return found;
    }

    bool scripting_context_t::get_string(const char* field, char* out_buffer, usize buffer_size)
    {
        lua_getfield(lua_state, stack_top, field);

        bool found = lua_type(lua_state, stack_top) == LUA_TSTRING;

        if (found)
        {
            std::snprintf(out_buffer, buffer_size, "%s", lua_tostring(lua_state, stack_top));
        }

        lua_pop(lua_state, 1);

        return found;
    }

    usize scripting_context_t::get_memory_bytes() const
    {
        if (lua_state == nullptr)
        {
            return 0;
        }

        return static_cast<usize>(lua_gc(lua_state, LUA_GCCOUNT)) * kilobyte_size + static_cast<usize>(lua_gc(lua_state, LUA_GCCOUNTB));
    }

    void scripting_context_t::dispose()
    {
        if (lua_state != nullptr)
        {
            lua_close(lua_state);
            lua_state = nullptr;
        }
    }
}
