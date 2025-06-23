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

	private:
		TDerived& derived() { return *static_cast<TDerived*>(this); }

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

		// allocate_console(L"Application Console");
		auto result = derived().run_message_loop(hwnd);
		// free_console();

		return result;
	}
}
