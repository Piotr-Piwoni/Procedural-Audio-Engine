#include <print>

#include "DemoApplication.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/Window.hpp"
#include "core/audio/AudioBackend.hpp"


int main()
{
	const MT::Core::Window window(1280, 720, "Musical Trunk - PAE");
	if (!window.Ptr) return EXIT_FAILURE;

	MT::Core::ImGuiLayer imGuiLayer(window.Ptr.get());
	const auto app = std::make_unique<MT::DemoApplication>(window.Ptr.get());

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
