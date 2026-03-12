#include "Application.hpp"

#include <print>
#include <random>
#include <thread>

#include "core/audio/effects/FadeEffect.hpp"
#include "IMGUI/imgui.h"
#include "ImGUI/imgui_internal.h"
#include "ui/imgui-knobs.h"
#include "Utilities/Utils.hpp"

using namespace MT::Core::Audio;

MT::Application::Application(GLFWwindow* win, AudioBackend* backend) :
	m_Window(win), m_Backend{backend}
{
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, KeyCallback);

	m_Sounds.reserve(16);
	m_BaseFrequencies.reserve(16);

	if (const WAVEFORMATEX* format = m_Backend->GetFormat())
		m_SampleRate = format->nSamplesPerSec;
}

void MT::Application::Update()
{
	const WAVEFORMATEX* format = m_Backend->GetFormat();
	if (!format) return;

	// Update sample rate if it has changed.
	if (m_SampleRate != format->nSamplesPerSec)
	{
		m_SampleRate = format->nSamplesPerSec;
		for (auto& sound : m_Sounds)
			sound.SetSampleRate(m_SampleRate);
	}

	if (m_Backend->GetPlaybackState() != PlaybackState::PLAYING) return;

	const auto [buffer, frames] = m_Backend->GetBuffer();
	if (!buffer) return;

	if (frames == 0)
	{
		std::this_thread::sleep_for(chrono::milliseconds(1));
		return;
	}

	const auto maxFrames = static_cast<uint64_t>(
		static_cast<float>(format->nSamplesPerSec) * m_AudioLength.count());

	if (!m_IsSoundContinues && m_FramesGenerated >= maxFrames)
	{
		m_Backend->ReleaseBuffer(frames);
		m_Backend->StopPlayback();
		return;
	}

	for (size_t i = 0; i < m_Sounds.size(); i++)
		m_Sounds[i].SetFrequency(m_BaseFrequencies[i] *
								 m_Sounds[i].GetPitchMultiplier());

	uint32_t framesToWrite = frames;
	if (!m_IsSoundContinues && m_FramesGenerated + frames > maxFrames)
		framesToWrite = static_cast<uint32_t>(maxFrames - m_FramesGenerated);

	// Generate noise.
	for (uint32_t frame = 0; frame < framesToWrite; frame++)
	{
		float mixedSample = 0.f;
		for (auto& sound : m_Sounds)
			mixedSample += sound.Generate();

		mixedSample *= m_MasterVolume;
		mixedSample = std::clamp(mixedSample, -1.f, 1.f);

		buffer[frame * format->nChannels + 0] = mixedSample;
		if (format->nChannels > 1)
			buffer[frame * format->nChannels + 1] = mixedSample;
	}

	if (!m_IsSoundContinues) m_FramesGenerated += framesToWrite;
	m_Backend->ReleaseBuffer(frames);
}

void MT::Application::Render()
{
	const ImVec2 size = ImGui::GetMainViewport()->Size;
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Appearing);

	if (ImGui::Begin("Welcome to Musical Trunk", nullptr,
					 ImGuiWindowFlags_AlwaysAutoResize |
					 ImGuiWindowFlags_NoDocking |
					 ImGuiWindowFlags_NoCollapse |
					 ImGuiWindowFlags_NoMove))
	{
		ImGui::Text("Sound Settings");
		ImGui::SliderFloat("Master Volume", &m_MasterVolume, 0.f, 1.f);
		ImGui::Checkbox("Is Audio Continues", &m_IsSoundContinues);

		// Render field only if the user wants to have a set audio length.
		if (!m_IsSoundContinues)
		{
			ImGui::Indent();
			float audioLength = m_AudioLength.count();
			if (ImGui::DragFloat("Audio Length (seconds)", &audioLength, 1.f,
								 0.f,
								 FLT_MAX, "%0.3f",
								 ImGuiSliderFlags_AlwaysClamp))
			{
				m_AudioLength = Duration(audioLength);
				for (auto& sound : m_Sounds)
					sound.SetAudioLength(m_AudioLength);
			}
			ImGui::Unindent();
		}

		CreateStartStopAudioButton();

		ImGui::Dummy({0.f, 15.f}); //< Spacer.
		for (size_t i = 0; i < m_Sounds.size(); i++)
			RenderSoundSettings(m_Sounds[i], i);

		// Buttons for adding and removing sounds.
		ImGui::Dummy({0.f, 10.f}); //< Spacer.
		constexpr ImVec2 buttonSize = {200.f, 25.f};
		if (ImGui::Button("Add Sound", buttonSize))
		{
			auto& sound = m_Sounds.emplace_back(0.25f, m_SampleRate);
			m_BaseFrequencies.push_back(sound.GetFrequency());
			sound.SetAudioLength(m_AudioLength);
		}

		if (!m_Sounds.empty() && ImGui::Button("Remove Sound", buttonSize))
		{
			m_BaseFrequencies.pop_back();
			m_Sounds.pop_back();
		}
	}
	ImGui::End();
}


