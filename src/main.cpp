#include <Audioclient.h>
#include <Windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <print>
#include <random>
#include <thread>

#include "core/Application.hpp"
#include "core/AudioBackend.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/Window.hpp"


int main()
{
	const MT::Core::Window window(1280, 720, "Musical Trunk - PAE");
	if (!window.Ptr)
		return EXIT_FAILURE;


	MT::Core::AudioBackend backend;
	const MT::Core::BackendError result = backend.Create(COINIT_MULTITHREADED);

	if (result != MT::Core::BackendError::NONE)
	{
		std::println("Audio backend failed to be created, the reason {} error!",
					 to_string(result));
		return EXIT_FAILURE;
	}

	// Output some information about the format.
	if (const auto format = backend.GetFormat())
		std::println("Channels: {}\nSampleRate: {}\nBits per sample: {}",
					 format->nChannels, format->nSamplesPerSec,
					 format->wBitsPerSample);


	MT::Core::ImGuiLayer imGuiLayer(window.Ptr.get());
	const auto app = std::make_unique<MT::Application>(
			window.Ptr.get(), &backend);

	while (!window.ShouldClose())
	{
		glfwPollEvents();

		app->Update();

		imGuiLayer.BeginFrame();
		app->Render();

		window.SwapBuffers(&imGuiLayer);

	}
	return EXIT_SUCCESS;
}
