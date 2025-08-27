#pragma once

// Windows headers
#include <wtypes.h>

// Standard C++ headers
#include <string_view>

// WIL headers
#include <wil/result.h>

using namespace std::literals;

namespace taz
{
	namespace details
	{
		inline void* find_and_load_core(HMODULE module, PCWSTR id, PCWSTR type, uint16_t /*lang*/, _Out_opt_ uint32_t* byteCount)
		{
			auto resourceInformationBlock = ::FindResourceW(module, id, type);
			THROW_LAST_ERROR_IF_NULL(resourceInformationBlock);

			auto resource = ::LoadResource(module, resourceInformationBlock);
			THROW_LAST_ERROR_IF_NULL(resource);

			auto data = ::LockResource(resource);
			THROW_LAST_ERROR_IF_NULL(data);

			if (byteCount)
			{
				*byteCount = ::SizeofResource(module, resourceInformationBlock);
			}

			return data;
		}
	}

	struct resource_loader final
	{
		inline static constexpr std::wstring_view c_AnsiText = L"ANSI_TEXT"sv;

		#pragma region static methods
		template<typename T>
		static T find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
		{
			static_assert("T is a non-supported type.");
		}

		//template<> static std::span<uint8_t> find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
		//{
		//		uint32_t byteCount{};
		//		const void* data = details::find_and_load_core(module, id, type, 0, &byteCount);
		//		const auto begin = reinterpret_cast<const uint8_t*>(data);
		//		return std::span<uint8_t>(begin, begin + byteCount);
		//}

		template<> std::string_view find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
		{
			uint32_t byteCount{};
			const void* data = details::find_and_load_core(module, id, type, 0, &byteCount);
			const auto begin = reinterpret_cast<const char*>(data);
			return { begin, byteCount };
		}

		template<> std::wstring_view find_and_load(HMODULE module, PCWSTR id, PCWSTR type)
		{
			uint32_t byteCount{};
			const void* data = details::find_and_load_core(module, id, type, 0, &byteCount);
			const auto begin = reinterpret_cast<const wchar_t*>(data);
			return { begin, byteCount / sizeof(wchar_t) };
		}

		template<typename T>
		static T find_and_load(HMODULE module, uint16_t id, PCWSTR type)
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(HMODULE module, uint16_t id, PCWSTR type)
		//{
		//    return find_and_load<std::span<uint8_t>>(module, MAKEINTRESOURCE(id), type);
		//}
		template<> std::string_view find_and_load(HMODULE module, uint16_t id, PCWSTR type)
		{
			return find_and_load<std::string_view>(module, MAKEINTRESOURCE(id), type);
		}
		template<> std::wstring_view find_and_load(HMODULE module, uint16_t id, PCWSTR type)
		{
			return find_and_load<std::wstring_view>(module, MAKEINTRESOURCE(id), type);
		}

