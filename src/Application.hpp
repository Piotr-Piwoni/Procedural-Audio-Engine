#pragma once
#include <unordered_map>
#include <vector>

#include "GLFW/glfw3.h"
#include "core/audio/AudioBackend.hpp"
#include "core/audio/Sound.hpp"

namespace MT
{
class Application
{
public:
	Application(GLFWwindow* win, Core::Audio::AudioBackend* backend);

	void Update();
	void Render();

private:
	void OnKey(int key, int scancode, int action, int mods) const;

	/// <summary> GLFW static callback to key press detection. </summary>
	static void KeyCallback(GLFWwindow* window, const int key,
							const int scancode, const int action,
							const int mods)
	{
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		if (app)
			app->OnKey(key, scancode, action, mods);
	}

	static std::string MakeLabel(const std::string& name, size_t index);

	template<class T, class Fn>
	void RenderSlider(const std::string& name, size_t index, T value, T min,
					  T max, Fn setter, const char* fmt = "%0.3f");

	template<class T, class Fn>
	void RenderKnob(const std::string& name, size_t index, T value, T min,
					T max, T step, Fn setter, const char* fmt = "%0.3f");

	void RenderSoundSettings(Core::Audio::Sound& sound, size_t i);

	void CreateStartStopAudioButton() const;

private:
	GLFWwindow* m_Window{nullptr};
	Core::Audio::AudioBackend* m_Backend{nullptr};

	float m_MasterVolume{1.f};
	std::vector<Core::Audio::Sound> m_Sounds{};
	std::unordered_map<Core::Audio::Sound*, float> m_BaseFrequencies{};
};
}
