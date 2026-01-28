#pragma once
#include "GLFW/glfw3.h"
#include "IMGUI/imgui.h"
#include "IMGUI/backend/imgui_impl_glfw.h"
#include "IMGUI/backend/imgui_impl_opengl3.h"

struct GLFWwindow;

namespace MT::Core
{
/**
 * @brief Wrapper around ImGui integration for a GLFW + OpenGL3 context.
 *
 * Provides lifecycle management for ImGui including initialization,
 * frame start/end, rendering, and shutdown.
 */
struct ImGuiLayer
{
	/**
	 * @brief Constructs the ImGui layer and initializes ImGui context.
	 * @param window Pointer to the GLFW window used for input and context.
	 */
	explicit ImGuiLayer(GLFWwindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		// Enable docking support.
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Use dark theme.
		ImGui::StyleColorsDark();

		// Initialize platform and renderer bindings.
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	~ImGuiLayer() { Shutdown(); }

	/**
	 * @brief Starts a new ImGui frame.
	 *
	 * Must be called at the beginning of each render loop iteration.
	 */
	void BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}


	/**
	 * @brief Renders ImGui draw data to the current OpenGL context.
	 *
	 * Should be called <b>after</b> populating the ImGui UI but <b>before</b>
	 * clearing the GLFW window and spawning its buffers.
	 */
	void Render() { ImGui::Render(); }

	/**
	 * @brief Executes the actual OpenGL draw commands for ImGui.
	 *
	 * Should be called <b>after</b> Render() and the GLFW window has
	 * finished clearing.
	 */
	void Draw() { ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); }

	/**
	 * @brief Shuts down ImGui, releasing resources and context.
	 *
	 * Safe to call multiple times; automatically called in destructor.
	 */
	void Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
};
}
