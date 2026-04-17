#pragma once
#include <random>

#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class NoiseGenerator final : public SoundGenerator
{
public:
	float Generate() override
	{
		static std::uniform_real_distribution dist(-1.f, 1.0f);
		static std::mt19937 gen(std::random_device{}());

		if (m_Seed != 0) gen.seed(m_Seed);
		return dist(gen);
	}

	void SetSeed(const uint32_t seed) { m_Seed = seed; }
	[[nodiscard]] uint32_t GetSeed() const { return m_Seed; }

private:
	uint32_t m_Seed = 0;
};
}
