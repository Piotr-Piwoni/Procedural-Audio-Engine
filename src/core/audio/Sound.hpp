#pragma once
#include <chrono>
#include <memory>

#include "../../Utilities/TypeDefinitions.hpp"
#include "generators/GeneratorParams.hpp"
#include "generators/GeneratorType.hpp"
#include "generators/SoundGenerator.hpp"

namespace MT::Core::Audio
{
class Sound
{
public:
	explicit Sound(float volume = 1.f, float sampleRate = 1.f,
				   GeneratorType type = GeneratorType::SINE);


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

	void SetPitchMultiplier(float multiplier);
	[[nodiscard]] float GetPitchMultiplier() const;

	void SetGeneratorType(GeneratorType type);
	[[nodiscard]] GeneratorType GetGeneratorType() const;

	void SetAudioLength(Duration length);
	[[nodiscard]] Duration GetAudioLength() const;

private:
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	bool m_IsMuted{false};
	float m_SampleRate{1.f};
	float m_Frequency{440.f};
	float m_PitchMultiplier{1.f};
	std::unique_ptr<SoundGenerator> m_Generator{nullptr};
	GeneratorParams m_GeneratorParams{};
	GeneratorType m_Type{GeneratorType::NONE};
	Duration m_AudioLength{0.f};
};
}
