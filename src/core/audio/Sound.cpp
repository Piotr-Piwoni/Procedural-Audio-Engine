#include "Sound.h"

#include <algorithm>
#include <random>

#include "../../Utilities/Utils.hpp"

namespace MT::Core::Audio
{
Sound::Sound(const float volume)
{
	SetVolume(volume);
}


float Sound::GetBuffer()
{
	return GenerateSample() * TotalGain();
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


float Sound::GenerateSample()
{
	static std::mt19937 gen(std::random_device{}());
	static std::uniform_real_distribution dist(-1.f, 1.0f);
	return dist(gen);
}
}
