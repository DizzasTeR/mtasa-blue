/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/
#pragma once
#include <optional>
#include <variant>
#include <array>

/*
    Basic Lua operations:
        void Push(L, T value)
        T PopPrimitive(L, std::size_t stackIndex)
*/

class CVector2D;
class CVector;
class CVector4D;

namespace lua
{
    // PopTrival should read a simple value of type T from the stack without extra type checks
    // If whatever is at that point in the stack is not convertible to T, the behavior is undefined
    template <typename T>
    inline T PopPrimitive(lua_State* L, std::size_t& index);

    // Push should push a value of type T to the Lua Stack
    // This will always increase the stack size by 1
    inline void Push(lua_State* L, int value)
    {
        lua_pushnumber(L, value);
    }
    inline void Push(lua_State* L, unsigned int value)
    {
        lua_pushnumber(L, value);
    }
    inline void Push(lua_State* L, float value)
    {
        lua_pushnumber(L, value);
    }
    inline void Push(lua_State* L, double value)
    {
        lua_pushnumber(L, value);
    }

    inline void Push(lua_State* L, bool value)
    {
        lua_pushboolean(L, value);
    }

    inline void Push(lua_State* L, nullptr_t)
    {
        lua_pushnil(L);
    }

    inline void Push(lua_State* L, const std::string& value)
    {
        lua_pushlstring(L, value.data(), value.length());
    }

    inline void Push(lua_State* L, const CLuaArgument& arg)
    {
        if (arg.GetType() == LUA_TNONE)
        {
            // Calling lua::Push with a LUA_TNONE type value is not allowed, since this is rather error-prone
            // as many callers would not check the stack position after pushing.
            // Hence throw & abort here and let developers fix it.
            throw std::logic_error("lua::Push");
        }
        
        arg.Push(L);
    }

    inline void Push(lua_State* L, const CLuaArguments& args)
    {
        args.PushAsTable(L);
    }

    inline void Push(lua_State* L, const CVector2D& value)
    {
        lua_pushvector(L, value);
    }

    inline void Push(lua_State* L, const CVector& value)
    {
        lua_pushvector(L, value);
    }

    inline void  Push(lua_State* L, const CVector4D& value)
    {
        lua_pushvector(L, value);
    }

    inline void Push(lua_State* L, const CMatrix& value)
    {
        lua_pushmatrix(L, value);
    }

    // Overload for enum types only
    template <typename T>
    typename std::enable_if_t<std::is_enum_v<T>> Push(lua_State* L, const T& val)
    {
        // Push<string> must be defined before this function, otherwise it wont compile
        Push(L, EnumToString(val));
    }

    // Overload for pointers to classes. We boldly assume that these are script entities
    template <typename T>
    std::enable_if_t<std::is_class_v<T>> Push(lua_State* L, T* val)
    {
        lua_pushelement(L, val);
    }

    template <typename T>
    void Push(lua_State* L, const std::shared_ptr<T>& ptr)
    {
        lua_pushelement(L, ptr.get());
    }

    template <typename T>
    void Push(lua_State* L, const std::unique_ptr<T>& ptr)
    {
        lua_pushelement(L, ptr.get());
    }

    /*****************************************************************\
    * The functions below may depend on each other, so they need to be
    * forward declared.
    * Please declare functions that call `Push` after this line.
    \*****************************************************************/

    template <typename... Ts>
    void Push(lua_State* L, const std::variant<Ts...>& val);

    template <typename T>
    void Push(lua_State* L, const std::optional<T>& val);

    template <typename T, size_t N>
    void Push(lua_State* L, const std::array<T, N>& val);

    template <typename T>
    void Push(lua_State* L, const std::vector<T>& val);

    template <typename K, typename V>
    void Push(lua_State* L, const std::unordered_map<K, V>& val);

    template<typename... Ts>
    void Push(lua_State* L, const std::tuple<Ts...>& tuple);

    // Define after this line, declare above.

    template <typename... Ts>
    void Push(lua_State* L, const std::variant<Ts...>& val)
    {
        std::visit([L](const auto& value) { Push(L, value); }, val);
    }

    template <typename T>
    void Push(lua_State* L, const std::optional<T>& val)
    {
        if (val.has_value())
            Push(L, val.value());
        else
            Push(L, nullptr);
    }

    template <typename T, size_t N>
    void Push(lua_State* L, const std::array<T, N>& val)
    {
        lua_createtable(L, N, 0);
        lua_Number i = 1;
        for (const auto& v : val)
        {
            Push(L, v);
            lua_rawseti(L, -2, i++);
        }
    }

    template <typename T>
    void Push(lua_State* L, const std::vector<T>& val)
    {
        lua_newtable(L);
        int i = 1;
        for (const auto& v : val)
        {
            Push(L, i++);
            Push(L, v);
            lua_settable(L, -3);
        }
    }

    template <typename K, typename V>
    void Push(lua_State* L, const std::unordered_map<K, V>& val)
    {
        lua_newtable(L);
        for (const auto& [k, v] : val)
        {
            Push(L, k);
            Push(L, v);
            lua_settable(L, -3);
        }
    }

    // Tuples can be used to push fixed-size tables
    // e.g. `std::tuple<float, int, bool>` will be pushed as { float, int, bool }
    template<typename... Ts>
    void Push(lua_State* L, const std::tuple<Ts...>& tuple)
    {
        // Call Push on each element of the tuple
        lua_createtable(L, sizeof...(Ts), 0);
        std::apply([L](const auto&... values) {
            int  key = 1;
            auto PushTable = [](lua_State* L, int& key, const auto& value)
            {
                Push(L, value);
                lua_rawseti(L, -2, key++);
            };

            (PushTable(L, key, values), ...);
        }, tuple);
    }
}
