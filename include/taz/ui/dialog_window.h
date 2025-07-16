#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cinttypes>
#include <optional>
#include <type_traits>
#include "application.h"
#include "message_lookup.h"
#include "top_level_window.h"
#include "window_base.h"

namespace taz::ui
{
	template<typename TDerived>
	struct dialog_window : top_level_window<TDerived>
	{
		friend TDerived;

		// Statically overridable methods
		HWND create();
		bool is_main_application_window() { return false; }
		bool make_console_child_of_main() const { return false; }
		HICON load_big_icon() { return nullptr; }
		HICON load_small_icon() { return nullptr; }
		void on_subclassed(HWND hwnd, bool isDialogAsWindow);
		bool on_init_dialog(HWND hwndFocusControl, LPARAM parameter) { return true; }
		void on_resized(ResizeType resizeType, uint16_t cx, uint16_t cy);
		bool on_close();
		bool on_destroy();

	protected:
		HWND create_from_template(uint16_t templateID);
		HWND create_from_template(PCWSTR templateName);
		void attach_to_console();
		void detach_from_console();

	private:
		dialog_window()
		{
			static_assert(std::is_base_of_v<dialog_window, TDerived>, "TDerived must inherit from dialog_window");
		}
		dialog_window(const dialog_window&) = delete;
		dialog_window(dialog_window&&) = delete;
		dialog_window& operator=(const dialog_window&) = delete;
		dialog_window& operator=(dialog_window&&) = delete;

		static INT_PTR CALLBACK dialog_proc_thunk(HWND, UINT, WPARAM, LPARAM);
		INT_PTR dialog_proc(HWND, UINT, WPARAM, LPARAM);
		bool on_wm_init_dialog(HWND hwndFocusControl, LPARAM parameter);

		HWND m_hwndPreviousConsoleParent{};
		DWORD m_previousConsoleStyle{};
		DWORD m_previousConsoleStyleEx{};
		bool m_inFormatting{};
	};

	template<typename TDerived>
	inline void dialog_window<TDerived>::on_resized(ResizeType resizeType, uint16_t cx, uint16_t cy)
	{
		if (make_console_child_of_main())
		{
			// Get the client rectangle of the main window
			RECT rcClient{};
			if (!GetClientRect(this->hwnd(), &rcClient))
				taz::debug.write_line("on_resized: GetClientRect({:08X}) failed with LastError={:08X}", this->hwnd(), GetLastError());

			// Resize the console child window within the client rectangle of the main window
			int xClient = rcClient.left + 10;
			int yClient = rcClient.top + 10;
			int cxClient = rcClient.right - rcClient.left - 10;
			int cyClient = rcClient.bottom - rcClient.top - 10;
			DWORD dwFlags = SWP_NOZORDER;
			if (!SetWindowPos(application_base::current().get_console_window(), nullptr, xClient, yClient, cxClient, cyClient, dwFlags))
				taz::debug.write_line("on_resized: SetWindowPos({:08X}) failed with LastError={:08X}", this->hwnd(), GetLastError());
		}
	}

	template<typename TDerived>
	inline bool dialog_window<TDerived>::on_close()
	{
		EndDialog(this->derived().hwnd(), (INT_PTR)IDCANCEL + 3);
		return true; // Indicate that the dialog should be destroyed
	}

	template<typename TDerived>
	inline bool dialog_window<TDerived>::on_destroy()
	{
		FreeConsole();
		detach_from_console();
		return true; // Indicate that the dialog is being destroyed and should post the quit message if it's the main application window
	}

	template<typename TDerived>
	inline HWND dialog_window<TDerived>::create_from_template(uint16_t templateID)
	{
		return create_from_template(MAKEINTRESOURCEW(templateID));
	}

