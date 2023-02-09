#pragma once

// Windows headers
#include <processthreadsapi.h>

// Standard C++ headers
#include <array>
#include <concepts>
#include <type_traits>
#include <queue>

// tasler-cpp headers
#include "debug.h"

// WIL headers
#include <wil/resource.h>

namespace taz
{
	template<typename TWorkItem>
	concept WorkItem = std::move_constructible<TWorkItem>
		&& std::copy_constructible<TWorkItem>
		&& requires(TWorkItem& workItem)
	{
		workItem.execute();
	};

	template<WorkItem TWorkItem>
	struct thread_queue final
	{
		thread_queue(bool createSuspended = false)
		{
			m_readyEvent.create(wil::EventOptions::ManualReset);
			m_exitEvent.create(wil::EventOptions::ManualReset);

			m_handle = CreateThread(nullptr, 0, thread_start_thunk, reinterpret_cast<void*>(this), createSuspended ? CREATE_SUSPENDED : 0, &m_threadId);
			THROW_LAST_ERROR_IF_NULL(m_handle);
		}
		~thread_queue() = default;

		void push(TWorkItem&& workItem)
		{
			auto lock = m_lock.lock_exclusive();
			m_queue.push(std::move(workItem));
			m_readyEvent.SetEvent();
		}

		void run()
		{
			std::array<HANDLE, 2> events = { m_exitEvent.get(), m_readyEvent.get() };
			DWORD result{};
			while ((result = WaitForMultipleObjects(events.size(), events.data(), false, INFINITE)) == (WAIT_OBJECT_0 + 1))
			{
				auto lock = m_lock.lock_exclusive();
				while (!m_queue.empty())
				{
					try
					{
						auto&& workItem = std::move(m_queue.front());
						workItem.execute();
						m_queue.pop();
					}
					catch (std::exception const& ex)
					{
						debug.write_line(L"taz::thread_queue::run: exception={}", ex.what());
					}
					catch (...)
					{
						debug.write_line(L"taz::thread_queue::run: unknown exception");
					}
				}

				m_readyEvent.ResetEvent();
			}

			if (result == WAIT_FAILED)
			{
				debug.write_line(L"taz::thread_queue::run: WaitForMultipleObjects failed lastError={:08X}", GetLastError());
			}

			m_readyEvent.SetEvent();
		}

		void exit()
		{
			m_exitEvent.SetEvent();
			m_readyEvent.wait();
		}

		DWORD id() const { return m_threadId; }
		HANDLE handle() const { return m_handle; }

	private:
		static DWORD WINAPI thread_start_thunk(void* param)
		{
			auto& thisref = *reinterpret_cast<thread_queue*>(param);
			thisref.run();
			return 0;
		}

		thread_queue(thread_queue const&) = delete;
		thread_queue(thread_queue&&) = delete;
		thread_queue& operator=(thread_queue const&) = delete;
		thread_queue& operator=(thread_queue&&) = delete;

		wil::srwlock m_lock{};
		wil::unique_event m_readyEvent{};
		wil::unique_event m_exitEvent{};
		std::queue<TWorkItem> m_queue{};
		DWORD m_threadId{};
		HANDLE m_handle{};
	};
}
