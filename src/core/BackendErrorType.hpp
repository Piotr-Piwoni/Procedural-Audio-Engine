#pragma once

namespace MT::Core
{
enum class BackendError : std::uint8_t
{
	NONE          = 0,
	COM_INIT      = 1,
	DEVICE_ENUM   = 2,
	DEVICE        = 3,
	CLIENT_ACTIVE = 4,
	FORMAT        = 5,
	CLIENT_INIT   = 6,
	RENDER_CLIENT = 7
};

inline const char* to_string(const BackendError e)
{
	switch (e)
	{
	case BackendError::NONE:
		return "NONE";
	case BackendError::COM_INIT:
		return "COM_INIT";
	case BackendError::DEVICE_ENUM:
		return "DEVICE_ENUM";
	case BackendError::DEVICE:
		return "DEVICE";
	case BackendError::CLIENT_ACTIVE:
		return "CLIENT_ACTIVE";
	case BackendError::FORMAT:
		return "FORMAT";
	case BackendError::CLIENT_INIT:
		return "CLIENT_INIT";
	case BackendError::RENDER_CLIENT:
		return "RENDER_CLIENT";
	default:
		return "unknown";
	}
}
}
