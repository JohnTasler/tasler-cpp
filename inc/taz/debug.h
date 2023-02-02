#pragma once

#include <format>
#include <string>
#include <string_view>

using namespace std::literals;

#include <debugapi.h>

#define ALWAYS_OUTPUT_DEBUG_STRING true

struct debug final
{
	inline static constexpr auto c_crlf = "\r\n"sv;
	inline static constexpr auto c_w_crlf = L"\r\n"sv;

	debug() = delete;
	debug(debug const&) = delete;
	debug(debug &&) = delete;
	~debug() = delete;
	debug& operator=(debug const&) = delete;
	debug& operator=(debug &&) = delete;

	template< class... Args >
	static void write_line([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args )
	{
		#if ALWAYS_OUTPUT_DEBUG_STRING || defined(_DEBUG)
			OutputDebugStringA((std::vformat(fmt, std::make_format_args(args...)).append(c_crlf)).c_str());
		#endif
	}

	template< class... Args >
	static void write_line([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args )
	{
		#if ALWAYS_OUTPUT_DEBUG_STRING || defined(_DEBUG)
			OutputDebugStringW((std::vformat(fmt, std::make_wformat_args(args...)).append(c_w_crlf)).c_str());
		#endif
	}

	template< class... Args >
	static void write([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args )
	{
		#if ALWAYS_OUTPUT_DEBUG_STRING || defined(_DEBUG)
			OutputDebugStringA(std::vformat(fmt, std::make_format_args(args...)).c_str());
		#endif
	}

	template< class... Args >
	static void write([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args )
	{
		#if ALWAYS_OUTPUT_DEBUG_STRING || defined(_DEBUG)
			OutputDebugStringW(std::vformat(fmt, std::make_wformat_args(args...)).c_str());
		#endif
	}
};

struct debugger final
{
	debugger() = delete;
	debugger(debugger const&) = delete;
	debugger(debugger&&) = delete;
	~debugger() = delete;
	debugger& operator=(debugger const&) = delete;
	debugger& operator=(debugger&&) = delete;

	template< class... Args >
	static bool is_attached()
	{
		return !!IsDebuggerPresent();
	}
};
