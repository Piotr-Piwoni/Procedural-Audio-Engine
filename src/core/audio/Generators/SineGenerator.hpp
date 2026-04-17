#pragma once
#include <numbers>

#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class SineGenerator final : public SoundGenerator
{
public:
	float Generate() override
	{
		const float sample = std::sinf(m_Phase);

		m_Phase += m_PhaseIncrement;
		if (m_Phase >= 2.f * std::numbers::pi_v<float>)
			m_Phase -= 2.f * std::numbers::pi_v<float>;

		return sample;
	}

	void UpdatePhaseIncrement(const float frequency,
							  const unsigned long sampleRate) override
	{
		if (sampleRate < 1)
		{
			std::cerr << "Sample rate must be greater than 0!\n";
			return;
		}

		m_PhaseIncrement = 2.f * std::numbers::pi_v<float> * (frequency /
							   static_cast<float>(sampleRate));
	}
};
}
