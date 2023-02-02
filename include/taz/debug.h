#pragma once

// Windows headers
#include <debugapi.h>

// Local headers
#include "logger.h"

#define ALWAYS_OUTPUT_LOGGER_STRING true

namespace taz
{
	struct debug_output final
	{
		void write_out(std::string const& message)
		{
			#if ALWAYS_OUTPUT_LOGGER_STRING || defined(_DEBUG)
				OutputDebugStringA(message.c_str());
			#endif
		}
		void write_out(std::wstring const& message)
		{
			#if ALWAYS_OUTPUT_LOGGER_STRING || defined(_DEBUG)
				OutputDebugStringW(message.c_str());
			#endif
		}

		debug_output() = default;
		~debug_output() = default;
		debug_output(debug_output const&) = default;
		debug_output(debug_output&&) = default;
		debug_output& operator=(debug_output const&) = default;
		debug_output& operator=(debug_output&&) = default;
	};
	static_assert(log_writer<debug_output>);

	inline logger<debug_output> debug{ {} };

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
