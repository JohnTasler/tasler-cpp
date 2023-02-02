#pragma once

// Standard C headers
#include <stdio.h>

// Local headers
#include "logger.h"

namespace taz
{
	struct console_output final
	{
		void write_out(std::string const& message) const
		{
			fputs(message.c_str(), m_file);
		}
		void write_out(std::wstring const& message) const
		{
			fputws(message.c_str(), m_file);
		}

		console_output(FILE* file)
			: m_file(file)
		{
		}
		~console_output() = default;
		console_output(console_output const&) = default;
		console_output(console_output&&) = default;
		console_output& operator=(console_output const&) = default;
		console_output& operator=(console_output&&) = default;

	private:
		FILE* m_file{};
	};
	static_assert(log_writer<console_output>);

	inline logger<console_output> console_out{ { stdout } };
	inline logger<console_output> console_err{ { stderr } };
}