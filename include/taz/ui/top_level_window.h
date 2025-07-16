#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <type_traits>
#include "window_base.h"

namespace taz::ui
{
	template <typename TDerived>
	struct top_level_window : window_base<TDerived>
	{
		friend typename TDerived;

		top_level_window()
		{
			static_assert(std::is_base_of_v<window_base<TDerived>, TDerived>, "TDerived must inherit from window_base");
		}
		top_level_window(HWND hwnd)
		{
			static_assert(std::is_base_of_v<window_base<TDerived>, TDerived>, "TDerived must inherit from window_base");
			this->subclass_window(hwnd);
		}
		// Uncomment if you want to enforce inheritance from top_level_window

		void center_on_desktop()
		{
			RECT rectDesktop{};
			GetClientRect(GetDesktopWindow(), &rectDesktop);
			RECT rectWindow{};
			GetWindowRect(this->hwnd(), &rectWindow);
			auto centeredX = ((rectDesktop.right - rectDesktop.left) - (rectWindow.right - rectWindow.left)) / 2;
			auto centeredY = ((rectDesktop.bottom - rectDesktop.top) - (rectWindow.bottom - rectWindow.top)) / 2;
			SetWindowPos(this->hwnd(), HWND_TOP, centeredX, centeredY, 0, 0, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
		}
	};
}