		template<typename T>
		static T find_and_load(HMODULE module, uint16_t id, uint16_t type)
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(HMODULE module, uint16_t id, uint16_t type)
		//{
		//		return find_and_load<std::span<uint8_t>>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		//}
		template<> std::string_view find_and_load(HMODULE module, uint16_t id, uint16_t type)
		{
			return find_and_load<std::string_view>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		}
		template<> std::wstring_view find_and_load(HMODULE module, uint16_t id, uint16_t type)
		{
			return find_and_load<std::wstring_view>(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		}

		template<typename T>
		static T find_and_load(HMODULE module, PCWSTR id, uint16_t type)
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(HMODULE module, PCWSTR id, uint16_t type)
		//{
		//		return find_and_load<std::span<uint8_t>>(module, id, MAKEINTRESOURCE(type));
		//}
		template<> std::string_view find_and_load(HMODULE module, PCWSTR id, uint16_t type)
		{
			return find_and_load<std::string_view>(module, id, MAKEINTRESOURCE(type));
		}
		template<> std::wstring_view find_and_load(HMODULE module, PCWSTR id, uint16_t type)
		{
			return find_and_load<std::wstring_view>(module, id, MAKEINTRESOURCE(type));
		}

		inline std::wstring_view load_string(HMODULE module, uint16_t id) const
		{
			wchar_t* bufferAddress{};

			auto result = LoadStringW(module, id, reinterpret_cast<PWSTR>(&bufferAddress), 0);
			THROW_IF_WIN32_BOOL_FALSE(result);

			return { bufferAddress, static_cast<std::size_t>(result) };
		}

		#pragma endregion static methods

		#pragma region construction, destruction, and assignment operators
		resource_loader() = delete;
		resource_loader(resource_loader const& that) noexcept
			: m_module(that.m_module)
		{
		}
		resource_loader(resource_loader&& that) noexcept
		{
			std::swap(this->m_module, that.m_module);
		}
		~resource_loader() = default;
		resource_loader& operator=(resource_loader const& that) noexcept
		{
			m_module = that.m_module;
			return *this;
		}
		resource_loader& operator=(resource_loader&& that) noexcept
		{
			std::swap(this->m_module, that.m_module);
			return *this;
		}
		#pragma endregion construction, destruction, and assignment operators

		#pragma region instance methods
		template<typename T>
		T find_and_load(PCWSTR id, PCWSTR type) const
		{
			static_assert("T is a non-supported type.");
		}

		//template<> std::span<uint8_t> find_and_load(PCWSTR id, PCWSTR type) const
		//{
		//	return std::span<uint8_t> find_and_load(m_module, id, type);
		//}

		template<> std::string_view find_and_load(PCWSTR id, PCWSTR type) const
		{
			return find_and_load<std::string_view>(m_module, id, type);
		}

		template<> std::wstring_view find_and_load(PCWSTR id, PCWSTR type) const
		{
			return find_and_load<std::wstring_view>(m_module, id, type);
		}

		template<typename T>
		T find_and_load(uint16_t id, PCWSTR type) const
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(uint16_t id, PCWSTR type) const
		//{
		//	return std::span<uint8_t> find_and_load(m_module, id, type);
		//}
		template<> std::string_view find_and_load(uint16_t id, PCWSTR type) const
		{
			return find_and_load<std::string_view>(m_module, id, type);
		}
		template<> std::wstring_view find_and_load(uint16_t id, PCWSTR type) const
		{
			return find_and_load<std::wstring_view>(m_module, id, type);
		}

		template<typename T>
		T find_and_load(uint16_t id, uint16_t type) const
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(uint16_t id, uint16_t type) const
		//{
		//	return std::span<uint8_t> find_and_load(m_module, id, type);
		//}
		template<> std::string_view find_and_load(uint16_t id, uint16_t type) const
		{
			return find_and_load<std::string_view>(m_module, id, type);
		}
		template<> std::wstring_view find_and_load(uint16_t id, uint16_t type) const
		{
			return find_and_load<std::wstring_view>(m_module, id, type);
		}

		template<typename T>
		T find_and_load(PCWSTR id, uint16_t type) const
		{
			static_assert("T is a non-supported type.");
		}
		//template<> std::span<uint8_t> find_and_load(PCWSTR id, uint16_t type) const
		//{
		//	return std::span<uint8_t> find_and_load(m_module, id, type);
		//}
		template<> std::string_view find_and_load(PCWSTR id, uint16_t type) const
		{
			return find_and_load<std::string_view>(m_module, id, type);
		}
		template<> std::wstring_view find_and_load(PCWSTR id, uint16_t type) const
		{
			return find_and_load<std::wstring_view>(m_module, id, type);
		}

		inline std::wstring_view load_string(uint16_t id) const
		{
			return load_string(m_module, id);
		}
		#pragma endregion instance methods

	private:
		HINSTANCE m_module{};
	};
}