	template<typename TDerived>
	inline HWND dialog_window<TDerived>::create_from_template(PCWSTR templateName)
	{
		// Create a dialog box with the specified template name and parameter
		auto parameter = reinterpret_cast<LPARAM>(&this->derived());
		this->derived().m_hwnd = CreateDialogParamW(application_base::current().instance_handle(), templateName, nullptr, dialog_proc_thunk, parameter);
		if (!this->derived().m_hwnd)
		{
			taz::debug.write_line("dialog_window::create_from_template: CreateDialogParamW failed with LastError={:08X}", GetLastError());
			return nullptr;
		}

		return this->derived().m_hwnd;
	}

	template<typename TDerived>
	inline void dialog_window<TDerived>::attach_to_console()
	{
		if (!make_console_child_of_main())
			return;

		// Turn the console window into a child in our client area
		if (auto hwndConsole = taz::ui::application_base::current().get_console_window(); hwndConsole != nullptr)
		{
			// Change the window styles
			DWORD dwStyle = WS_CHILD /*| WS_CLIPSIBLINGS*/ | WS_VSCROLL | WS_BORDER;
			DWORD dwExStyle = 0;
			m_previousConsoleStyle = SetWindowLong(hwndConsole, GWL_STYLE, dwStyle);
			m_previousConsoleStyleEx = SetWindowLong(hwndConsole, GWL_EXSTYLE, dwExStyle);

			// Apply the window styles
			DWORD dwFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE;
			if (!SetWindowPos(hwndConsole, nullptr, 0, 0, 0, 0, dwFlags))
				taz::debug.write_line("attach_to_console: SetWindowPos on Console window failed with LastError={:08X}", GetLastError());

			// Change the parent of the console window
			m_hwndPreviousConsoleParent = SetParent(hwndConsole, this->derived().m_hwnd);
			DWORD lastError = GetLastError();
			if (ERROR_SUCCESS == lastError)
				taz::debug.write_line("attach_to_console: SetParent on Console window returned previous parent {:016X}", m_hwndPreviousConsoleParent);
			else
				taz::debug.write_line("attach_to_console: SetParent on Console window failed with LastError={:08X}", lastError);

			// Get the client rectangle of the new parent window
			RECT rcClient{};
			if (!GetClientRect(this->derived().m_hwnd, &rcClient))
				taz::debug.write_line("attach_to_console: GetClientRect({:016X}) failed with LastError={:08X}", this->derived().m_hwnd, GetLastError());

			// Resize the console child window within the client rectangle of the main window
			int xClient = rcClient.left;
			int yClient = rcClient.top;
			int cxClient = rcClient.right - rcClient.left;
			int cyClient = rcClient.bottom - rcClient.top;
			dwFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW;
				taz::debug.write_line("attach_to_console: SetWindowPos on Console window failed with LastError={:08X}", GetLastError());
			if (!SetWindowPos(hwndConsole, nullptr, xClient, yClient, cxClient, cyClient, dwFlags))

			// Invalidate the changed windows
			if (!InvalidateRect(hwndConsole, nullptr, true))
				taz::debug.write_line("attach_to_console: InvalidateRect on Console window failed with LastError={:08X}", GetLastError());
			if (!InvalidateRect(this->derived().m_hwnd, nullptr, true))
				taz::debug.write_line("attach_to_console: InvalidateRect on main window failed with LastError={:08X}", GetLastError());
			UpdateWindow(hwndConsole);
			UpdateWindow(this->derived().m_hwnd);
		}
	}

