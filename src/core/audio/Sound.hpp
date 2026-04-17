#pragma once
#include <chrono>
#include <memory>
#include <print>

#include "../../Utilities/TypeDefinitions.hpp"
#include "effects/AudioEffect.hpp"
#include "effects/Modulator.hpp"
#include "generators/GeneratorParams.hpp"
#include "generators/GeneratorType.hpp"
#include "generators/SoundGenerator.hpp"

namespace MT::Core::Audio
{
class FadeEffect;

/**
 * @brief Represents a configurable audio signal generator with support for modulation and effects.
 *
 * The Sound class is responsible for producing audio samples in real time via the Generate() function.
 * It encapsulates a waveform generator, amplitude controls, pitch handling, modulators, and a chain
 * of audio effects.
 *
 * A Sound instance operates as follows:
 * - A generator (e.g. sine, square, noise) produces the base waveform.
 * - Frequency is determined by the base frequency, frequency offset, and pitch multiplier.
 * - Modulators dynamically influence parameters such as frequency, pitch, and amplitude over time.
 * - Effects are applied sequentially to the generated sample.
 * - Final output is scaled by volume, decibel level, and mute state.
 *
 * Key features:
 * - Multiple generator types selectable at runtime.
 * - Real-time parameter modulation via Modulator objects.
 * - Extensible effect chain using AudioEffect-derived classes.
 * - Independent control of linear volume and decibel gain.
 *
 * Typical usage:
 * - Construct a Sound with a sample rate and generator type.
 * - Configure frequency, volume, and pitch.
 * - Optionally add modulators and effects.
 * - Call Generate() repeatedly to produce audio samples.
 *
 * The class is designed for flexible synthesis and can be extended with custom generators,
 * effects, and modulation behaviours.
 */
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

	void SetFrequencyOffset(float offset);
	[[nodiscard]] float GetFrequencyOffset() const;

	void SetBaseFrequency(float frequency);
	[[nodiscard]] float GetBaseFrequency() const;

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

	void AddModulator(const Modulator& mod);
	[[nodiscard]] Modulator* GetModulator(ModulatorTarget modTarget) const;
	[[nodiscard]] std::vector<std::unique_ptr<Modulator>>& GetModulators();

private:
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	bool m_IsMuted{false};
	unsigned long m_SampleRate{1};
	float m_FrequencyOffset{0.f};
	float m_BaseFrequency{440.f};
	float m_LastFrequency{0.f};
	float m_PitchMultiplier{1.f};
	std::unique_ptr<SoundGenerator> m_Generator{nullptr};
	GeneratorParams m_GeneratorParams{};
	GeneratorType m_Type{GeneratorType::NONE};
	Duration m_AudioLength{0.f};
	std::vector<std::unique_ptr<AudioEffect>> m_Effects{};
	std::vector<std::unique_ptr<Modulator>> m_Modulators{};
	float m_DeltaTime{0.f};
};

/**
 * @brief Adds an audio effect of type T to the sound.
 * @tparam T Type of effect, must derive from AudioEffect.
 *
 * Ensures only one FadeEffect exists per Sound instance.
 * Initialises the effect with the current audio length.
 */
template<typename T>
void Sound::AddEffect()
{
	static_assert(std::is_base_of_v<AudioEffect, T>,
				  "T must derive from AudioEffect!");

	// Only one fade effects per sound object.
	if (HasEffect<FadeEffect>())
	{
		std::print("There's already a Fade Effect on the Sound!\n");
		return;
	}

	auto effect = std::make_unique<T>();
	effect->OnAudioLengthChanged(m_AudioLength);
	m_Effects.push_back(std::move(effect));
}

/**
 * @brief Retrieves an audio effect of type T if it exists.
 * @tparam T Type of effect to retrieve.
 * @return Pointer to the effect if found, otherwise nullptr.
 *
 * Uses dynamic_cast to safely identify the requested effect type.
 */
template<typename T>
T* Sound::GetEffect()
{
	for (auto& effect : m_Effects)
		if (auto casted = dynamic_cast<T*>(effect.get()))
			return casted;

	return nullptr;
}

/**
 * @brief Checks whether an effect of type T exists.
 * @tparam T Type of effect to check.
 * @return True if the effect exists, false otherwise.
 */
template<typename T>
bool Sound::HasEffect()
{
	return GetEffect<T>() != nullptr;
}
}
