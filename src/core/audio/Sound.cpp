#include "Sound.hpp"

#include <algorithm>
#include <iostream>
#include <numbers>
#include <random>

#include "../../Utilities/Utils.hpp"

namespace MT::Core::Audio
{

/**
 * @brief Constructs a Sound object with specified volume and sample rate.
 * @param volume Initial volume, clamped between 0 and 1.
 * @param sampleRate Sample rate in Hz; must be greater than 0.
 * @throws std::invalid_argument If sampleRate is <= 0.
 */
Sound::Sound(const float volume, const float sampleRate)
{
	if (sampleRate <= 0.f)
		throw std::invalid_argument("Sample rate must be greater than 0.");

	m_SampleRate = sampleRate;
	SetVolume(volume);
	UpdatePhaseIncrement();
}

/**
 * @brief Generates the next audio sample from the sound's waveform.
 * @return Sample value multiplied by the total gain.
 */
float Sound::GetBuffer()
{
	float sample = std::sinf(m_Phase);

	m_Phase += m_PhaseIncrement;

	if (m_Phase >= 2.f * std::numbers::pi_v<float>)
		m_Phase -= 2.f * std::numbers::pi_v<float>;

	return sample * TotalGain();
	//return GenerateSample() * TotalGain();
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
	UpdatePhaseIncrement();
}

/**
 * @brief Retrieves the current frequency.
 * @return Frequency in Hz.
 */
float Sound::GetFrequency() const
{
	return m_Frequency;
}

/**
 * @brief Updates the phase increment based on current frequency and sample rate.
 * This controls the progression of the waveform phase for sample generation.
 */
void Sound::UpdatePhaseIncrement()
{
	if (m_SampleRate < 1.f)
	{
		std::cerr << "Sample rate must be greater than 0!\n";
		return;
	}

	m_PhaseIncrement = 2.f * std::numbers::pi_v<float> *
					   (m_Frequency / m_SampleRate);
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


/**
 * @brief Generates a random sample in the range [-1, 1].
 * @return Random float between -1 and 1.
 */
float Sound::GenerateSample()
{
	static std::mt19937 gen(std::random_device{}());
	static std::uniform_real_distribution dist(-1.f, 1.0f);

	return dist(gen);
}
}
