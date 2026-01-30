#include "Application.hpp"

#include <print>
#include <random>
#include <thread>

#include "IMGUI/imgui.h"

using namespace MT::Core::Audio;


MT::Application::Application(GLFWwindow* win, AudioBackend* backend) :
	m_Window(win), m_Backend{backend}
{
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, KeyCallback);

	backend->StartPlayback();

	m_Sounds.reserve(16);
	m_Sounds.emplace_back(0.25f);
	m_Sounds.emplace_back(0.25f);
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
	for (uint32_t i = 0; i < frames; i++)
	{
		float mixedSample = 0.f;
		for (auto sound : m_Sounds)
			mixedSample += sound.GetBuffer();

		mixedSample *= m_MasterVolume;
		mixedSample = std::clamp(mixedSample, -1.f, 1.f);

		buffer[i * format->nChannels + 0] = mixedSample;
		if (format->nChannels > 1)
			buffer[i * format->nChannels + 1] = mixedSample;
	}
	m_Backend->ReleaseBuffer(frames);
}

void MT::Application::Render()
{
	ImGui::Begin("Hello ImGui");
	ImGui::Text("Sound Settings");
	ImGui::SliderFloat("Master Volume", &m_MasterVolume, 0.f, 1.f);
	for (size_t i = 0; i < m_Sounds.size(); i++)
	{
		auto& sound = m_Sounds[i];
		char header[32];
		std::snprintf(header, sizeof(header), "Sound %zu", i + 1);

		if (ImGui::CollapsingHeader(header))
		{
			// Mute/Unmute button.
			if (sound.IsMuted())
			{
				char buttonLabel[32];
				std::snprintf(buttonLabel, sizeof(buttonLabel), "Unmute##%zu",
							  i);
				if (ImGui::Button(buttonLabel))
					sound.UnMute();
			}
			else
			{
				char buttonLabel[32];
				std::snprintf(buttonLabel, sizeof(buttonLabel), "Mute##%zu", i);
				if (ImGui::Button(buttonLabel))
					sound.Mute();
			}

			// Volume slider.
			float volume = sound.GetVolume();
			char volumeLabel[32];
			std::snprintf(volumeLabel, sizeof(volumeLabel), "Volume##%zu", i);
			if (ImGui::SliderFloat(volumeLabel, &volume, 0.f, 1.f))
				sound.SetVolume(volume);

			// Decibel input.
			float db = sound.GetDBLevel();
			char dbLabel[32];
			std::snprintf(dbLabel, sizeof(dbLabel), "dB Offset##%zu", i);
			if (ImGui::DragFloat(dbLabel, &db, 1.f, INT_MIN, INT_MAX))
				sound.SetDBLevel(db);
		}
	}
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
		if (state == PlaybackState::PLAYING)
			m_Backend->PausePlayback();
		else if (state == PlaybackState::PAUSED)
			m_Backend->ResumePlayback();
	}
	if (key == GLFW_KEY_K)
	{
		const auto state = m_Backend->GetPlaybackState();
		if (state == PlaybackState::PLAYING)
			m_Backend->StopPlayback();
		else if (state == PlaybackState::STOPPED)
			m_Backend->StartPlayback();
	}
}
