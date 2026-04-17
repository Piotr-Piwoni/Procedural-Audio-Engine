#pragma once
#include "chrono"
#include "core/AudioEngine.hpp"
#include "core/audio/Sound.hpp"
#include "GLFW/glfw3.h"

namespace chrono = std::chrono;

namespace MT
{
/**
 * @brief Demo application showcasing the audio engine with real-time UI controls.
 *
 * The DemoApplication class provides a simple interactive environment for testing
 * and demonstrating the AudioEngine. It integrates with GLFW for window/input
 * handling and ImGui for rendering a graphical interface.
 *
 * Responsibilities:
 * - Initialise and own an AudioEngine instance.
 * - Forward update calls to the audio engine.
 * - Render an interactive UI for controlling audio behaviour.
 * - Handle user input for playback and application control.
 *
 * UI features:
 * - Global audio controls (master volume, playback mode, audio length).
 * - Dynamic creation and removal of Sound objects.
 * - Per-sound configuration (volume, pitch, generator type, frequency).
 * - Generator-specific controls (e.g. noise seed, pulse width).
 * - Effect and modulator management with live parameter editing.
 *
 * Input handling:
 * - Uses GLFW callbacks to process keyboard input.
 * - Provides shortcuts for playback control and application exit.
 *
 * Design notes:
 * - Intended purely as a demo/debug tool rather than production UI.
 * - UI logic is tightly coupled to ImGui for rapid iteration.
 * - Demonstrates real-time audio parameter manipulation.
 *
 * Typical usage:
 * - Construct with a valid GLFWwindow pointer.
 * - Call Update() each frame to process audio.
 * - Call Render() each frame to draw the UI.
 *
 * This class serves as a reference implementation for integrating the audio
 * system with a graphical interface and user interaction layer.
 */
class DemoApplication
{
public:
	DemoApplication(GLFWwindow* win);

	void Update();
	void Render();

private:
	void OnKey(int key, int scancode, int action, int mods);

	static void KeyCallback(GLFWwindow* window, int key, int scancode,
							int action, int mods);

	static std::string MakeLabel(const std::string& name, size_t index);

	template<class T, class Fn>
	void RenderSlider(const std::string& name, size_t index, T value, T min,
					  T max, Fn setter, const char* fmt = "%0.3f");
	template<class T, class Fn>
	void RenderKnob(const std::string& name, size_t index, T value, T min,
					T max, T step, Fn setter, const char* fmt = "%0.3f");

	void RenderSoundSettings(Core::Audio::Sound& sound, size_t index);
	void CreateStartStopAudioButton() const;
	bool RenderEffectUI(Core::Audio::AudioEffect* effect) const;
	bool RenderModulatorUI(Core::Audio::Modulator& modulator);

private:
	GLFWwindow* m_Window{nullptr};
	std::unique_ptr<Core::AudioEngine> m_Engine{nullptr};
};
}
