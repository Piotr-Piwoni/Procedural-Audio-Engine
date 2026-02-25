#include "Sound.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numbers>
#include <random>

#include "../../Utilities/Utils.hpp"

namespace MT::Core::Audio
{

Sound::Sound(const float volume, const float sampleRate)
{
	if (sampleRate <= 0.f)
		throw std::invalid_argument("Sample rate must be greater than 0.");

	m_SampleRate = sampleRate;
	SetVolume(volume);
	UpdatePhaseIncrement();
}

float Sound::GetBuffer()
{
	float sample = std::sinf(m_Phase);

	m_Phase += m_PhaseIncrement;

	if (m_Phase >= 2.f * std::numbers::pi_v<float>)
		m_Phase -= 2.f * std::numbers::pi_v<float>;

	return sample * TotalGain();
	//return GenerateSample() * TotalGain();
}

void Sound::SetVolume(float volume)
{
	volume = std::clamp(volume, 0.f, 1.f);
	m_Volume = volume;
}

float Sound::GetVolume() const
{
	return m_Volume;
}

float Sound::GetVolumeAsDB() const
{
	return Utilities::AsDecibels(m_Volume);
}

void Sound::Mute()
{
	m_IsMuted = true;
}

void Sound::UnMute()
{
	m_IsMuted = false;
}

bool Sound::IsMuted() const
{
	return m_IsMuted;
}

void Sound::SetDBLevel(const float db)
{
	m_DBLevel = db;
}

float Sound::GetDBLevel() const
{
	return m_DBLevel;
}

float Sound::GetDBAsGain() const
{
	return Utilities::AsGain(m_DBLevel);
}

float Sound::TotalGain() const
{
	return (m_IsMuted ? 0.f : m_Volume) * GetDBAsGain();
}

float Sound::TotalDBLevel() const
{
	return m_DBLevel + GetVolumeAsDB();
}

void Sound::SetFrequency(const float frequency)
{
	m_Frequency = frequency;
	UpdatePhaseIncrement();
}

float Sound::GetFrequency() const
{
	return m_Frequency;
}

void Sound::UpdatePhaseIncrement()
{
	if (m_SampleRate < 1.f)
	{
		std::cerr << "Sample rate must be greater than 0!\n";
		return;
	}

	m_PhaseIncrement += 2.f * std::numbers::pi_v<float> *
			(m_Frequency / m_SampleRate);
}


float Sound::GenerateSample()
{
	static std::mt19937 gen(std::random_device{}());
	static std::uniform_real_distribution dist(-1.f, 1.0f);

	return dist(gen);
}
}
