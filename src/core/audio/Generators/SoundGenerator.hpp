#pragma once

namespace MT::Core::Audio
{
class SoundGenerator
{
public:
	virtual ~SoundGenerator() = default;

	virtual float Generate() = 0;

	/**
	 * @brief Updates the phase increment based on current frequency and sample rate.
	 * This controls the progression of the waveform phase for sample generation.
	 */
	virtual void UpdatePhaseIncrement(float frequency, unsigned long sampleRate)
	{}

protected:
	float m_Phase{0.f};
	float m_PhaseIncrement{0.f};
};
}
