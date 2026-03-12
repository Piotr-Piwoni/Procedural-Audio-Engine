#pragma once
#include <chrono>
#include <memory>

#include "../../Utilities/TypeDefinitions.hpp"
#include "effects/AudioEffect.hpp"
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


	[[nodiscard]] float Generate();

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

	template<typename T>
	void AddEffect();
	std::vector<std::unique_ptr<AudioEffect>>& GetEffects();

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
	std::vector<std::unique_ptr<AudioEffect>> m_Effects{};
};

template<typename T>
void Sound::AddEffect()
{
	static_assert(std::is_base_of_v<AudioEffect, T>,
				  "T must derive from AudioEffect!");

	auto effect = std::make_unique<T>();
	effect->OnAudioLengthChanged(m_AudioLength);
	m_Effects.push_back(std::move(effect));
}
}
