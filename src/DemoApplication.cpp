#include "DemoApplication.hpp"

#include <random>
#include <thread>

#include "core/audio/effects/FadeEffect.hpp"
#include "IMGUI/imgui.h"
#include "ImGUI/imgui_internal.h"
#include "ui/imgui-knobs.h"
#include "Utilities/Utils.hpp"

using namespace MT::Core::Audio;
using namespace std::chrono_literals;


MT::DemoApplication::DemoApplication(GLFWwindow* win) :
	m_Window(win)
{
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, KeyCallback);

	// In this demo, is the Audio Engine fails to initialise for any reason,
	// the application will immediately close.
	m_Engine = std::make_unique<Core::AudioEngine>();
	if (!m_Engine->Init())
		glfwSetWindowShouldClose(m_Window, true);
}


void MT::DemoApplication::Update() { m_Engine->Update(); }

void MT::DemoApplication::Render()
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

		float volume = m_Engine->GetVolume();
		if (ImGui::SliderFloat("Master Volume", &volume, 0.f, 1.f, "%.3f",
							   ImGuiSliderFlags_AlwaysClamp))
			m_Engine->SetVolume(volume);

		bool isContinues = m_Engine->AreSoundsContinues();
		if (ImGui::Checkbox("Is Audio Continues", &isContinues))
			m_Engine->SetSoundsContinues(isContinues);

		// Render field only if the user wants to have a set audio length.
		if (!m_Engine->AreSoundsContinues())
		{
			ImGui::Indent();
			float audioLength = m_Engine->GetAudioLength().count();
			if (ImGui::DragFloat("Audio Length (seconds)", &audioLength, 1.f,
								 0.f,FLT_MAX, "%0.3f",
								 ImGuiSliderFlags_AlwaysClamp))
				m_Engine->SetAudioLength(Duration(audioLength));
			ImGui::Unindent();
		}

		CreateStartStopAudioButton();

		ImGui::Dummy({0.f, 15.f}); //< Spacer.
		std::deque<Sound>& sounds = m_Engine->GetSounds();
		for (size_t i = 0; i < sounds.size(); i++)
			RenderSoundSettings(sounds[i], i);

		// Buttons for adding and removing sounds.
		ImGui::Dummy({0.f, 10.f}); //< Spacer.
		constexpr ImVec2 buttonSize = {200.f, 25.f};
		if (ImGui::Button("Add Sound", buttonSize))
			m_Engine->CreateSound(0.25f);

		if (m_Engine->HasSounds() && ImGui::Button("Remove Sound", buttonSize))
			m_Engine->RemoveSound();
	}
	ImGui::End();
}


void MT::DemoApplication::OnKey(const int key, int scancode,
								const int action, int mods)
{
	if (action != GLFW_PRESS) return;

	// Close application when the Escape key is pressed.
	if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(m_Window, true);

	// If there are sounds, allow playback controls.
	if (m_Engine->HasSounds())
	{
		if (key == GLFW_KEY_P) m_Engine->TogglePause();
		if (key == GLFW_KEY_K) m_Engine->TogglePlayback();
	}
}

/// <summary> GLFW static callback to key press detection. </summary>
void MT::DemoApplication::KeyCallback(GLFWwindow* window, const int key,
									  const int scancode, const int action,
									  const int mods)
{
	auto app = static_cast<DemoApplication*>(glfwGetWindowUserPointer(window));
	if (app) app->OnKey(key, scancode, action, mods);
}

/**
 * @brief Generates a unique label for ImGui widgets using a base name and index.
 * @param name Base name of the widget.
 * @param index Index to ensure uniqueness.
 * @return Concatenated label string in the format "name##index".
 */
