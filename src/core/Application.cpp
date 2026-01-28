#include "Application.hpp"

#include "IMGUI/imgui.h"


void MT::Application::Update() {}

void MT::Application::Render()
{
	ImGui::Begin("Hello ImGui");
	ImGui::Text("Hello World!");
	ImGui::End();
}


void MT::Application::OnKey(const int key, int scancode,
							const int action,
							int mods) const
{
	// Close application when the Escape key is pressed.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_Window, true);
}
