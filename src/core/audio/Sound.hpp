#pragma once
#include <cstdint>
#include <mmdeviceapi.h>
#include <optional>

namespace MT::Core::Audio
{
class Sound
{
public:
	explicit Sound(float volume = 1.f, float sampleRate = 1.f);


	[[nodiscard]] float GetBuffer();

	void SetVolume(float volume);
	[[nodiscard]] float GetVolume() const;
	[[nodiscard]] float GetVolumeAsDB() const;

	void Mute();
	void UnMute();
	[[nodiscard]] bool IsMuted() const;

	void SetDBLevel(float db);
	[[nodiscard]] float GetDBLevel() const;
	[[nodiscard]] float GetDBAsGain() const;

	[[nodiscard]] float TotalGain() const;
	[[nodiscard]] float TotalDBLevel() const;

	void SetFrequency(float frequency);
	[[nodiscard]] float GetFrequency() const;

	void UpdatePhaseIncrement();

private:
	[[nodiscard]] static float GenerateSample();

private:
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	bool m_IsMuted{false};
	float m_SampleRate{1.f};
	float m_Phase{0.f};
	float m_Frequency{440.f};
	float m_PhaseIncrement{0.f};
};
}
