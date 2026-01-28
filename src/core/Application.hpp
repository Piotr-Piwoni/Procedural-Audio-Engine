#pragma once
#include "GLFW/glfw3.h"

namespace MT
{
class Application
{
public:
	Application(GLFWwindow* win) :
		m_Window(win)
	{
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetKeyCallback(m_Window, KeyCallback);
	}

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
	GLFWwindow* m_Window;
};
}
