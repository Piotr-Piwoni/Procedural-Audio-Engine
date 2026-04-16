#pragma once
#include <chrono>
#include <memory>
#include <print>

#include "../../Utilities/TypeDefinitions.hpp"
#include "effects/AudioEffect.hpp"
#include "generators/GeneratorParams.hpp"
#include "generators/GeneratorType.hpp"
#include "generators/SoundGenerator.hpp"


namespace MT::Core::Audio
{
class FadeEffect;


class Sound
{
public:
	explicit Sound(float volume = 1.f, unsigned long sampleRate = 1,
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

	void SetSampleRate(unsigned long sampleRate);
	[[nodiscard]] unsigned long GetSampleRate() const;

	void SetPitchMultiplier(float multiplier);
	[[nodiscard]] float GetPitchMultiplier() const;

	void SetGeneratorType(GeneratorType type);
	[[nodiscard]] GeneratorType GetGeneratorType() const;

	void SetAudioLength(Duration length);
	[[nodiscard]] Duration GetAudioLength() const;

	template<typename T>
	void AddEffect();
	template<typename T>
	[[nodiscard]] T* GetEffect();
	[[nodiscard]] std::vector<std::unique_ptr<AudioEffect>>& GetEffects();
	template<typename T>
	[[nodiscard]] bool HasEffect();

private:
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	bool m_IsMuted{false};
	unsigned long m_SampleRate{1};
	float m_Frequency{440.f};
	float m_PitchMultiplier{1.f};
	std::unique_ptr<SoundGenerator> m_Generator{nullptr};
	GeneratorParams m_GeneratorParams{};
	GeneratorType m_Type{GeneratorType::NONE};
	Duration m_AudioLength{0.f};
	std::vector<std::unique_ptr<AudioEffect>> m_Effects{};
	float m_DeltaTime{0.f};
};

template<typename T>
void Sound::AddEffect()
{
	static_assert(std::is_base_of_v<AudioEffect, T>,
				  "T must derive from AudioEffect!");

	// Only one fade effects per sound object.
	if (HasEffect<FadeEffect>())return;

	auto effect = std::make_unique<T>();
	effect->OnAudioLengthChanged(m_AudioLength);
	m_Effects.push_back(std::move(effect));
}

template<typename T>
T* Sound::GetEffect()
{
	for (auto& effect : m_Effects)
		if (auto casted = dynamic_cast<T*>(effect.get()))
			return casted;

	std::print("Chosen effect could not be found!");
	return nullptr;
}

template<typename T>
bool Sound::HasEffect()
{
	return GetEffect<T>() != nullptr;
}
}
