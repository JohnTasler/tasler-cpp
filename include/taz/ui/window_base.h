#pragma once
#define WINVER 0x0A00
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <stdexcept>
#include <type_traits>
#include "..\debug.h"
#include "..\error_utility.h"
#include "..\console.h"
#include "resize_type.h"

namespace taz::ui
{
	template<typename TDerived>
	struct window_base
	{
		friend typename TDerived;

		// Overridable methods
		void on_subclassed(HWND hwnd) {}
		std::optional<LRESULT> on_message(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) { return std::nullopt; }
		void on_resized(ResizeType resizeType, uint16_t cx, uint16_t cy) {}
		void on_locale_policy_setting_changed() {}
		void on_user_policy_setting_changed() {}
		void on_machine_policy_setting_changed() {}
		void on_system_parameter_setting_changed(uint32_t uiAction, std::wstring_view pszSection) {}
		HBRUSH get_background_brush() const { return nullptr; }
		uint16_t on_non_client_hit_test(int defaultCode, uint16_t cxScreen, uint16_t cyScreen);
		bool on_erase_background(HDC hdc) { return false; };
		bool on_closed() { return true; };
		bool on_destroy() { return true; };
		void on_menu_command(uint16_t commandID) {}
		void on_accelerator_command(uint16_t commandID) { derived().on_menu_command(commandID); }
		void on_control_command(uint16_t notificationCode, uint16_t controlID, HWND controlWindow) {}
		void on_notify(NMHDR header, uint16_t controlID) {}
		void initialize_and_show(HWND hwnd, bool isDialog);
		HWND hwnd() const { return this->const_derived().m_hwnd; }

	protected:
		void subclass_window(HWND hwnd);
		void unsubclass_window(HWND hwnd);
		static TDerived* from_hwnd(HWND hwnd, bool throwOnNull = true)
		{
			auto ptr = reinterpret_cast<TDerived*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
			if (!ptr && throwOnNull)
				throw std::logic_error("The window is not subclassed or the user data is not set.");
			return ptr;
		}
		window_base()
		{
			static_assert(std::is_base_of_v<window_base, TDerived>, "TDerived must inherit from window_base");
		}

	public:
		TDerived& derived() { return *static_cast<TDerived*>(this); }
		TDerived const& const_derived() const { return *static_cast<TDerived const*>(this); }

	private:
		static TDerived* window_base_from_hwnd(HWND hwnd, bool noThrow = false);
		static LRESULT STDAPICALLTYPE window_proc_thunk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void on_wm_setting_change(uint32_t parameterType, std::wstring_view sectionName);
		int on_wm_non_client_hit_test(LPARAM lParam);
		bool on_wm_erase_background(HDC hdc);
		void on_wm_command(WPARAM wParam, LPARAM lParam);

	protected:
		HWND m_hwnd{};
		WNDPROC m_previousWindowProc{};
	};

