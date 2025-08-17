#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>
#include "..\debug.h"
#include "..\error_utility.h"
#include "..\console.h"

namespace taz::ui
{
	struct application_base
	{
		application_base();
		application_base(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, PWSTR commandLine, int showCommand);
		~application_base();

		static application_base& current() { return *s_current; }
		HINSTANCE instance_handle() const { return m_hInstance; }
		HWND get_console_window() const { return m_hwndConsole; }

	public:
		void allocate_console(PCWSTR consoleTitle);
		void free_console();

	private:
		application_base(const application_base&) = delete;
		application_base(application_base&&) = delete;
		application_base& operator=(const application_base&) = delete;
		application_base& operator=(application_base&&) = delete;

		thread_local inline static application_base* s_current{};

		HINSTANCE m_hInstance{};
		PWSTR m_commandLine{};
		int m_showCommand{};
		HWND m_hwndConsole{};
		HANDLE m_stdoutPrevious{};
		HANDLE m_stderrPrevious{};
		FILE* m_stdoutFilePrevious{};
		FILE* m_stderrFilePrevious{};
	};

	inline application_base::application_base()
		: application_base(GetModuleHandleW(nullptr), nullptr, GetCommandLineW(), SW_SHOWDEFAULT)
	{
	}

	inline application_base::application_base(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, PWSTR commandLine, int showCommand)
		: m_hInstance{ hInstance }
		, m_commandLine{ commandLine }
		, m_showCommand{ showCommand }
	{
		if (s_current)
			throw std::logic_error("Only one instance of application_base can be created simultaneously on a thread.");

		s_current = this;
		m_hInstance = hInstance;
		m_commandLine = commandLine;
		m_showCommand = showCommand;
	}

	inline application_base::~application_base()
	{
		s_current = nullptr;
	}

	inline void application_base::allocate_console(PCWSTR consoleTitle)
	{
		m_stdoutPrevious = GetStdHandle(STD_OUTPUT_HANDLE);
		m_stderrPrevious = GetStdHandle(STD_ERROR_HANDLE);

		m_stdoutFilePrevious = stdout;
		m_stderrFilePrevious = stderr;

		// Create a console for trace/debugging output
		if (!AllocConsole())
			return;

		FILE* newOut{};
		freopen_s(&newOut, "CONOUT$", "a", stdout);

		// Get the console window's handle and hide the window for the time being
		m_hwndConsole = GetConsoleWindow();
//		ShowWindow(m_hwndConsole, SW_HIDE);

		std::println(stdout, "application_base::allocate_console: Allocated console with handle {0:016X}", m_hwndConsole);

		// Set the console window's title
		SetConsoleTitle(consoleTitle);
	}

	inline void application_base::free_console()
	{
		taz::console_out.exit();
		taz::console_err.exit();

		SetStdHandle(STD_OUTPUT_HANDLE, m_stdoutPrevious);
		SetStdHandle(STD_ERROR_HANDLE, m_stderrPrevious);

		FreeConsole();
	}
}
