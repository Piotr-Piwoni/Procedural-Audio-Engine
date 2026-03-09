#pragma once
#include <cstdint>

namespace MT::Core::Audio
{
struct GeneratorParams
{
	uint32_t RandomSeed = 0;
	float PulseWidth = 0.5f;
};
}
