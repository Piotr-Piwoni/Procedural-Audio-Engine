#include "AudioEngine.hpp"

#include <algorithm>

namespace MT::Core
{
/**
 * @brief Initialises the audio engine and underlying backend.
 * @return True if initialisation succeeds, false otherwise.
 *
 * Creates the audio backend, initialises it, and retrieves the audio format.
 * Stores sample rate and channel count for later use.
 */
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

/**
 * @brief Updates the audio engine and fills the output buffer.
 *
 * Validates the backend state and retrieves a writable audio buffer. Generates
 * mixed audio samples from all active sounds, applies master volume, and writes
 * the result across all output channels.
 *
 * For non-continuous playback:
 * - Limits generation based on the configured audio length.
 * - Stops playback when the total frame count is reached.
 *
 * The function also tracks generated frames and ensures the backend buffer
 * is always released after processing.
 */
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

		// Write to all channels.
		const uint32_t baseIndex = frame * m_Channels;
		for (uint32_t channel = 0; channel < m_Channels; channel++)
			buffer.Data[baseIndex + channel] = mixedSample;
	}

	// Push to the audio thread.
	if (!m_IsSoundContinues) m_FramesGenerated += framesToWrite;
	m_Backend->ReleaseBuffer(buffer.Frames);
}

/**
 * @brief Starts audio playback.
 *
 * Resets frame counters and all effects on active sounds before starting playback.
 */
void AudioEngine::PlayAudio()
{
	m_FramesGenerated = 0;

	// Reset all effects on the sounds.
	for (auto& sound : m_Sounds)
		for (const auto& effect : sound.GetEffects())
			effect->Reset();

	m_Backend->StartPlayback();
}

/**
 * @brief Stops audio playback.
 */
void AudioEngine::StopAudio() const
{
	m_Backend->StopPlayback();
}

/**
 * @brief Toggles playback state between playing and stopped.
 */
void AudioEngine::TogglePlayback()
{
	const auto state = m_Backend->GetPlaybackState();
	if (state == Audio::PlaybackState::PLAYING) m_Backend->StopPlayback();
	else if (state == Audio::PlaybackState::STOPPED) PlayAudio();
}

/**
 * @brief Pauses audio playback.
 */
void AudioEngine::PauseAudio() const
{
	m_Backend->PausePlayback();
}

/**
 * @brief Resumes audio playback if paused.
 */
void AudioEngine::UnpauseAudio() const
{
	m_Backend->ResumePlayback();
}

/**
 * @brief Toggles pause state between paused and playing.
 */
void AudioEngine::TogglePause() const
{
	const auto state = m_Backend->GetPlaybackState();
	if (state == Audio::PlaybackState::PLAYING) m_Backend->PausePlayback();
	else if (state == Audio::PlaybackState::PAUSED) m_Backend->ResumePlayback();
}

/**
 * @brief Checks if any sounds are currently managed.
 * @return True if at least one sound exists, false otherwise.
 */
bool AudioEngine::HasSounds() const { return !m_Sounds.empty(); }

/**
 * @brief Provides access to the list of sounds.
 * @return Reference to the internal deque of sounds.
 */
std::deque<Audio::Sound>& AudioEngine::GetSounds() { return m_Sounds; }

/**
 * @brief Creates and adds a new sound.
 * @param volume Initial volume.
 * @param type Generator type.
 * @return Reference to the created sound.
 */
Audio::Sound& AudioEngine::CreateSound(const float volume,
									   const Audio::GeneratorType type)
{
	auto& sound = m_Sounds.emplace_back(volume, m_SampleRate, type);
	return sound;
}

/**
 * @brief Creates multiple sounds.
 * @param amount Number of sounds to create.
 * @param volume Initial volume for each sound.
 * @param type Generator type.
 * @return Vector of pointers to created sounds.
 */
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

/**
 * @brief Adds a sound to the engine.
 * @param sound Sound to add (moved into storage).
 */
void AudioEngine::AddSound(Audio::Sound sound)
{
	m_Sounds.push_back(std::move(sound));
}

/**
 * @brief Adds multiple sounds to the engine.
 * @param sounds Collection of sounds to add (moved into storage).
 */
void AudioEngine::AddSounds(std::vector<Audio::Sound> sounds)
{
	for (auto& sound : sounds)
		m_Sounds.push_back(std::move(sound));
}

/**
 * @brief Removes the last sound from the engine.
 */
void AudioEngine::RemoveSound()
{
	if (HasSounds()) m_Sounds.pop_back();
}

/**
 * @brief Removes a sound at a specific index.
 * @param index Index of the sound to remove.
 */
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

/**
 * @brief Removes a range of sounds.
 * @param start Starting index (inclusive).
 * @param end Ending index (inclusive).
 */
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

/**
 * @brief Sets the total audio length.
 * @param length Duration of playback.
 *
 * Updates all sounds with the new length, or sets to zero if continuous mode is enabled.
 */
void AudioEngine::SetAudioLength(const Duration length)
{
	m_AudioLength = length;
	for (auto& sound : m_Sounds)
	{
		if (m_IsSoundContinues) sound.SetAudioLength(Duration(0.f));
		else sound.SetAudioLength(m_AudioLength);
	}
}

/**
 * @brief Retrieves the current audio length.
 * @return Duration of playback.
 */
Duration AudioEngine::GetAudioLength() const { return m_AudioLength; }

/**
 * @brief Checks if sounds are set to continuous playback.
 * @return True if continuous, false otherwise.
 */
bool AudioEngine::AreSoundsContinues() const { return m_IsSoundContinues; }

/**
 * @brief Enables or disables continuous playback.
 * @param val True for continuous playback, false otherwise.
 *
 * Updates all sounds accordingly.
 */
void AudioEngine::SetSoundsContinues(const bool val)
{
	m_IsSoundContinues = val;
	for (auto& sound : m_Sounds)
	{
		if (m_IsSoundContinues) sound.SetAudioLength(Duration(0.f));
		else sound.SetAudioLength(m_AudioLength);
	}
}

/**
 * @brief Retrieves the master volume.
 * @return Volume value between 0 and 1.
 */
float AudioEngine::GetVolume() const { return m_MasterVolume; }

/**
 * @brief Sets the master volume.
 * @param volume Volume value clamped between 0 and 1.
 */
void AudioEngine::SetVolume(const float volume)
{
	m_MasterVolume = std::clamp(volume, 0.f, 1.f);
}

/**
 * @brief Retrieves the current sample rate.
 * @return Sample rate in Hz.
 */
unsigned long AudioEngine::GetSampleRate() const { return m_SampleRate; }

/**
 * @brief Retrieves the number of audio channels.
 * @return Number of channels.
 */
unsigned short AudioEngine::GetChannels() const { return m_Channels; }

/**
 * @brief Retrieves the current audio format.
 * @return Reference to WAVEFORMATEX structure.
 */
const WAVEFORMATEX& AudioEngine::GetFormat() const
{
	return *m_Backend->GetFormat();
}

/**
 * @brief Retrieves the current playback state.
 * @return Playback state enum.
 */
Audio::PlaybackState AudioEngine::GetState() const
{
	return m_Backend->GetPlaybackState();
}

/**
 * @brief Validates whether the engine is ready to generate audio.
 * @param audioBuffer Output buffer to populate if valid.
 * @return True if ready to run, false otherwise.
 *
 * Ensures backend format is valid, updates sample rate and channels if needed,
 * checks playback state, and retrieves a valid buffer for writing.
 */
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
