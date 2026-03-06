#pragma once
#include <iostream>
#include <numbers>

namespace MT::Core::Audio
{
struct GeneratorParams
{
	uint32_t RandomSeed = 0;
};

class SoundGenerator
{
public:
	virtual ~SoundGenerator() = default;

	virtual float Generate(const GeneratorParams& params) = 0;

	/**
	 * @brief Updates the phase increment based on current frequency and sample rate.
	 * This controls the progression of the waveform phase for sample generation.
	 */
	void UpdatePhaseIncrement(const float frequency, const float sampleRate)
	{
		if (sampleRate < 1.f)
		{
			std::cerr << "Sample rate must be greater than 0!\n";
			return;
		}

		m_PhaseIncrement = 2.f * std::numbers::pi_v<float> *
						   (frequency / sampleRate);
	}

protected:
	float m_Phase{0.f};
	float m_PhaseIncrement{0.f};
};
}
