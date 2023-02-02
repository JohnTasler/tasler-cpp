#pragma once

#include <string_view>

namespace jtasler::resource_loader
{
    using namespace std::literals;
    constexpr auto c_AnsiText = L"ANSI_TEXT"sv;

    namespace
    {
        inline void* find_and_load_core(HMODULE module, PCWSTR id, PCWSTR type, uint16_t /*lang*/, _Out_ uint32_t* byteCount)
        {
            auto resourceInformationBlock = ::FindResourceW(module, id, type);
            THROW_LAST_ERROR_IF_NULL(resourceInformationBlock);

            auto resource = ::LoadResource(module, resourceInformationBlock);
            THROW_LAST_ERROR_IF_NULL(resource);

            auto data = ::LockResource(resource);
            THROW_LAST_ERROR_IF_NULL(data);

            if (byteCount)
            {
                *byteCount = ::SizeofResource(module, resourceInformationBlock);
            }

            return data;
        }
    }

    template<typename T>
    T find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
    {
        static_assert("T is a non-supported type.");
    }

    //template<> inline std::span<uint8_t> find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
    //{
    //    uint32_t byteCount{};
    //    const void* data = find_and_load_core(module, id, type, 0, &byteCount);
    //    const auto begin = reinterpret_cast<const uint8_t*>(data);
    //    return std::span<uint8_t>(begin, begin + byteCount);
    //}

    template<> inline std::string_view find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
    {
        uint32_t byteCount{};
        const void* data = find_and_load_core(module, id, type, 0, &byteCount);
        const auto begin = reinterpret_cast<const char*>(data);
        return { begin, byteCount };
    }

    template<> inline std::wstring_view find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
    {
        uint32_t byteCount{};
        const void* data = find_and_load_core(module, id, type, 0, &byteCount);
        const auto begin = reinterpret_cast<const wchar_t*>(data);
        return { begin, byteCount / sizeof(wchar_t) };
    }

    template<typename T>
    T find_and_load(HMODULE module, uint16_t id, PCWSTR type)
    {
        static_assert("T is a non-supported type.");
    }
    //template<> inline std::span<uint8_t> find_and_load(HMODULE module, uint16_t id, PCWSTR type)
    //{
    //    return find_and_load<std::span<uint8_t>>(module, MAKEINTRESOURCE(id), type);
    //}
    template<> inline std::string_view find_and_load(HMODULE module, uint16_t id, PCWSTR type)
    {
        return find_and_load<std::string_view>(module, MAKEINTRESOURCE(id), type);
    }
    template<> inline std::wstring_view find_and_load(HMODULE module, uint16_t id, PCWSTR type)
    {
        return find_and_load<std::wstring_view>(module, MAKEINTRESOURCE(id), type);
    }

    template<typename T>
    T find_and_load(HMODULE module, uint16_t id, uint16_t type)
    {
        static_assert("T is a non-supported type.");
    }
    //template<> inline std::span<uint8_t> find_and_load(HMODULE module, uint16_t id, uint16_t type)
    //{
    //    return find_and_load<std::span<uint8_t>>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
    //}
    template<> inline std::string_view find_and_load(HMODULE module, uint16_t id, uint16_t type)
    {
        return find_and_load<std::string_view>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
    }
    template<> inline std::wstring_view find_and_load(HMODULE module, uint16_t id, uint16_t type)
    {
        return find_and_load<std::wstring_view>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
    }

    template<typename T>
    T find_and_load(HMODULE module, PCWSTR id, uint16_t type)
    {
        static_assert("T is a non-supported type.");
    }
    //template<> inline std::span<uint8_t> find_and_load(HMODULE module, PCWSTR id, uint16_t type)
    //{
    //    return find_and_load<std::span<uint8_t>>(module, id, MAKEINTRESOURCE(type));
    //}
    template<> inline std::string_view find_and_load(HMODULE module, PCWSTR id, uint16_t type)
    {
        return find_and_load<std::string_view>(module, id, MAKEINTRESOURCE(type));
    }
    template<> inline std::wstring_view find_and_load(HMODULE module, PCWSTR id, uint16_t type)
    {
        return find_and_load<std::wstring_view>(module, id, MAKEINTRESOURCE(type));
    }
}
