#include "Application.hpp"

#include <print>
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
	std::println("Master Volume: {}", m_MasterVolume);


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
		// INFO: Before writing to buffer, make sure the sample is mapped to -1 and 1.
		// Volume control: sample *= gain;
		// Per sound + Master Volume: sample *= perSound * Master;
		float sample = dist(gen);
		float gain = std::powf(
				10.f, static_cast<float>(m_MasterVolumeDB) / 20.f);
		sample *= m_MasterVolume * gain;

		sample = std::clamp(sample, -1.f, 1.f);
		buffer[i * format->nChannels + 0] = sample;
		if (format->nChannels > 1)
			buffer[i * format->nChannels + 1] = sample;

	}
	m_Backend->ReleaseBuffer(frames);
}

void MT::Application::Render()
{
	ImGui::Begin("Hello ImGui");
	ImGui::Text("Sound Settings");
	ImGui::SliderFloat("Master Volume", &m_MasterVolume, 0.f, 1.f);
	ImGui::DragInt("Master Volume DB", &m_MasterVolumeDB, 1, INT_MIN, INT_MAX);
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
