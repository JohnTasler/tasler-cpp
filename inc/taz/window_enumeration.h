#pragma once

#include "stdafx.h"

struct window_enumeration final
{
	static std::vector<HWND> get_process_windows(DWORD idProcess)
	{
		window_enumeration windowEnumeration;
		windowEnumeration.m_predicate = [=](HWND hwnd) { return window_process_matches(hwnd, idProcess); };

		EnumWindows(enum_windows_proc, reinterpret_cast<LPARAM>(&windowEnumeration));
		return std::move(windowEnumeration.m_foundHwnds);
	}

private:
	std::function<bool(HWND hwnd)> m_predicate{};
	std::vector<HWND> m_foundHwnds;

	window_enumeration() = default;
	window_enumeration(window_enumeration const&) = delete;
	window_enumeration(window_enumeration&&) = delete;
	window_enumeration& operator=(window_enumeration const&) = delete;
	window_enumeration& operator=(window_enumeration&&) = delete;

	inline static bool window_process_matches(HWND hwnd, const DWORD idProcess)
	{
		DWORD dwProcessId{};
		DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);

		return idProcess == dwProcessId;
	}

	inline bool enum_windows_proc(HWND hwnd)
	{
		if (!m_predicate || std::invoke(m_predicate, hwnd))
		{
			m_foundHwnds.push_back(hwnd);
		}

		return true;
	}

	static BOOL WINAPI enum_windows_proc(HWND hwnd, LPARAM lParam)
	{
		auto& ctx = *reinterpret_cast<window_enumeration*>(lParam);
		return ctx.enum_windows_proc(hwnd);
	}
};

