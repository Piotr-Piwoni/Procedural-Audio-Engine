#pragma once
#include <memory>
#include <string>

#include "../Utilities/Utils.hpp"
#include "GLAD/glad.h"

namespace MT::Core
{
/**
 * @brief Smart pointer alias for managing a GLFWwindow with a custom deleter.
 */
using WindowPtr = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>;

/**
 * @brief Wrapper around a GLFW window with OpenGL context.
 *
 * Handles GLFW initialization, window creation, OpenGL context setup,
 * buffer management, and optional ImGui integration.
 */
struct Window
{
	/**
	 * @brief Constructs a Window with given width, height, and title.
	 *
	 * Initializes GLFW, creates a window, sets the OpenGL context, and
	 * loads OpenGL functions using Glad.
	 *
	 * @param width Window width in pixels (default 1280)
	 * @param height Window height in pixels (default 720)
	 * @param title Window title (default "Procedural Audio Engine")
	 */
	explicit Window(const uint32_t width = 1280,
					const uint32_t height = 720,
					const std::string& title = "Procedural Audio Engine") :
		Ptr{InitGlfwWindow(width, height, title), &glfwDestroyWindow} {}

	~Window() { glfwTerminate(); }

	/**
	 * @brief Initializes GLFW, creates a window, sets OpenGL context, and loads Glad.
	 *
	 * @param width Window width in pixels (default 1280)
	 * @param height Window height in pixels (default 720)
	 * @param title Window title (default "Procedural Audio Engine")
	 * @return Pointer to the created GLFWwindow, or nullptr if initialization failed
	 */
	static GLFWwindow* InitGlfwWindow(const uint32_t width = 1280,
									  const uint32_t height = 720,
									  const std::string& title =
											  "Procedural Audio Engine")
	{
		if (!glfwInit())
			return nullptr;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(static_cast<int>(width),
											  static_cast<int>(height),
											  title.c_str(),
											  nullptr,
											  nullptr);
		if (!window)
		{
			glfwTerminate();
			return nullptr;
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		if (!gladLoadGLLoader(Utilities::GladLoader))
			return nullptr;

		return window;
	}

	/**
	 * @brief Clears the framebuffer, optionally renders an ImGui layer, and swaps buffers.
	 *
	 * @param imGui Optional pointer to an ImGuiLayer to render before swapping buffers
	 */
	void SwapBuffers(ImGuiLayer* imGui = nullptr) const
	{
		if (imGui)
			imGui->Render();

		int width, height;
		glfwGetFramebufferSize(Ptr.get(), &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (imGui)
			imGui->Draw();
		glfwSwapBuffers(Ptr.get());
	}

	/**
	 * @brief Checks whether the window has received a close request.
	 * @return true if the window should close, false otherwise
	 */
	[[nodiscard]] bool ShouldClose() const
	{
		return glfwWindowShouldClose(Ptr.get());
	}


	WindowPtr Ptr{nullptr, nullptr};
};
}
