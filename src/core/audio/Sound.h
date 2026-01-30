#pragma once
#include <cstdint>

namespace MT::Core::Audio
{
class Sound
{
public:
	explicit Sound(float volume = 1.f);


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

private:
	[[nodiscard]] static float GenerateSample();

private:
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	bool m_IsMuted{false};
};
}
