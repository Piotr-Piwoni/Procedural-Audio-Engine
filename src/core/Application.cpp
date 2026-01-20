#include "Application.h"


void Application::Update() {}

void Application::OnKey(const int key, int scancode, const int action,
						int mods) const
{
	// Close application when the Escape key is pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_Window, true);
}

void Application::Render()
{
	ImGui::Begin("Hello ImGui");
	ImGui::Text("Hello World!");
	ImGui::End();
}
