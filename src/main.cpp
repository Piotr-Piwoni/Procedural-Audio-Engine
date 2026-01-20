#include <memory>
#include <string_view>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <IMGUI/imgui.h>
#include <IMGUI/backend/imgui_impl_glfw.h>
#include <IMGUI/backend/imgui_impl_opengl3.h>

#include "core/Application.h"


GLFWwindow* InitWindow(const int width = 1280, const int height = 720,
					   const std::string_view title = "Procedural Audio Engine")
{
	if (!glfwInit())
		return nullptr;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, title.data(), nullptr,
										  nullptr);
	if (!window)
	{
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		return nullptr;

	return window;
}

void InitImGui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void BeginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void EndFrame(GLFWwindow* window)
{
	ImGui::Render();

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}

void Cleanup(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

int main()
{
	const auto window = std::unique_ptr<
		GLFWwindow, decltype(&glfwDestroyWindow)>(InitWindow(),
												  &glfwDestroyWindow);
	if (!window)
		return EXIT_FAILURE;

	InitImGui(window.get());

	auto app = std::make_unique<Application>(window.get());
	while (!glfwWindowShouldClose(window.get()))
	{
		glfwPollEvents();

		app->Update();

		BeginFrame();
		app->Render();
		EndFrame(window.get());
	}

	Cleanup(window.get());
	return EXIT_SUCCESS;
}
