#pragma once
#include <iostream>

#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class SquareGenerator final : public SoundGenerator
{
public:
	float Generate() override
	{
		const float sample = m_Phase < m_PulseWidth ? 1.f : -1.f;

		m_Phase += m_PhaseIncrement;
		if (m_Phase >= 1.0f) m_Phase -= 1.0f;

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

		m_PhaseIncrement = frequency / static_cast<float>(sampleRate);
	}

	void SetPulseWidth(const float width) { m_PulseWidth = width; }
	[[nodiscard]] float GetPulseWidth() const { return m_PulseWidth; }

private:
	float m_PulseWidth = 0.5f;
};
}
