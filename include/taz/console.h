#pragma once

// Standard C headers
#include <stdio.h>

// Standard C++ headers
#include <string>
#include <utility>

// Local headers
#include "logger.h"
#include "string_utility.h"
#include "thread_queue.h"

namespace taz
{
	struct console_output final
	{
		void write_out(std::string const& message)
		{
			StringWorkItem workItem{ std::move(string_utility::widen(message)), m_file };
			s_queue.push( std::move(workItem) );
		}
		void write_out(std::wstring const& message)
		{
			StringWorkItem workItem(std::wstring{ message }, m_file);
			s_queue.push( std::move(workItem) );
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

	public:
		struct StringWorkItem final
		{
			StringWorkItem() = default;
			~StringWorkItem() = default;
			StringWorkItem(std::wstring&& message, FILE* file)
				: m_file(file)
			{
				std::swap(message, m_message);
			}
			StringWorkItem(const StringWorkItem& that)
				: m_message(that.m_message)
				, m_file(that.m_file)
			{
			}
			StringWorkItem(StringWorkItem&& that) noexcept
				: m_message(std::move(that.m_message))
			{
				std::swap(m_file, that.m_file);
			}
			StringWorkItem operator=(StringWorkItem& that)
			{
				m_message = that.m_message;
				std::swap(m_file, that.m_file);
			}
			StringWorkItem operator=(StringWorkItem&& that) noexcept
			{
				m_message = std::move(that.m_message);
				std::swap(m_file, that.m_file);
			}

			void execute()
			{
				fputws(m_message.c_str(), m_file);
			}

		private:
			std::wstring m_message;
			FILE* m_file{};
		};
		static_assert(WorkItem<StringWorkItem>);

		inline static thread_queue<StringWorkItem> s_queue{};
		FILE* m_file{};
	};
	static_assert(log_writer<console_output>);

	inline logger<console_output> console_out{ { stdout } };
	inline logger<console_output> console_err{ { stderr } };
}