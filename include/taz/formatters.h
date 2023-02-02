#pragma once

// Windows headers
#include <stringapiset.h>
#include <wtypes.h>

// Standard C++ headers
#include <format>
#include <string>

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
}

namespace std
{
	template<>
	struct formatter<const wchar_t*, char> : _Formatter_base<const wchar_t*, char, _Basic_format_arg_type::_Custom_type>
	{
		auto format(const wchar_t* input, std::format_context& ctx)
		{
			// Measure for the conversion to UTF-8
			std::wstring_view inputString{ input };
			auto bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), nullptr, 0, nullptr, nullptr);

			// Convert to UTF-8
			std::vector<char> output(bytesNeeded);
			WideCharToMultiByte(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), output.data(), bytesNeeded, nullptr, nullptr);

			// Write it out
			return std::copy(begin(output), end(output), ctx.out());
		}
	};

	template<>
	struct formatter<const char*, wchar_t> : _Formatter_base<const char*, wchar_t, _Basic_format_arg_type::_Custom_type>
	{
		auto format(const char* input, std::wformat_context& ctx)
		{
			// Measure for the conversion to UTF-8
			std::string_view inputString{ input };
			auto bytesNeeded = MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), nullptr, 0);

			// Convert to UTF-8
			std::vector<wchar_t> output(bytesNeeded);
			MultiByteToWideChar(CP_UTF8, 0, inputString.data(), static_cast<int>(inputString.size()), output.data(), bytesNeeded);

			// Write it out
			return std::copy(begin(output), end(output), ctx.out());
		}
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

