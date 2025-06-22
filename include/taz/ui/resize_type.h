#pragma once

namespace taz::ui
{
	/// <summary>
	/// Sent with the WM_SIZE message to indicate the type of resizing that is occurring.
	/// </summary>
	enum class ResizeType
	{
		/// <summary>Message is sent to all pop - up windows when some other window is maximized.</summary>
		MaxHide = 4,

		/// <summary>The window has been maximized.</summary>
		Maximized = 2,

		/// <summary>Message is sent to all pop - up windows when some other window has been restored to its former size.</summary>
		MaxShow = 3,

		/// <summary>The window has been minimized.</summary>
		Minimized = 1,

		/// <summary>The window has been resized, but neither the MINIMIZED nor MAXIMIZED value applies.</summary>
		Restored = 0,
	};
}
