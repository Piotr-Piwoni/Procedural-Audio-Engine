#pragma once
#include <deque>
#include <thread>

#include "audio/AudioBackend.hpp"
#include "audio/Sound.hpp"

namespace MT::Core
{
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
