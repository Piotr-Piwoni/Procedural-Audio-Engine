#include "Sound.hpp"

#include <algorithm>
#include <iostream>

#include "../../Utilities/Utils.hpp"
#include "effects/FadeEffect.hpp"
#include "generators/NoiseGenerator.hpp"
#include "generators/SawGenerator.hpp"
#include "generators/SineGenerator.hpp"
#include "generators/SquareGenerator.hpp"

namespace MT::Core::Audio
{
/**
 * @brief Constructs a Sound object with specified volume and sample rate.
 * @param volume Initial volume, clamped between 0 and 1.
 * @param sampleRate Sample rate in Hz; must be greater than 0.
 * @param type Type of the sound generator used.
 * @throws std::invalid_argument If sampleRate is <= 0.
 */
Sound::Sound(const float volume, const unsigned long sampleRate,
			 const GeneratorType type)
{
	if (sampleRate < 1)
		throw std::invalid_argument("Sample rate must be greater than 0.");

	m_Effects.reserve(5);

	SetGeneratorType(type);
	SetSampleRate(sampleRate);
	SetVolume(volume);
}

/**
 * @brief Generates the next audio sample from the sound's waveform.
 * @return Sample value multiplied by the total gain.
 */
float Sound::Generate()
{
	float sample = m_Generator->Generate(m_GeneratorParams);

	for (const auto& audioEffect : m_Effects)
		sample = audioEffect->Process(sample, m_DeltaTime);

	return sample * TotalGain();
}

/**
 * @brief Sets the sound volume.
 * @param volume Volume value, clamped between 0 and 1.
 */
void Sound::SetVolume(float volume)
{
	volume = std::clamp(volume, 0.f, 1.f);
	m_Volume = volume;
}

/**
 * @brief Retrieves the current volume.
 * @return Current volume as a float between 0 and 1.
 */
float Sound::GetVolume() const
{
	return m_Volume;
}

/**
 * @brief Retrieves the current volume in decibels.
 * @return Volume converted to decibels.
 */
float Sound::GetVolumeAsDB() const
{
	return Utilities::AsDecibels(m_Volume);
}

/**
 * @brief Mutes the sound.
 */
void Sound::Mute()
{
	m_IsMuted = true;
}

/**
 * @brief Unmutes the sound.
 */
void Sound::UnMute()
{
	m_IsMuted = false;
}

/**
 * @brief Checks if the sound is muted.
 * @return True if muted, false otherwise.
 */
bool Sound::IsMuted() const
{
	return m_IsMuted;
}

/**
 * @brief Sets the decibel level adjustment for the sound.
 * @param db Decibel value to apply.
 */
void Sound::SetDBLevel(const float db)
{
	m_DBLevel = db;
}

/**
 * @brief Retrieves the current decibel adjustment.
 * @return Current dB level.
 */
float Sound::GetDBLevel() const
{
	return m_DBLevel;
}

/**
 * @brief Converts the current dB level to a linear gain value.
 * @return Gain value corresponding to the dB level.
 */
float Sound::GetDBAsGain() const
{
	return Utilities::AsGain(m_DBLevel);
}

/**
 * @brief Computes the total gain of the sound, factoring in volume, mute state, and dB adjustment.
 * @return Combined gain value.
 */
float Sound::TotalGain() const
{
	return (m_IsMuted ? 0.f : m_Volume) * GetDBAsGain();
}

/**
 * @brief Computes the total decibel level including volume and dB adjustment.
 * @return Total dB level.
 */
float Sound::TotalDBLevel() const
{
	return m_DBLevel + GetVolumeAsDB();
}

/**
 * @brief Sets the frequency of the sound waveform.
 * @param frequency Frequency in Hz.
 */
void Sound::SetFrequency(const float frequency)
{
	m_Frequency = frequency;
	m_Generator->UpdatePhaseIncrement(m_Frequency, m_SampleRate);
}

/**
 * @brief Retrieves the current frequency.
 * @return Frequency in Hz.
 */
float Sound::GetFrequency() const
{
	return m_Frequency;
}

void Sound::SetSampleRate(const unsigned long sampleRate)
{
	if (sampleRate < 1)
	{
		std::cerr << "Sample rate must be greater than 0!\n";
		return;
	}

	m_SampleRate = sampleRate;
	m_Generator->UpdatePhaseIncrement(m_Frequency, m_SampleRate);
	m_DeltaTime = 1.0f / static_cast<float>(m_SampleRate);
}

unsigned long Sound::GetSampleRate() const
{
	return m_SampleRate;
}

/**
 * @brief Sets the pitch multiplier, affecting the playback frequency.
 * @param multiplier Multiplier for pitch adjustment.
 */
void Sound::SetPitchMultiplier(const float multiplier)
{
	m_PitchMultiplier = multiplier;
}

/**
 * @brief Retrieves the current pitch multiplier.
 * @return Current pitch multiplier.
 */
float Sound::GetPitchMultiplier() const
{
	return m_PitchMultiplier;
}

void Sound::SetGeneratorType(const GeneratorType type)
{
	m_Type = type;

	switch (m_Type)
	{
	case GeneratorType::NONE:
	case GeneratorType::SINE:
		m_Generator.reset();
		m_Generator = std::make_unique<SineGenerator>();
		break;
	case GeneratorType::NOISE:
		m_Generator.reset();
		m_Generator = std::make_unique<NoiseGenerator>();
		break;
	case GeneratorType::SAW_TOOTH:
		m_Generator.reset();
		m_Generator = std::make_unique<SawGenerator>();
		break;
	case GeneratorType::SQUARE:
		m_Generator.reset();
		m_Generator = std::make_unique<SquareGenerator>();
		break;
	}
}

GeneratorType Sound::GetGeneratorType() const
{
	return m_Type;
}

void Sound::SetAudioLength(const Duration length)
{
	m_AudioLength = length;

	for (const auto& effect : m_Effects)
		effect->OnAudioLengthChanged(m_AudioLength);
}

Duration Sound::GetAudioLength() const
{
	return m_AudioLength;
}

std::vector<std::unique_ptr<AudioEffect>>& Sound::GetEffects()
{
	return m_Effects;
}
}
