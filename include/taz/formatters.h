#pragma once

// Windows headers
#include <stringapiset.h>
#include <wtypes.h>

// Standard C++ headers
#include <format>
#include <string>

// Tasler C++ headers
#include "string_utility.h"

namespace taz::details
{
	inline std::string GetWindowTextString(HWND hwnd)
	{
		auto windowTextLength = GetWindowTextLength(hwnd);
		std::string windowText(windowTextLength + 1, '\0');
		GetWindowTextA(hwnd, windowText.data(), static_cast<int>(windowText.size()));
		windowText.resize(windowTextLength);
		return windowText;
	}

	inline std::string GetWindowClassNameString(HWND hwnd)
	{
		std::string className(256, '\0');
		auto classNameLength = GetClassNameA(hwnd, className.data(), static_cast<int32_t>(className.size()));
		className.resize(classNameLength);
		return className;
	}

	template <std::convertible_to<const wchar_t*> _Ty>
	struct wide_to_narrow_formatter : std::_Formatter_base<_Ty, char, std::_Basic_format_arg_type::_Custom_type>
	{
		constexpr auto parse(std::format_parse_context& ctx)
		{
			return ctx.begin();
		}

		auto format(_Ty input, std::format_context& ctx)
		{
			auto output = taz::string_utility::narrow(input);

			// Write it out
			return std::copy(begin(output), end(output), ctx.out());
		}
	};

	template <std::convertible_to<const char*> _Ty>
	struct narrow_to_wide_formatter : std::_Formatter_base<_Ty, wchar_t, std::_Basic_format_arg_type::_Custom_type>
	{
		constexpr auto parse(std::wformat_parse_context& ctx)
		{
			return ctx.begin();
		}

		auto format(_Ty input, std::wformat_context& ctx)
		{
			auto output = taz::string_utility::widen(input);

			// Write it out
			return std::copy(begin(output), end(output), ctx.out());
		}
	};
}

namespace std
{
	template <>
	struct formatter<const wchar_t*, char> : taz::details::wide_to_narrow_formatter<const wchar_t*>
	{
	};

	template <>
	struct formatter<wchar_t*, char> : taz::details::wide_to_narrow_formatter<wchar_t*>
	{
	};

	template<>
	struct formatter<const char*, wchar_t> : taz::details::narrow_to_wide_formatter<const char*>
	{
	};

	template<>
	struct formatter<char*, wchar_t> : taz::details::narrow_to_wide_formatter<char*>
	{
	};

	template <>
	struct formatter<HWND, char> : formatter<uint32_t, char>
	{
		constexpr auto parse(std::format_parse_context& ctx)
		{
			if (m_useDefaultFormatting = ctx.begin() == ctx.end(); !m_useDefaultFormatting)
			{
				for (auto&& it = ctx.begin(); it != ctx.end() && *it != '}'; ++it)
				{
					m_showHexPrefix = (it == ctx.begin()) && (*it == '#');
				}
			}

			auto resultIterator = formatter<uint32_t, char>::parse(ctx);
			return resultIterator;
		}

		auto format(HWND input, std::format_context& ctx)
		{
			auto value = static_cast<uint32_t>(reinterpret_cast<uint64_t>(input));
			if (m_useDefaultFormatting)
			{
				m_showHexPrefix = true;
				m_showWindowClass = true;
				m_showWindowText = true;
			}

			if (m_showHexPrefix)
			{
				format_to(ctx.out(), "0x"sv);
			}

			format_to(ctx.out(), "{:08X}"sv, value);

			if (m_showWindowText)
			{
				format_to(ctx.out(), " \"{}\""sv, taz::details::GetWindowTextString(input));
			}

			if (m_showWindowClass)
			{
				format_to(ctx.out(), " \"{}\""sv, taz::details::GetWindowClassNameString(input));
			}

			return ctx.out();
		}

	private:
		bool m_useDefaultFormatting : 1 {};
		bool m_showHexPrefix : 1 {};
		bool m_showWindowText : 1 {};
		bool m_showWindowClass : 1 {};
	};
}

