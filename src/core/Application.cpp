#include "Application.hpp"

#include <iostream>
#include <random>
#include <thread>

#include "IMGUI/imgui.h"


MT::Application::Application(GLFWwindow* win, Core::AudioBackend* backend) :
	m_Window(win), m_Backend{backend}
{
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, KeyCallback);

	backend->StartPlayback();
}

void MT::Application::Update()
{
	const auto format = m_Backend->GetFormat();
	if (!format)
		return;

	const Core::AudioBuffer buffer = m_Backend->GetBuffer();
	if (!buffer.Data)
		return;

	if (buffer.Frames == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return;
	}

	// Generate some white noise.
	static std::mt19937 gen(std::random_device{}());
	static std::uniform_real_distribution dist(-1.0f, 1.0f);
	for (uint32_t i = 0; i < buffer.Frames; i++)
	{
		const float sample = dist(gen);
		buffer.Data[i * format->nChannels + 0] = sample;
		if (format->nChannels > 1)
			buffer.Data[i * format->nChannels + 1] = sample;

	}
	m_Backend->ReleaseBuffer(buffer.Frames);
}

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
