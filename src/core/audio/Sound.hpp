#pragma once
#include <chrono>
#include <memory>
#include <print>

#include "../../Utilities/TypeDefinitions.hpp"
#include "effects/AudioEffect.hpp"
#include "effects/Modulator.hpp"
#include "generators/GeneratorType.hpp"
#include "generators/SoundGenerator.hpp"

namespace MT::Core::Audio
{
class FadeEffect;

/**
 * @brief Represents a configurable audio signal generator with modulation, effects, and modulation chains.
 *
 * The Sound class is a real-time audio synthesis unit that generates waveform samples using a selectable
 * generator, then processes them through modulators and an effect chain before producing final output.
 *
 * Audio pipeline overview:
 * - A SoundGenerator produces the base waveform (sine, square, noise, saw, etc.).
 * - Frequency is derived from base frequency, frequency offset, and pitch multiplier.
 * - Modulators dynamically influence frequency, pitch, amplitude, or decibel levels per sample.
 * - Audio effects are applied sequentially after waveform generation.
 * - Final output is scaled by volume, dB gain, and mute state.
 *
 * Modulation system:
 * - Each Sound instance can hold multiple Modulator objects.
 * - Each Modulator targets a specific parameter (frequency, pitch, amplitude, or dB).
 * - Only one Modulator per target type is permitted.
 * - Modulators are evaluated per generated sample for real-time parameter changes.
 *
 * Effect system:
 * - Supports a chain of AudioEffect-derived objects.
 * - Only one FadeEffect instance is allowed per Sound.
 * - Effects are applied in insertion order after waveform generation.
 *
 * Design features:
 * - Runtime switchable waveform generators.
 * - Independent linear and decibel-based amplitude control.
 * - Smooth parameter interpolation for volume and dB changes.
 * - Modular extensibility for generators, effects, and modulators.
 *
 * Typical usage:
 * - Construct Sound with sample rate and generator type.
 * - Configure frequency, pitch, and volume.
 * - Optionally attach modulators and effects.
 * - Call Generate() continuously to produce audio samples.
 *
 * This class is intended for procedural audio synthesis, sound design tools,
 * and real-time audio systems requiring flexible modulation and effect control.
 */
class Sound
{
public:
	explicit Sound(float volume = 1.f, unsigned long sampleRate = 1,
				   GeneratorType type = GeneratorType::SINE);
	Sound(const Sound&) = delete;
	Sound& operator=(const Sound&) = delete;

	Sound(Sound&&) noexcept = default;
	Sound& operator=(Sound&&) noexcept = default;


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
	[[nodiscard]] SoundGenerator* GetGenerator() const;

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
	float m_TargetVolume = 0.f;
	float m_Volume{0.f};
	float m_DBLevel{0.f};
	float m_TargetDB = 0.f;
	bool m_IsMuted{false};
	Duration m_AudioLength{0.f};

	float m_BaseFrequency{440.f};
	float m_FrequencyOffset{0.f};
	float m_LastFrequency{0.f};

	unsigned long m_SampleRate{1};
	float m_PitchMultiplier{1.f};
	float m_DeltaTime{0.f};

	std::unique_ptr<SoundGenerator> m_Generator{nullptr};
	GeneratorType m_GeneratorType{GeneratorType::NONE};

	std::vector<std::unique_ptr<AudioEffect>> m_Effects{};
	std::vector<std::unique_ptr<Modulator>> m_Modulators{};

	bool m_Generating{false};
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
