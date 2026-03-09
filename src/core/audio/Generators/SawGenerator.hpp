#pragma once
#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class SawGenerator final : public SoundGenerator
{
public:
	float Generate(const GeneratorParams& params) override
	{
		const float sample = 2.0f * m_Phase - 1.0f;

		m_Phase += m_PhaseIncrement;

		if (m_Phase >= 1.0f)
			m_Phase -= 1.0f;

		return sample;
	}

	void UpdatePhaseIncrement(const float frequency,
							  const float sampleRate) override
	{
		if (sampleRate < 1.f)
		{
			std::cerr << "Sample rate must be greater than 0!\n";
			return;
		}

		m_PhaseIncrement = frequency / sampleRate;
	}
};
}
