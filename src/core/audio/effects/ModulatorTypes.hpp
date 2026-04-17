#pragma once
#include <cstdint>

namespace MT::Core::Audio
{
enum class ModulatorTarget : uint8_t
{
	FREQUENCY = 0,
	PITCH,
	AMPLITUDE_GAIN,
	AMPLITUDE_DECIBEL
};

inline const char* ToString(const ModulatorTarget e)
{
	switch (e)
	{
	case ModulatorTarget::FREQUENCY: return "FREQUENCY";
	case ModulatorTarget::PITCH: return "PITCH";
	case ModulatorTarget::AMPLITUDE_GAIN: return "AMPLITUDE_GAIN";
	case ModulatorTarget::AMPLITUDE_DECIBEL: return "AMPLITUDE_DECIBEL";
	default: return "unknown";
	}
}


// Can be expanded to support for wave types.
enum class ModulatorWave : uint8_t
{
	SINE = 0,
};

inline const char* ToString(const ModulatorWave e)
{
	switch (e)
	{
	case ModulatorWave::SINE: return "SINE";
	default: return "unknown";
	}
}
}
