#include "Application.hpp"

#include <print>
#include <random>
#include <thread>

#include "IMGUI/imgui.h"
#include "ui/imgui-knobs.h"

using namespace MT::Core::Audio;

MT::Application::Application(GLFWwindow* win, AudioBackend* backend) :
	m_Window(win), m_Backend{backend}
{
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, KeyCallback);

	m_Sounds.reserve(16);
	m_BaseFrequencies.reserve(16);
}

void MT::Application::Update()
{
	if (m_Backend->GetPlaybackState() != PlaybackState::PLAYING)
		return;

	const auto format = m_Backend->GetFormat();
	if (!format)
		return;

	const auto [buffer, frames] = m_Backend->GetBuffer();
	if (!buffer)
		return;

	if (frames == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return;
	}

	for (auto& sound : m_Sounds)
		sound.SetFrequency(m_BaseFrequencies.at(&sound) *
						   sound.GetPitchMultiplier());

	// Generate noise.
	for (uint32_t i = 0; i < frames; i++)
	{
		float mixedSample = 0.f;
		for (auto& sound : m_Sounds)
			mixedSample += sound.GetBuffer();

		mixedSample *= m_MasterVolume;
		mixedSample = std::clamp(mixedSample, -1.f, 1.f);

		buffer[i * format->nChannels + 0] = mixedSample;
		if (format->nChannels > 1)
			buffer[i * format->nChannels + 1] = mixedSample;
	}
	m_Backend->ReleaseBuffer(frames);
}

void MT::Application::Render()
{
	ImGui::Begin("Hello ImGui");
	ImGui::Text("Sound Settings");
	ImGui::SliderFloat("Master Volume", &m_MasterVolume, 0.f, 1.f);

	for (size_t i = 0; i < m_Sounds.size(); i++)
		RenderSoundSettings(m_Sounds[i], i);

	// Buttons for adding and removing sounds.
	constexpr ImVec2 buttonSize = {200.f, 25.f};
	if (ImGui::Button("Add Sound", buttonSize))
	{
		auto& sound = m_Sounds.emplace_back(
				0.25f, m_Backend->GetFormat()->nSamplesPerSec);
		m_BaseFrequencies.insert_or_assign(&sound, sound.GetFrequency());
	}

	if (!m_Sounds.empty() && ImGui::Button("Remove Sound", buttonSize))
	{
		auto& sound = m_Sounds.back();
		m_BaseFrequencies.erase(&sound);
		m_Sounds.pop_back();
	}

	ImGui::End();
}


void MT::Application::OnKey(const int key, int scancode,
							const int action,
							int mods) const
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
				m_Backend->StartPlayback();
		}
	}
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
 * @param i Index of the sound in a list, used for unique widget labels.
 *
 * Renders mute/unmute button, volume slider, decibel offset knob, and pitch multiplier knob.
 */
void MT::Application::RenderSoundSettings(Sound& sound, const size_t i)
{
	const std::string heading = MakeLabel("Sound " + std::to_string(i + 1), i);

	if (!ImGui::CollapsingHeader(heading.c_str()))
		return;

	// Mute/Unmute button.
	const char* label = sound.IsMuted() ? "Unmute" : "Mute";
	if (ImGui::Button(MakeLabel(label, i).c_str()))
	{
		if (sound.IsMuted())
			sound.UnMute();
		else
			sound.Mute();
	}

	// Volume Slider.
	RenderSlider("Volume", i, sound.GetVolume(), 0.f, 1.f,
				 [&](const float v) { sound.SetVolume(v); });

	// Decibel Knob.
	RenderKnob("dB Offset", i, sound.GetDBLevel(), -100.f, 100.f, 1.f,
			   [&](const float db) { sound.SetDBLevel(db); }, "%.1f dB");

	ImGui::SameLine(90.f);

	// Pitch Knob.
	RenderKnob("Pitch Multiplier", i, sound.GetPitchMultiplier(), 0.f,
			   10.f, 0.1f, [&](const float p) { sound.SetPitchMultiplier(p); },
			   "%0.1f");
}
