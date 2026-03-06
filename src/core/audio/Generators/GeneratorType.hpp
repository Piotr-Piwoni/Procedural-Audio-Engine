#pragma once
#include <cstdint>

namespace MT::Core::Audio
{
enum class GeneratorType : uint8_t
{
	NONE = 0,
	SINE,
	NOISE
};

inline const char* ToString(const GeneratorType type)
{
	switch (type)
	{
	case GeneratorType::NONE: return "NONE";
	case GeneratorType::SINE: return "SINE";
	case GeneratorType::NOISE: return "NOISE";
	default: return "unknown";
	}
}
}
