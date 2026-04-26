#include "Sound.hpp"

#include <algorithm>
#include <iostream>

#include "../../Utilities/Utils.hpp"
#include "effects/FadeEffect.hpp"
#include "generators/NoiseGenerator.hpp"
#include "generators/SawGenerator.hpp"
#include "generators/SineGenerator.hpp"
#include "generators/SquareGenerator.hpp"

namespace MT::Core::Audio
{
/**
 * @brief Constructs a Sound object with specified volume and sample rate.
 * @param volume Initial volume, clamped between 0 and 1.
 * @param sampleRate Sample rate in Hz; must be greater than 0.
 * @param type Type of the sound generator used.
 * @throws std::invalid_argument If sampleRate is <= 0.
 */
Sound::Sound(const float volume, const unsigned long sampleRate,
			 const GeneratorType type)
{
	if (sampleRate < 1)
		throw std::invalid_argument("Sample rate must be greater than 0.");

	m_Effects.reserve(5);

	SetGeneratorType(type);
	SetSampleRate(sampleRate);

	m_TargetVolume = volume;
	m_Volume = volume;

	m_TargetDB = 0.f;
	m_DBLevel = 0.f;
}


/**
 * @brief Generates the next audio sample from the sound's waveform.
 *
 * Applies smooth transitions for volume and dB level, evaluates all active modulators,
 * updates generator frequency when needed, processes the waveform, and applies all
 * active audio effects before returning the final mixed sample.
 *
 * @return Final processed audio sample.
 */
float Sound::Generate()
{
	m_Generating = true;

	constexpr float smoothing = 0.0001f;
	m_Volume += (m_TargetVolume - m_Volume) * smoothing;
	m_DBLevel += (m_TargetDB - m_DBLevel) * smoothing;

	float effectiveFrequency = m_BaseFrequency;
	float pitchMult = m_PitchMultiplier;
	float volume = m_Volume;
	float db = m_DBLevel;

	// Apply modulators.
	for (const auto& modulator : m_Modulators)
	{
		const auto modSample = modulator->Process(m_DeltaTime);
		switch (modulator->GetTarget())
		{
		case ModulatorTarget::FREQUENCY:
		{
			effectiveFrequency *= 1.f + std::clamp(modSample, -1.f, 1.f);
			break;
		}
		case ModulatorTarget::PITCH:
		{
			pitchMult *= 1.f + modSample;
			break;
		}
		case ModulatorTarget::AMPLITUDE_GAIN:
		{
			volume = std::clamp(volume * (1.f + modSample), 0.f, 1.f);
			break;
		}
		case ModulatorTarget::AMPLITUDE_DECIBEL:
		{
			db += modSample;
			break;
		}
		}
	}

	effectiveFrequency = (effectiveFrequency + m_FrequencyOffset) * pitchMult;
	if (std::abs(effectiveFrequency - m_LastFrequency) > 0.0001f)
	{
		m_Generator->UpdatePhaseIncrement(effectiveFrequency, m_SampleRate);
		m_LastFrequency = effectiveFrequency;
	}

	float sample = m_Generator->Generate();

	// Apply effects.
	for (const auto& audioEffect : m_Effects)
		sample = audioEffect->Process(sample, m_DeltaTime);

	m_Generating = false;
	return sample * (m_IsMuted ? 0.f : volume) * Utilities::AsGain(db);
}

/**
 * @brief Sets the sound volume.
 * @param volume Volume value, clamped between 0 and 1.
 */
void Sound::SetVolume(float volume)
{
	m_TargetVolume = std::clamp(volume, 0.f, 1.f);
	if (!m_Generating) m_Volume = m_TargetVolume;
}

/**
 * @brief Retrieves the current volume.
 * @return Current volume as a float between 0 and 1.
 */
float Sound::GetVolume() const
{
	return m_Volume;
}

/**
 * @brief Retrieves the current volume in decibels.
 * @return Volume converted to decibels.
 */
float Sound::GetVolumeAsDB() const
{
	return Utilities::AsDecibels(m_Volume);
}

/**
 * @brief Mutes the sound.
 */
void Sound::Mute()
{
	m_IsMuted = true;
}

/**
 * @brief Unmutes the sound.
 */
void Sound::UnMute()
{
	m_IsMuted = false;
}

/**
 * @brief Checks if the sound is muted.
 * @return True if muted, false otherwise.
 */
bool Sound::IsMuted() const
{
	return m_IsMuted;
}

/**
 * @brief Sets the decibel level adjustment for the sound.
 * @param db Decibel value to apply.
 */
void Sound::SetDBLevel(float db)
{
	m_TargetDB = db;
	if (!m_Generating) m_DBLevel = m_TargetDB;
}

/**
 * @brief Retrieves the current decibel adjustment.
 * @return Current dB level.
 */
float Sound::GetDBLevel() const
{
	return m_DBLevel;
}

/**
 * @brief Converts the current dB level to a linear gain value.
 * @return Gain value corresponding to the dB level.
 */
float Sound::GetDBAsGain() const
{
	return Utilities::AsGain(m_DBLevel);
}

/**
 * @brief Computes the total gain of the sound, factoring in volume, mute state, and dB adjustment.
 * @return Combined gain value.
 */
float Sound::TotalGain() const
{
	return (m_IsMuted ? 0.f : m_Volume) * GetDBAsGain();
}

/**
 * @brief Computes the total decibel level including volume and dB adjustment.
 * @return Total dB level.
 */
float Sound::TotalDBLevel() const
{
	return m_DBLevel + GetVolumeAsDB();
}

/**
 * @brief Sets the frequency offset of the sound waveform.
 * @param offset Offset in Hz. Can be positive or negative.
 */
void Sound::SetFrequencyOffset(const float offset)
{
	m_FrequencyOffset = offset;
}

/**
 * @brief Retrieves the current frequency offset.
 * @return Frequency offset in Hz.
 */
float Sound::GetFrequencyOffset() const
{
	return m_FrequencyOffset;
}

/**
 * @brief Sets the base frequency of the sound.
 * @param frequency Frequency in Hz; values below 0 are clamped to 0.
 */
void Sound::SetBaseFrequency(const float frequency)
{
	m_BaseFrequency = std::max(0.f, frequency);
}

/**
 * @brief Retrieves the base frequency of the sound.
 * @return Base frequency in Hz.
 */
float Sound::GetBaseFrequency() const
{
	return m_BaseFrequency;
}

/**
 * @brief Retrieve the effective frequency calculated by offsetting the
 * <b>base frequency</b> with the <b>frequency offset</b> and multiplying but
 * the <b>pitch multiplier</b>.
 * @return Effective frequency in Hz.
 */
float Sound::GetFrequency() const
{
	return (m_BaseFrequency + m_FrequencyOffset) * m_PitchMultiplier;
}

/**
 * @brief Sets the sample rate used for audio generation.
 * @param sampleRate Sample rate in Hz; must be greater than 0.
 *
 * Also updates the internal delta time used for time-based processing.
 */
void Sound::SetSampleRate(const unsigned long sampleRate)
{
	if (sampleRate < 1)
	{
		std::cerr << "Sample rate must be greater than 0!\n";
		return;
	}

	m_SampleRate = sampleRate;
	m_DeltaTime = 1.0f / static_cast<float>(m_SampleRate);
}

/**
 * @brief Retrieves the current sample rate.
 * @return Sample rate in Hz.
 */
unsigned long Sound::GetSampleRate() const
{
	return m_SampleRate;
}

/**
 * @brief Sets the pitch multiplier, affecting the playback frequency.
 * @param multiplier Multiplier for pitch adjustment.
 */
void Sound::SetPitchMultiplier(const float multiplier)
{
	m_PitchMultiplier = multiplier;
}

/**
 * @brief Retrieves the current pitch multiplier.
 * @return Current pitch multiplier.
 */
float Sound::GetPitchMultiplier() const
{
	return m_PitchMultiplier;
}

/**
 * @brief Sets the generator type used to produce the waveform.
 * @param type Generator type enum value.
 *
 * Replaces the current generator instance with a new one matching the type.
 */
void Sound::SetGeneratorType(const GeneratorType type)
{
	m_GeneratorType = type;

	switch (m_GeneratorType)
	{
	case GeneratorType::NONE:
	case GeneratorType::SINE:
		m_Generator.reset();
		m_Generator = std::make_unique<SineGenerator>();
		break;
	case GeneratorType::NOISE:
		m_Generator.reset();
		m_Generator = std::make_unique<NoiseGenerator>();
		break;
	case GeneratorType::SAW_TOOTH:
		m_Generator.reset();
		m_Generator = std::make_unique<SawGenerator>();
		break;
	case GeneratorType::SQUARE:
		m_Generator.reset();
		m_Generator = std::make_unique<SquareGenerator>();
		break;
	}
}

/**
 * @brief Retrieves the current generator type.
 * @return Generator type enum value.
 */
GeneratorType Sound::GetGeneratorType() const
{
	return m_GeneratorType;
}

SoundGenerator* Sound::GetGenerator() const
{
	return m_Generator.get();
}

/**
 * @brief Sets the total audio length for the sound.
 * @param length Duration of the audio.
 *
 * Notifies all effects of the updated audio length.
 */
void Sound::SetAudioLength(const Duration length)
{
	m_AudioLength = length;

	for (const auto& effect : m_Effects)
		effect->OnAudioLengthChanged(m_AudioLength);
}

/**
 * @brief Retrieves the current audio length.
 * @return Audio duration.
 */
Duration Sound::GetAudioLength() const
{
	return m_AudioLength;
}

/**
 * @brief Provides access to the list of audio effects.
 * @return Reference to the vector of audio effects.
 */
std::vector<std::unique_ptr<AudioEffect>>& Sound::GetEffects()
{
	return m_Effects;
}

/**
 * @brief Adds a modulator to the sound object.
 * @param mod Modulator to add (copied into internal storage).
 *
 * Ensures only one modulator per target type exists. If a modulator for the same target
 * already exists, the function returns early and does not add a duplicate.
 */
void Sound::AddModulator(const Modulator& mod)
{
	m_Modulators.push_back(std::make_unique<Modulator>(mod));
}

/**
 * @brief Retrieves a modulator by its target type.
 * @param modTarget Target type of the modulator.
 * @return Pointer to the modulator if found, otherwise nullptr.
 *
 * Searches through all stored modulators and returns the first match. If no modulator
 * is found for the requested target, nullptr is returned and a diagnostic message is printed.
 */
Modulator* Sound::GetModulator(const ModulatorTarget modTarget) const
{
	for (const auto& modulator : m_Modulators)
		if (modulator->GetTarget() == modTarget)
			return modulator.get();

	std::print("Chosen modulator could not be found!");
	return nullptr;
}

/**
 * @brief Provides access to the list of modulators.
 * @return Reference to the internal vector of modulators.
 *
 * Allows direct iteration or inspection of all modulators attached to the sound object.
 */
std::vector<std::unique_ptr<Modulator>>& Sound::GetModulators()
{
	return m_Modulators;
}
}
