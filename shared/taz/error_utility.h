#pragma once

// Windows headers
#include <winbase.h>
#include <errhandlingapi.h>

// Standard C++ headers
#include <string>

// WIL headers
#include <wil\resource.h>

namespace taz::error_utility
{
	[[nodiscard]]
	inline std::wstring get_system_error_message(DWORD messageId)
	{
		wil::unique_hlocal_string message;
		auto size = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, messageId, 0, reinterpret_cast<PWSTR>(&message), 0, nullptr);
		return message.get();
	}

	[[nodiscard]]
	inline std::wstring get_last_error_message()
	{
		return get_system_error_message(GetLastError());
	}
}
