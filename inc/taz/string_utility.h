#pragma once

#include <string>
#include <string_view>

namespace jtasler::string_utility
{
    inline std::string narrow(std::wstring_view wideText)
    {
        std::string multibyteText;
        int32_t byteCount = ::WideCharToMultiByte(CP_UTF8, 0,
            wideText.data(), static_cast<int32_t>(wideText.length()),
            nullptr, 0,
            nullptr, nullptr);

        multibyteText.resize(byteCount);
        ::WideCharToMultiByte(CP_UTF8, 0,
            wideText.data(), static_cast<int32_t>(wideText.length()),
            multibyteText.data(), static_cast<int32_t>(multibyteText.length()),
            nullptr, nullptr);

        return multibyteText;
    }

    inline std::wstring widen(std::string_view multibylteText)
    {
        std::wstring wideText;
        int32_t byteCount = ::MultiByteToWideChar(CP_UTF8, 0,
            multibylteText.data(), static_cast<int32_t>(multibylteText.length()),
            nullptr, 0);

        wideText.resize(byteCount);
        ::MultiByteToWideChar(CP_UTF8, 0,
            multibylteText.data(), static_cast<int32_t>(multibylteText.length()),
            wideText.data(), static_cast<int32_t>(wideText.length()));

        return wideText;
    }

    inline std::string narrow(wchar_t wideChar)
    {
        std::wstring wideText(1, wideChar);
        return narrow(std::wstring_view(wideText));
    }

    inline std::wstring widen(char multibyteChar)
    {
        std::string multibyteText(1, multibyteChar);
        return widen(std::string_view(multibyteText));
    }

    template <typename _Elem, typename _Traits = std::char_traits<_Elem>, typename _Alloc = std::allocator<_Elem>>
    std::basic_string<_Elem, _Traits, _Alloc>
    replace_all(
        std::basic_string_view<_Elem> text,
        const std::vector<std::pair<std::basic_string_view<_Elem>, std::basic_string_view<_Elem>>>& replacements)
    {
        // Note that this could be optimized to not modify in-place
        std::basic_string<_Elem, _Traits, _Alloc> result(text);

        for (const auto& [from, to] : replacements)
        {
            for (std::size_t position{}, found{}; found != std::string::npos; )
            {
                if (found = result.find(from, position); found != std::string::npos)
                {
                    result.replace(result.begin() + found, result.begin() + found + from.length(), to);
                    position += to.size();
                }
            }
        }

        return result;
    }

    template <typename _Elem, typename _Traits = std::char_traits<_Elem>, typename _Alloc = std::allocator<_Elem>>
    std::basic_string<_Elem, _Traits, _Alloc>
    replace_all(
        std::basic_string<_Elem, _Traits, _Alloc>& text,
        const std::vector<std::pair<std::basic_string_view<_Elem>, std::basic_string_view<_Elem>>>& replacements)
    {
        return replace_all(std::basic_string_view<_Elem>(text), replacements);
    }

    struct ordinal_ignore_case_less
    {
        bool operator()(const std::wstring_view& left, const std::wstring_view& right) const
        {
            return CSTR_LESS_THAN == CompareStringOrdinal(
                left.data(), static_cast<int32_t>(left.length()),
                right.data(), static_cast<int32_t>(right.length()), true);
        }

        bool operator()(const std::wstring& left, const std::wstring& right) const
        {
            return (*this)(std::wstring_view(left), std::wstring_view(right));
        }
    };
}
