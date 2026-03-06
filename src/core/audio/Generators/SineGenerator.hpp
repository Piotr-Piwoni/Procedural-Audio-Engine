#pragma once
#include "SoundGenerator.hpp"

namespace MT::Core::Audio
{
class SineGenerator final : public SoundGenerator
{
public:
	float Generate(const GeneratorParams& params) override
	{
		const float sample = std::sinf(m_Phase);

		m_Phase += m_PhaseIncrement;
		if (m_Phase >= 2.f * std::numbers::pi_v<float>)
			m_Phase -= 2.f * std::numbers::pi_v<float>;

		return sample;
	}
};
}
