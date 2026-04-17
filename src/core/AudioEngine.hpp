#pragma once
#include <deque>
#include <thread>

#include "audio/AudioBackend.hpp"
#include "audio/Sound.hpp"

namespace MT::Core
{
/**
 * @brief Manages real-time audio playback, mixing, and sound lifecycle.
 *
 * The AudioEngine class acts as the central coordinator for audio output. It
 * interfaces with the underlying AudioBackend to stream audio data, manages a
 * collection of Sound objects, and handles playback control, mixing, and timing.
 *
 * Core responsibilities:
 * - Initialise and communicate with the audio backend.
 * - Generate and mix audio samples from multiple Sound instances.
 * - Stream mixed audio into the backend buffer for playback.
 * - Control playback state (play, stop, pause, resume).
 * - Manage sound creation, storage, and removal.
 *
 * Audio pipeline:
 * - Each Sound generates its own audio samples.
 * - Samples from all active sounds are summed (mixed).
 * - The mixed signal is scaled by the master volume.
 * - The result is written to all output channels.
 *
 * Playback behaviour:
 * - Supports both finite and continuous playback modes.
 * - In finite mode, playback stops once the configured audio length is reached.
 * - In continuous mode, audio loops indefinitely without frame limits.
 *
 * Additional features:
 * - Automatically adapts to backend format changes (sample rate, channels).
 * - Propagates sample rate updates to all managed Sound objects.
 * - Resets sound effects when playback starts.
 *
 * Typical usage:
 * - Call Init() to initialise the backend.
 * - Create or add Sound objects.
 * - Configure playback settings (length, volume, continuous mode).
 * - Call PlayAudio() and repeatedly call Update() in a loop.
 *
 * This class is designed for real-time audio systems and procedural sound
 * generation where multiple sound sources must be mixed efficiently.
 */
class AudioEngine
{
public:
	AudioEngine() = default;

	[[nodiscard]] bool Init();
	void Update();

	void PlayAudio();
	void StopAudio() const;
	void TogglePlayback();
	void PauseAudio() const;
	void UnpauseAudio() const;
	void TogglePause() const;

	[[nodiscard]] bool HasSounds() const;
	[[nodiscard]] std::deque<Audio::Sound>& GetSounds();

	Audio::Sound& CreateSound(
			float volume = 1.f,
			Audio::GeneratorType type = Audio::GeneratorType::SINE);
	std::vector<Audio::Sound*> CreateSounds(
			int amount, float volume = 1.f,
			Audio::GeneratorType type = Audio::GeneratorType::SINE);

	void AddSound(Audio::Sound sound);
	void AddSounds(std::vector<Audio::Sound> sounds);
	void RemoveSound();
	void RemoveSound(size_t index);
	void RemoveSounds(size_t start, size_t end);

	void SetAudioLength(Duration length);
	[[nodiscard]] Duration GetAudioLength() const;

	[[nodiscard]] bool AreSoundsContinues() const;
	void SetSoundsContinues(bool val);

	[[nodiscard]] float GetVolume() const;
	void SetVolume(float volume);

	[[nodiscard]] unsigned long GetSampleRate() const;
	[[nodiscard]] unsigned short GetChannels() const;
	[[nodiscard]] const WAVEFORMATEX& GetFormat() const;
	[[nodiscard]] Audio::PlaybackState GetState() const;

private:
	bool IsValidToRun(Audio::AudioBuffer& audioBuffer);

private:
	std::unique_ptr<Audio::AudioBackend> m_Backend{nullptr};
	uint32_t m_FramesGenerated{0};
	unsigned long m_SampleRate{0};
	unsigned short m_Channels{0};

	float m_MasterVolume{1.f};
	Duration m_AudioLength{5.f};

	std::deque<Audio::Sound> m_Sounds{};
	bool m_IsSoundContinues{false};
};
}
