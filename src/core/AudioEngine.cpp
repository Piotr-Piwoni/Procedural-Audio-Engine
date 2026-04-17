#include "AudioEngine.hpp"

#include <algorithm>

namespace MT::Core
{
bool AudioEngine::Init()
{
	m_Backend = std::make_unique<Audio::AudioBackend>();
	const Audio::BackendError result = m_Backend->Create(COINIT_MULTITHREADED);

	if (result != Audio::BackendError::NONE)
	{
		std::println("Audio backend failed to be created, "
					 "the reason {} error!\n", to_string(result));
		return false;
	}

	// Output some information about the format.
	if (const auto format = m_Backend->GetFormat())
	{
		std::println("Channels: {}\nSampleRate: {}\nBits per sample: {}",
					 format->nChannels, format->nSamplesPerSec,
					 format->wBitsPerSample);

		m_SampleRate = format->nSamplesPerSec;
		m_Channels = format->nChannels;
	}

	return true;
}

void AudioEngine::Update()
{
	Audio::AudioBuffer buffer{};
	if (!IsValidToRun(buffer)) return;

	const auto maxFrames = static_cast<uint64_t>(
		static_cast<float>(m_SampleRate) * m_AudioLength.count());

	// Stop playback if we've reached the end of the audio clip.
	if (!m_IsSoundContinues && m_FramesGenerated >= maxFrames)
	{
		m_Backend->ReleaseBuffer(buffer.Frames);
		m_Backend->StopPlayback();
		return;
	}

	// Determine how long to write to the audio buffer based on the audio
	// clip's length.
	uint32_t framesToWrite = buffer.Frames;
	if (!m_IsSoundContinues && m_FramesGenerated + buffer.Frames > maxFrames)
		framesToWrite = static_cast<uint32_t>(maxFrames - m_FramesGenerated);

	// Generate noise.
	for (uint32_t frame = 0; frame < framesToWrite; frame++)
	{
		float mixedSample = 0.f;
		for (auto& sound : m_Sounds)
			mixedSample += sound.Generate();
		mixedSample *= m_MasterVolume;

		buffer.Data[frame * m_Channels + 0] = mixedSample;
		if (m_Channels > 1)
			buffer.Data[frame * m_Channels + 1] = mixedSample;
	}

	// Push to the audio thread.
	if (!m_IsSoundContinues) m_FramesGenerated += framesToWrite;
	m_Backend->ReleaseBuffer(buffer.Frames);
}


void AudioEngine::PlayAudio()
{
	m_FramesGenerated = 0;

	// Reset all effects on the sounds.
	for (auto& sound : m_Sounds)
		for (const auto& effect : sound.GetEffects())
			effect->Reset();

	m_Backend->StartPlayback();
}

void AudioEngine::StopAudio() const
{
	m_Backend->StopPlayback();
}

void AudioEngine::TogglePlayback()
{
	const auto state = m_Backend->GetPlaybackState();
	if (state == Audio::PlaybackState::PLAYING) m_Backend->StopPlayback();
	else if (state == Audio::PlaybackState::STOPPED) PlayAudio();
}

void AudioEngine::PauseAudio() const
{
	m_Backend->PausePlayback();
}

void AudioEngine::UnpauseAudio() const
{
	m_Backend->ResumePlayback();
}

void AudioEngine::TogglePause() const
{
	const auto state = m_Backend->GetPlaybackState();
	if (state == Audio::PlaybackState::PLAYING) m_Backend->PausePlayback();
	else if (state == Audio::PlaybackState::PAUSED) m_Backend->ResumePlayback();
}


bool AudioEngine::HasSounds() const { return !m_Sounds.empty(); }
std::deque<Audio::Sound>& AudioEngine::GetSounds() { return m_Sounds; }

Audio::Sound& AudioEngine::CreateSound(const float volume,
									   const Audio::GeneratorType type)
{
	auto& sound = m_Sounds.emplace_back(volume, m_SampleRate, type);
	return sound;
}

std::vector<Audio::Sound*> AudioEngine::CreateSounds(
		const int amount, float volume, Audio::GeneratorType type)
{
	std::vector<Audio::Sound*> createdSounds;
	createdSounds.reserve(amount);

	for (int i = 0; i < amount; i++)
	{
		auto& sound = m_Sounds.emplace_back(volume, m_SampleRate, type);
		createdSounds.push_back(&sound);
	}

	return createdSounds;
}

void AudioEngine::AddSound(Audio::Sound sound)
{
	m_Sounds.push_back(std::move(sound));
}

void AudioEngine::AddSounds(std::vector<Audio::Sound> sounds)
{
	for (auto& sound : sounds)
		m_Sounds.push_back(std::move(sound));
}

void AudioEngine::RemoveSound()
{
	if (HasSounds()) m_Sounds.pop_back();
}

void AudioEngine::RemoveSound(const size_t index)
{
	if (!HasSounds()) return;

	if (index >= m_Sounds.size())
	{
		std::print("The provided index exceeds the number of "
				"sounds available!\n");
		return;
	}
	m_Sounds.erase(m_Sounds.begin() + index);
}

void AudioEngine::RemoveSounds(const size_t start, const size_t end)
{
	if (!HasSounds()) return;

	if (start > end || end >= m_Sounds.size())
	{
		std::print("The provided indexes exceeds the number of "
				"sounds available!\n");
		return;
	}
	// Range based remove, inclusively.
	m_Sounds.erase(m_Sounds.begin() + start, m_Sounds.begin() + end + 1);
}


void AudioEngine::SetAudioLength(const Duration length)
{
	m_AudioLength = length;
	for (auto& sound : m_Sounds)
	{
		if (m_IsSoundContinues) sound.SetAudioLength(Duration(0.f));
		else sound.SetAudioLength(m_AudioLength);
	}
}

Duration AudioEngine::GetAudioLength() const { return m_AudioLength; }


bool AudioEngine::AreSoundsContinues() const { return m_IsSoundContinues; }

void AudioEngine::SetSoundsContinues(const bool val)
{
	m_IsSoundContinues = val;
	for (auto& sound : m_Sounds)
	{
		if (m_IsSoundContinues) sound.SetAudioLength(Duration(0.f));
		else sound.SetAudioLength(m_AudioLength);
	}
}


float AudioEngine::GetVolume() const { return m_MasterVolume; }

void AudioEngine::SetVolume(const float volume)
{
	m_MasterVolume = std::clamp(volume, 0.f, 1.f);
}


unsigned long AudioEngine::GetSampleRate() const { return m_SampleRate; }
unsigned short AudioEngine::GetChannels() const { return m_Channels; }

const WAVEFORMATEX& AudioEngine::GetFormat() const
{
	return *m_Backend->GetFormat();
}

Audio::PlaybackState AudioEngine::GetState() const
{
	return m_Backend->GetPlaybackState();
}


bool AudioEngine::IsValidToRun(Audio::AudioBuffer& audioBuffer)
{
	const WAVEFORMATEX* format = m_Backend->GetFormat();
	if (!format) return false;

	// Update sample rate if it has changed.
	if (m_SampleRate != format->nSamplesPerSec)
	{
		m_SampleRate = format->nSamplesPerSec;
		for (auto& sound : m_Sounds)
			sound.SetSampleRate(m_SampleRate);
	}
	// Update number of channels if it has changed.
	if (m_Channels != format->nChannels)
		m_Channels = format->nChannels;

	if (m_Backend->GetPlaybackState() != Audio::PlaybackState::PLAYING)
		return false;

	const auto buffer = m_Backend->GetBuffer();
	if (!buffer.Data) return false;

	if (buffer.Frames == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return false;
	}

	audioBuffer = buffer;
	return true;
}
}
