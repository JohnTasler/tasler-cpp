#pragma once

// Standard C++ headers
#include <concepts>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

// Local headers
#include "formatters.h"

using namespace std::literals;

namespace taz
{
	template <typename Writer>
	concept log_writer = (std::copy_constructible<Writer> || std::move_constructible<Writer>) && requires(Writer w)
	{
		w.write_out(""s);
		w.write_out(L""s);
		w.exit();
	};


	template <log_writer Writer>
	struct logger final
	{
		inline static constexpr auto c_crlf = "\r\n"sv;
		inline static constexpr auto c_w_crlf = L"\r\n"sv;

		template< class... Args >
		void write_line([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_format_args(args...)).append(c_crlf);
			m_writer.write_out(message);
		}

		template< class... Args >
		void write_line([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_wformat_args(args...)).append(c_w_crlf);
			m_writer.write_out(message);
		}

		template< class... Args >
		void write([[maybe_unused]] std::string_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_format_args(args...));
			m_writer.write_out(message);
		}

		template< class... Args >
		void write([[maybe_unused]] std::wstring_view fmt, [[maybe_unused]] Args&&... args)
		{
			auto message = std::vformat(fmt, std::make_wformat_args(args...));
			m_writer.write_out(message);
		}

		logger(Writer&& writer)
			: m_writer(std::move(writer))
		{
		}
		~logger()
		{
			exit();
		}

		logger() = delete;
		logger(logger const&) = default;
		logger(logger&&) = default;
		logger& operator=(logger const&) = default;
		logger& operator=(logger&&) = default;

		void exit()
		{
			m_writer.exit();
		}

	private:
		Writer m_writer{};
	};
}
