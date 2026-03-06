#pragma once
#include <random>

#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class NoiseGenerator final : public SoundGenerator
{
public:
	float Generate(const GeneratorParams& params) override
	{
		static std::uniform_real_distribution dist(-1.f, 1.0f);
		static std::mt19937 gen(std::random_device{}());

		if (params.RandomSeed != 0)
			gen.seed(params.RandomSeed);

		return dist(gen);
	}
};
}