void MT::Application::OnKey(const int key, int scancode,
							const int action,
							int mods)
{
	if (action != GLFW_PRESS)
		return;

	// Close application when the Escape key is pressed.
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(m_Window, true);

	// If there are sounds, allow playback controls.
	if (!m_Sounds.empty())
	{
		if (key == GLFW_KEY_P)
		{
			const auto state = m_Backend->GetPlaybackState();
			if (state == PlaybackState::PLAYING)
				m_Backend->PausePlayback();
			else if (state == PlaybackState::PAUSED)
				m_Backend->ResumePlayback();
		}
		if (key == GLFW_KEY_K)
		{
			const auto state = m_Backend->GetPlaybackState();
			if (state == PlaybackState::PLAYING)
				m_Backend->StopPlayback();
			else if (state == PlaybackState::STOPPED)
				PlayAudio();
		}
	}
}

void MT::Application::PlayAudio()
{
	m_FramesGenerated = 0;
	m_Backend->StartPlayback();
}

/**
 * @brief Generates a unique label for ImGui widgets using a base name and index.
 * @param name Base name of the widget.
 * @param index Index to ensure uniqueness.
 * @return Concatenated label string in the format "name##index".
 */
std::string MT::Application::MakeLabel(const std::string& name,
									   const size_t index)
{
	return name + "##" + std::to_string(index);
}

/**
 * @brief Renders an ImGui slider for a numeric value and updates it via a setter function.
 * @tparam T Numeric type of the slider.
 * @tparam Fn Callable type for updating the value.
 * @param name Display name of the slider.
 * @param index Unique index for the widget.
 * @param value Current value of the slider.
 * @param min Minimum slider value.
 * @param max Maximum slider value.
 * @param setter Function called with the new value when changed.
 * @param fmt Optional format string for displaying the value.
 */
template<typename T, typename Fn>
void MT::Application::RenderSlider(const std::string& name, size_t index,
								   T value, T min, T max, Fn setter,
								   const char* fmt)
{
	const std::string label = MakeLabel(name, index);
	if (ImGui::SliderFloat(label.c_str(), &value, min, max, fmt))
		setter(value);
}

/**
 * @brief Renders an ImGui knob for a numeric value and updates it via a setter function.
 * @tparam T Numeric type of the knob.
 * @tparam Fn Callable type for updating the value.
 * @param name Display name of the knob.
 * @param index Unique index for the widget.
 * @param value Current value of the knob.
 * @param min Minimum knob value.
 * @param max Maximum knob value.
 * @param step Increment step for knob adjustments.
 * @param setter Function called with the new value when changed.
 * @param fmt Optional format string for displaying the value.
 */
template<typename T, typename Fn>
void MT::Application::RenderKnob(const std::string& name, size_t index,
								 T value, T min, T max, T step, Fn setter,
								 const char* fmt)
{
	std::string label = MakeLabel(name, index);
	if (ImGuiKnobs::Knob(label.c_str(), &value, min, max, step, fmt,
						 ImGuiKnobVariant_Tick, 64.f))
		setter(value);
}

/**
 * @brief Renders the ImGui UI for editing the properties of a Sound object.
 * @param sound Sound object to render controls for.
 * @param index Index of the sound in a list, used for unique widget labels.
 *
 * Renders mute/unmute button, volume slider, decibel offset knob, and pitch multiplier knob.
 */
