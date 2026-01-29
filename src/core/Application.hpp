#pragma once
#include "AudioBackend.hpp"
#include "GLFW/glfw3.h"

namespace MT
{
class Application
{
public:
	Application(GLFWwindow* win, Core::AudioBackend* backend);

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

private:
	GLFWwindow* m_Window{nullptr};
	Core::AudioBackend* m_Backend{nullptr};
};
}