std::string MT::DemoApplication::MakeLabel(const std::string& name,
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
void MT::DemoApplication::RenderSlider(const std::string& name, size_t index,
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
void MT::DemoApplication::RenderKnob(const std::string& name, size_t index,
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
void MT::DemoApplication::RenderSoundSettings(Sound& sound, const size_t index)
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
		float frequency = sound.GetBaseFrequency();
		if (ImGui::DragFloat(MakeLabel("Hz", index).c_str(), &frequency, 1.f,
							 1.f, FLT_MAX, "%0.3f",
							 ImGuiSliderFlags_AlwaysClamp))
			sound.SetBaseFrequency(frequency);
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
		if (ImGui::MenuItem(MakeLabel("Modulator", index).c_str()))
			sound.AddModulator(Modulator());

		ImGui::EndMenu();
	}

	// Effects UI.
	std::vector<std::unique_ptr<AudioEffect>>& effects = sound.GetEffects();
	for (int i = 0; static_cast<size_t>(i) < effects.size();)
	{
		ImGui::PushID(i);
		const bool remove = !RenderEffectUI(effects[i].get());
		ImGui::PopID();

		if (remove) effects.erase(effects.begin() + i);
		else i++;
	}

	// Modulator UI.
	std::vector<std::unique_ptr<Modulator>>& modulators = sound.GetModulators();
	for (int i = 0; static_cast<size_t>(i) < modulators.size();)
	{
		ImGui::PushID(i);
		const bool remove = !RenderModulatorUI(*modulators[i]);
		ImGui::PopID();

		if (remove) modulators.erase(modulators.begin() + i);
		else i++;
	}
}

void MT::DemoApplication::CreateStartStopAudioButton() const
{
	const auto state = m_Engine->GetState();
	std::string startStopBtnLabel;
	if (state == PlaybackState::PLAYING) startStopBtnLabel = "Stop";
	else if (state == PlaybackState::STOPPED) startStopBtnLabel = "Start";

	if (!ImGui::Button(startStopBtnLabel.c_str(), {100.f, 25.f}) |
		!m_Engine->HasSounds())
		return;

	m_Engine->TogglePlayback();
}

bool MT::DemoApplication::RenderEffectUI(AudioEffect* effect) const
{
	ImGui::PushID(effect);

	// This is for demo purposes only, in a profession project UI for
	// different effects would be done differently.
	if (const auto fadeEffect = dynamic_cast<FadeEffect*>(effect))
	{
		if (ImGui::CollapsingHeader("Fade Effect"))
		{
			if (ImGui::Button("X"))
			{
				fadeEffect->Reset();
				ImGui::PopID();
				return false;
			}

			float inPoint = fadeEffect->GetFadeInPoint().count();
			const float length = m_Engine->GetAudioLength().count();
			const float max = length > 0.f ? length : FLT_MAX;
			if (ImGui::DragFloat("Fade Start (seconds)", &inPoint, 1.f, 0.f,
								 max, "%0.3f", ImGuiSliderFlags_AlwaysClamp))
				fadeEffect->SetFadeInPoint(inPoint);

			if (length > 0.f)
			{
				ImGui::SameLine();
				float outPoint = fadeEffect->GetFadeOutPoint().count();
				if (ImGui::DragFloat("Fade Out (seconds)", &outPoint, 1.f, 0.f,
									 length, "%0.3f",
									 ImGuiSliderFlags_AlwaysClamp))
					fadeEffect->SetFadeOutPoint(outPoint);
			}
		}

		ImGui::PopID();
		return true;
	}
	return true;
}

bool MT::DemoApplication::RenderModulatorUI(Modulator& modulator)
{
	ImGui::PushID(&modulator);

	if (ImGui::CollapsingHeader("Modulator"))
	{
		if (ImGui::Button("X"))
		{
			ImGui::PopID();
			return false;
		}

		// Arrays for combo.
		static constexpr ModulatorTarget MODULATOR_TARGET[] = {
			ModulatorTarget::FREQUENCY,
			ModulatorTarget::PITCH,
			ModulatorTarget::AMPLITUDE_GAIN,
			ModulatorTarget::AMPLITUDE_DECIBEL
		};
		static const char* modTargetNames[] = {
			"Frequency",
			"Pitch",
			"Amplitude Gain",
			"Amplitude Decibel"
		};

		// Modulator Target Combo.
		int selectedTargetIndex = 0;
		for (size_t i = 0; i < std::size(MODULATOR_TARGET); i++)
			if (modulator.GetTarget() == MODULATOR_TARGET[i])
				selectedTargetIndex = static_cast<int>(i);

		if (ImGui::Combo("Target", &selectedTargetIndex, modTargetNames,
						 std::size(MODULATOR_TARGET)))
			modulator.SetTarget(MODULATOR_TARGET[selectedTargetIndex]);

		ImGui::SameLine(300.f);


		float frequency = modulator.GetFrequency();
		if (ImGui::DragFloat("Frequency", &frequency, 1.f, 0.f, FLT_MAX,
							 "%0.3f", ImGuiSliderFlags_AlwaysClamp))
			modulator.SetFrequency(frequency);


		// Arrays for combo.
		static constexpr ModulatorWave MODULATOR_WAVE[] = {
			ModulatorWave::SINE,
		};
		static const char* modWaveNames[] = {
			"Sine",
		};

		// Modulator Wave Combo.
		int selectedWaveIndex = 0;
		for (size_t i = 0; i < std::size(MODULATOR_WAVE); i++)
			if (modulator.GetWaveType() == MODULATOR_WAVE[i])
				selectedWaveIndex = static_cast<int>(i);

		if (ImGui::Combo("Wave", &selectedWaveIndex, modWaveNames,
						 std::size(MODULATOR_WAVE)))
			modulator.SetWaveType(MODULATOR_WAVE[selectedWaveIndex]);


		ImGui::SameLine(300.f);

		float depth = modulator.GetDepth();
		if (ImGui::DragFloat("Depth", &depth, 0.01f, 0.f, FLT_MAX,
							 "%0.3f", ImGuiSliderFlags_AlwaysClamp))
			modulator.SetDepth(depth);
	}

	ImGui::PopID();
	return true;
}
