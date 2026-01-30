#include "Application.hpp"

#include <random>
#include <thread>

#include "IMGUI/imgui.h"


MT::Application::Application(GLFWwindow* win,
							 Core::Audio::AudioBackend* backend) :
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

	const auto [buffer, frames] = m_Backend->GetBuffer();
	if (!buffer)
		return;

	if (frames == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return;
	}

	// Generate some white noise.
	static std::mt19937 gen(std::random_device{}());
	static std::uniform_real_distribution dist(-1.0f, 1.0f);
	for (uint32_t i = 0; i < frames; i++)
	{
		const float sample = dist(gen);
		buffer[i * format->nChannels + 0] = sample;
		if (format->nChannels > 1)
			buffer[i * format->nChannels + 1] = sample;

	}
	m_Backend->ReleaseBuffer(frames);
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
	if (action != GLFW_PRESS)
		return;

	// Close application when the Escape key is pressed.
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(m_Window, true);
	if (key == GLFW_KEY_P)
	{
		const auto state = m_Backend->GetPlaybackState();
		if (state == Core::Audio::PlaybackState::PLAYING)
			m_Backend->PausePlayback();
		else if (state == Core::Audio::PlaybackState::PAUSED)
			m_Backend->ResumePlayback();
	}
	if (key == GLFW_KEY_K)
	{
		const auto state = m_Backend->GetPlaybackState();
		if (state == Core::Audio::PlaybackState::PLAYING)
			m_Backend->StopPlayback();
		else if (state == Core::Audio::PlaybackState::STOPPED)
			m_Backend->StartPlayback();
	}
}
