#pragma once
#include <cstdint>

namespace MT::Core::Audio
{
enum class GeneratorType : uint8_t
{
	NONE = 0,
	SINE,
	NOISE,
	SAW_TOOTH,
	SQUARE
};

inline const char* ToString(const GeneratorType type)
{
	switch (type)
	{
	case GeneratorType::NONE: return "NONE";
	case GeneratorType::SINE: return "SINE";
	case GeneratorType::NOISE: return "NOISE";
	case GeneratorType::SAW_TOOTH: return "SAW_TOOTH";
	case GeneratorType::SQUARE: return "SQUARE";
	default: return "unknown";
	}
}
}