void MT::Application::RenderSoundSettings(Sound& sound, const size_t index)
{
	const std::string heading = MakeLabel("Sound " + std::to_string(index + 1),
										  index);

	if (!ImGui::CollapsingHeader(heading.c_str()))return;

	// Mute/Unmute button.
	const char* label = sound.IsMuted() ? "Unmute" : "Mute";
	if (ImGui::Button(MakeLabel(label, index).c_str()))
		sound.IsMuted() ? sound.UnMute() : sound.Mute();


	// Volume slider.
	RenderSlider("Volume", index, sound.GetVolume(), 0.f, 1.f,
				 [&](const float v) { sound.SetVolume(v); });


	// Arrays for combo.
	static constexpr GeneratorType GENERATOR_TYPES[] = {
		GeneratorType::SINE,
		GeneratorType::NOISE,
		GeneratorType::SAW_TOOTH,
		GeneratorType::SQUARE
	};
	static const char* generatorTitles[] = {
		"Sine",
		"White Noise",
		"Sawtooth",
		"Square"
	};

	// Find combo index from current type.
	int selectedIndex = 0;
	for (size_t i = 0; i < std::size(GENERATOR_TYPES); i++)
		if (sound.GetGeneratorType() == GENERATOR_TYPES[i])
			selectedIndex = static_cast<int>(i);

	if (ImGui::Combo(MakeLabel("Sound Type", index).c_str(), &selectedIndex,
					 generatorTitles, std::size(GENERATOR_TYPES)))
		sound.SetGeneratorType(GENERATOR_TYPES[selectedIndex]);


	// Frequency.
	if (sound.GetGeneratorType() != GeneratorType::NOISE)
	{
		float frequency = m_BaseFrequencies[index];
		if (ImGui::DragFloat(MakeLabel("Hz", index).c_str(), &frequency, 1.f,
							 1.f, FLT_MAX, "%0.3f",
							 ImGuiSliderFlags_AlwaysClamp))
			m_BaseFrequencies[index] = frequency;
	}


	// Decibel knob.
	RenderKnob("dB Offset", index, sound.GetDBLevel(), -100.f, 100.f, 1.f,
			   [&](const float db) { sound.SetDBLevel(db); }, "%.1f dB");


	// Pitch knob.
	ImGui::SameLine(90.f);
	if (sound.GetGeneratorType() != GeneratorType::NOISE)
	{
		RenderKnob("Pitch Multiplier", index, sound.GetPitchMultiplier(), 0.f,
				   10.f, 0.1f, [&](const float p)
				   {
					   sound.SetPitchMultiplier(p);
				   },
				   "%0.1f");
	}


	// Effects menu.
	if (ImGui::BeginMenu(MakeLabel("Add Effect", index).c_str()))
	{
		if (ImGui::MenuItem(MakeLabel("Fade Fx", index).c_str()))
			sound.AddEffect<FadeEffect>();

		ImGui::EndMenu();
	}

	// Effects UI.
	std::vector<std::unique_ptr<AudioEffect>>& effects = sound.GetEffects();
	for (int i = 0; static_cast<size_t>(i) < effects.size();)
	{
		ImGui::PushID(i);
		const bool remove = !effects[i]->RenderUI();
		ImGui::PopID();

		if (remove)
			effects.erase(effects.begin() + i);
		else
			i++;
	}
}

void MT::Application::CreateStartStopAudioButton()
{
	const auto state = m_Backend->GetPlaybackState();
	std::string startStopBtnLabel;
	if (state == PlaybackState::PLAYING)
		startStopBtnLabel = "Stop";
	else if (state == PlaybackState::STOPPED)
		startStopBtnLabel = "Start";

	if (!ImGui::Button(startStopBtnLabel.c_str(), {100.f, 25.f}) ||
		m_Sounds.empty())
		return;

	if (state == PlaybackState::PLAYING)
		m_Backend->StopPlayback();
	else if (state == PlaybackState::STOPPED)
		PlayAudio();
}