	template<typename TDerived>
	inline void window_base<TDerived>::subclass_window(HWND hwnd)
	{
		m_previousWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
		if (m_previousWindowProc == window_proc_thunk)
			throw std::logic_error("The window is already subclassed.");

		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_proc_thunk));
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&this->derived()));
	}

	template<typename TDerived>
	inline void window_base<TDerived>::unsubclass_window(HWND hwnd)
	{
		if (m_previousWindowProc)
		{
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_previousWindowProc));
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			m_previousWindowProc = nullptr;
		}
		else
		{
			throw std::logic_error("The window is not subclassed.");
		}
	}

	template<typename TDerived>
	inline void window_base<TDerived>::initialize_and_show(HWND hwnd, bool isDialogAsWindow)
	{
		if (hwnd)
		{
			this->derived().m_hwnd = hwnd;
			this->subclass_window(hwnd);
			this->derived().on_subclassed(hwnd, isDialogAsWindow);
			ShowWindow(hwnd, SW_SHOW);
		}
	}

	template<typename TDerived>
	inline TDerived* window_base<TDerived>::window_base_from_hwnd(HWND hwnd, bool noThrow)
	{
		auto ptr = reinterpret_cast<TDerived*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (!ptr && !noThrow)
			throw std::logic_error("The window is not subclassed or the user data is not set.");
		return ptr;
	}

	template<typename TDerived>
	inline LRESULT STDAPICALLTYPE window_base<TDerived>::window_proc_thunk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return from_hwnd(hwnd)->window_proc(hwnd, message, wParam, lParam);
	}

	template<typename TDerived>
	inline LRESULT window_base<TDerived>::window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (auto result = derived().on_message(hwnd, message, wParam, lParam); result.has_value())
			return result.value();

		switch (message)
		{
		case WM_SIZE:
			derived().on_resized(static_cast<ResizeType>(wParam), LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_SETTINGCHANGE:
			this->derived().on_wm_setting_change(static_cast<uint32_t>(wParam), reinterpret_cast<PCWSTR>(lParam));
			break;
		case WM_NCHITTEST:
			return this->derived().on_wm_non_client_hit_test(lParam);
		case WM_CTLCOLORDLG:
			return reinterpret_cast<LRESULT>(this->derived().get_background_brush());
		case WM_ERASEBKGND:
			if (this->derived().on_erase_background(reinterpret_cast<HDC>(wParam)))
				return true; // Indicate that the background was erased
			break;
		case WM_CLOSE:
			if (this->derived().on_close())
				DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			if (this->derived().is_main_application_window() && this->derived().on_destroy())
				PostQuitMessage(0);
			break;
		case WM_COMMAND:
			this->derived().on_wm_command(wParam, lParam);
			return 0;
		case WM_NOTIFY:
			this->derived().on_notify(*reinterpret_cast<NMHDR*>(lParam), static_cast<uint16_t>(wParam));
			return NFR_UNICODE;
		case WM_NOTIFYFORMAT:
			return NFR_UNICODE;
		}

		return this->derived().m_previousWindowProc(hwnd, message, wParam, lParam);
	}

	template<typename TDerived>
	inline void window_base<TDerived>::on_wm_setting_change(uint32_t parameterType, std::wstring_view sectionName)
	{
		switch (parameterType)
		{
		case 0:
			if (sectionName == L"Policy")
				derived().on_user_policy_setting_changed();
			else if (sectionName == L"intl")
				derived().on_locale_policy_setting_changed();
			break;
		case 1:
			derived().on_machine_policy_setting_changed();
			break;
		default:
			derived().on_system_parameter_setting_changed(parameterType, sectionName);
			break;
		}
	}

	template<typename TDerived>
	inline int window_base<TDerived>::on_wm_non_client_hit_test(LPARAM lParam)
	{
		int defaultHitTestResult = static_cast<int>(m_previousWindowProc(this->hwnd(), WM_NCHITTEST, 0, lParam));
		return this->derived().on_non_client_hit_test(defaultHitTestResult, LOWORD(lParam), HIWORD(lParam));
	}

	template<typename TDerived>
	bool window_base<TDerived>::on_wm_erase_background(HDC hdc)
	{
		return this->derived().on_erase_background(hdc);
	}

	template<typename TDerived>
	inline void window_base<TDerived>::on_wm_command(WPARAM wParam, LPARAM lParam)
	{
		uint16_t commandID = LOWORD(wParam);
		uint16_t notificationCode = HIWORD(wParam);
		if (notificationCode == 0 && !lParam)
		{
			// Menu command
			this->derived().on_menu_command(commandID);
		}
		else if (notificationCode == 1)
		{
			// Accelerator command
			this->derived().on_accelerator_command(commandID);
		}
		else
		{
			// Control command
			this->derived().on_control_command(notificationCode, commandID, reinterpret_cast<HWND>(lParam));
		}
	}
}
