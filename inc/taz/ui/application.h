#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <type_traits>
#include "application_base.h"
#include "..\error_utility.h"

namespace taz::ui
{
	template<typename TDerived>
	struct application : application_base
	{
		friend typename TDerived;

		application() : application_base()
		{
			static_assert(std::is_base_of_v<application_base, TDerived>, "TDerived must derive from application_base");
		}

		application(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, PWSTR commandLine, int showCommand)
			: application_base(hInstance, hPrevInstance, commandLine, showCommand) { }

		int run();

		// Overridable methods
		HWND create_main_window();
		PCWSTR get_console_title() const { return L"Application Console"; }
		bool get_show_console() const { return false; }
		int run_message_loop(HWND hwnd);
		HACCEL get_accelerator() { return nullptr; }

	private:
		TDerived& derived() { return *static_cast<TDerived*>(this); }
		TDerived const& derived() const { return *static_cast<TDerived const*>(this); }

	private:
		application(const application&) = delete;
		application(application&&) = delete;
		application& operator=(const application&) = delete;
		application& operator=(application&&) = delete;
	};

	template<typename TDerived>
	inline int application<TDerived>::run()
	{
		auto hwnd = derived().create_main_window();
		if (!hwnd)
		{
			taz::debug.write_line("application::run: Failed to create main window.");
			return -1; // Exit on error
		}

		auto consoleTitle = derived().get_console_title();
		auto showConsole = derived().get_show_console();
		if (consoleTitle && showConsole)
			allocate_console(consoleTitle);

		auto result = derived().run_message_loop(hwnd);

		if (consoleTitle && showConsole)
			free_console();

		return result;
	}

	template<typename TDerived>
	inline int application<TDerived>::run_message_loop(HWND hwnd)
	{
		MSG message{};
		BOOL result{};
		while ((result = GetMessageW(&message, nullptr, 0, 0)) != 0)
		{
			if (result == -1)
			{
				auto lastError = GetLastError();
				taz::debug.write_line("application_base::run_message_loop: GetMessageW: lastError={:08X}: {}",
					lastError, taz::error_utility::get_last_error_message().c_str());
				return -1; // Exit on error
			}

			try
			{
				if (derived().get_accelerator() && !TranslateAcceleratorW(hwnd, derived().get_accelerator(), &message))
				{
					if (!IsDialogMessageW(hwnd, &message))
					{
						TranslateMessage(&message);
						DispatchMessageW(&message);
					}
				}
			}
			catch (std::exception const& ex)
			{
				taz::debug.write_line("application_base::run_message_loop: exception: what={}", ex.what());
			}
			catch (...)
			{
				taz::debug.write_line("application_base::run_message_loop: unknown exception: ");
			}
		}

		taz::console_out.exit();
		taz::console_err.exit();

		auto messageResult = message.wParam;
		return static_cast<int>(messageResult);
	}

}
