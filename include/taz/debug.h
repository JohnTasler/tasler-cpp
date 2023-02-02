#pragma once

// Windows headers
#include <debugapi.h>

// Standard C++ headers
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

// Local headers
#include "formatters.h"

using namespace std::literals;

#define ALWAYS_OUTPUT_LOGGER_STRING true

namespace taz
{
	template <typename Writer>
	concept log_writer = requires(Writer)
	{
		Writer::write_out(""s);
		Writer::write_out(L""s);
	};

	template <log_writer Writer>
	struct logger final
	{
		inline static constexpr auto c_crlf = "\r\n"sv;
		inline static constexpr auto c_w_crlf = L"\r\n"sv;

		logger() = delete;
		logger(logger const&) = delete;
		logger(logger&&) = delete;
		~logger() = delete;
		logger& operator=(logger const&) = delete;
		logger& operator=(logger&&) = delete;

		template< class... Args >
		static void write_line([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_format_args(args...)).append(c_crlf);
			Writer::write_out(message);
		}

		template< class... Args >
		static void write_line([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_wformat_args(args...)).append(c_w_crlf);
			Writer::write_out(message);
		}

		template< class... Args >
		static void write([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::format(fmt, args...);
			Writer::write_out(message);
		}

		template< class... Args >
		static void write([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_wformat_args(args...));
			Writer::write_out(message);
		}
	};

	struct debug_output final
	{
		debug_output() = delete;
		debug_output(debug_output const&) = delete;
		debug_output(debug_output&&) = delete;
		~debug_output() = delete;
		debug_output& operator=(debug_output const&) = delete;
		debug_output& operator=(debug_output&&) = delete;

		static void write_out(std::string const& message)
		{
			#if ALWAYS_OUTPUT_LOGGER_STRING || defined(_DEBUG)
				OutputDebugStringA(message.c_str());
			#endif
		}
		static void write_out(std::wstring const& message)
		{
			#if ALWAYS_OUTPUT_LOGGER_STRING || defined(_DEBUG)
				OutputDebugStringW(message.c_str());
			#endif
		}
	};
	static_assert(log_writer<debug_output>);

	struct console_output final
	{
		console_output() = delete;
		console_output(console_output const&) = delete;
		console_output(console_output&&) = delete;
		~console_output() = delete;
		console_output& operator=(console_output const&) = delete;
		console_output& operator=(console_output&&) = delete;

		static void write_out(std::string const& message)
		{
			fputs(message.c_str(), stdout);
		}
		static void write_out(std::wstring const& message)
		{
			fputws(message.c_str(), stdout);
		}
	};
	static_assert(log_writer<console_output>);

	using debug = logger<debug_output>;
	using console = logger<console_output>;

	struct debugger final
	{
		debugger() = delete;
		debugger(debugger const&) = delete;
		debugger(debugger&&) = delete;
		~debugger() = delete;
		debugger& operator=(debugger const&) = delete;
		debugger& operator=(debugger&&) = delete;

		static bool is_attached()
		{
			return !!IsDebuggerPresent();
		}
	};
}