	template<typename TDerived>
	inline void dialog_window<TDerived>::detach_from_console()
	{
		if (!make_console_child_of_main())
			return;

		// Restore the console's previous styles and parent/owner
		if (auto hwndConsole = taz::ui::application_base::current().get_console_window(); hwndConsole != nullptr)
		{
			// Change the parent of the console window
			SetParent(hwndConsole, m_hwndPreviousConsoleParent);
			DWORD lastError = GetLastError();
			if (ERROR_SUCCESS == lastError)
				taz::debug.write_line("detach_from_console: SetParent on Console window returned previous parent {:016X}", m_hwndPreviousConsoleParent);
			else
				taz::debug.write_line("detach_from_console: SetParent on Console window failed with LastError={:08X}", lastError);

			// Change the window styles
			SetWindowLong(hwndConsole, GWL_STYLE, m_previousConsoleStyle);
			SetWindowLong(hwndConsole, GWL_EXSTYLE, m_previousConsoleStyleEx);

			// Apply the window styles
			DWORD dwFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE;
			if (!SetWindowPos(hwndConsole, nullptr, 0, 0, 0, 0, dwFlags))
				taz::debug.write_line("detach_from_console: SetWindowPos on Console window failed with LastError={:08X}", GetLastError());
		}
	}

	// Dialog procedure for main window dialog box
	template<typename TDerived>
	inline INT_PTR CALLBACK dialog_window<TDerived>::dialog_proc_thunk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// If this is the initialization message, we need to set the user data
		if (message == WM_INITDIALOG && lParam)
		{
			SetWindowSubclass(hwnd, window_base<TDerived>::subclass_proc, 0, lParam);
		}

		if (auto& that = *dialog_window<TDerived>::from_hwnd(hwnd, false); &that)
		{
			that.m_hwnd = hwnd;
			that.dialog_proc(hwnd, message, wParam, lParam);
		}
		return 0;
	}

	template<typename TDerived>
	inline INT_PTR dialog_window<TDerived>::dialog_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//if (!m_inFormatting)
		//{
		//	m_inFormatting = true;
		//	auto formattingScope = wil::scope_exit([this]() { m_inFormatting = false; });
		//	auto messageName = message_lookup::get_name(message).data();
		//	taz::debug.write_line("dialog_window::dialog_proc: wParam={:016X} lParam={:016X} hwnd={} {}", wParam, lParam, hwnd, messageName);
		//	taz::console_out.write_line("dialog_window::dialog_proc: wParam={:016X} lParam={:016X} hwnd={} {}", wParam, lParam, hwnd, messageName);
		//}

		std::optional<LRESULT> result;

		if (message == WM_INITDIALOG)
			return this->derived().on_wm_init_dialog(reinterpret_cast<HWND>(wParam), lParam);

		if (message == WM_COMMAND)
			result = true;

		if (result.has_value())
			SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result.value());

		return (INT_PTR)result.has_value();
	}

	template<typename TDerived>
	inline void dialog_window<TDerived>::on_subclassed(HWND hwnd, bool isDialogAsWindow)
	{
		SetWindowLongPtrW(hwnd, DWLP_DLGPROC, reinterpret_cast<LONG_PTR>(this->derived().dialog_proc_thunk));
		if (isDialogAsWindow)
		{
			HWND hwndTab = GetNextDlgTabItem(this->derived().m_hwnd, nullptr, false);
			SendMessageW(hwnd, WM_INITDIALOG, reinterpret_cast<WPARAM>(hwndTab), {});
		}
	}

	template<typename TDerived>
	inline bool dialog_window<TDerived>::on_wm_init_dialog(HWND hwndFocusControl, LPARAM parameter)
	{
		// Center the dialog if specified
		auto style = static_cast<DWORD>(GetWindowLongPtrW(this->hwnd(), GWL_STYLE));
		if (style & DS_CENTER)
			this->center_on_desktop();

		// Set the large and small icons
		SendMessage(this->derived().hwnd(), WM_SETICON, ICON_SMALL2, reinterpret_cast<LPARAM>(this->derived().load_small_icon()));
		SendMessage(this->derived().hwnd(), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(this->derived().load_small_icon()));
		SendMessage(this->derived().hwnd(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(this->derived().load_big_icon()));

		// Allow the derived class to handle initialization
		bool result = this->derived().on_init_dialog(hwndFocusControl, parameter);

		// Turn the console window into a child in our client area
		attach_to_console();
		taz::console_out.write_line("dialog_window::on_wm_init_dialog: Hello, world\n");

		return result;
	}
}
