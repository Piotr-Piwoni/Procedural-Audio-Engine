#pragma once
#include "chrono"
#include "core/AudioEngine.hpp"
#include "core/audio/Sound.hpp"
#include "GLFW/glfw3.h"
#include "Utilities/TypeDefinitions.hpp"

namespace chrono = std::chrono;

namespace MT
{
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
