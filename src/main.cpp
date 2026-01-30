#include <print>

#include "Application.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/Window.hpp"
#include "core/audio/AudioBackend.hpp"


int main()
{
	const MT::Core::Window window(1280, 720, "Musical Trunk - PAE");
	if (!window.Ptr)
		return EXIT_FAILURE;


	MT::Core::Audio::AudioBackend backend;
	const MT::Core::Audio::BackendError result = backend.Create(
			COINIT_MULTITHREADED);

	if (result != MT::Core::Audio::BackendError::NONE)
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
