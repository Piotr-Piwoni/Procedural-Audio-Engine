#pragma once
#include "GLFW/glfw3.h"

namespace MT::Utilities
{
/**
 * @brief Loader function for Glad to retrieve OpenGL function pointers.
 *
 * Used by gladLoadGLLoader to bind OpenGL functions to the current context.
 *
 * @param name Name of the OpenGL function.
 * @return Pointer to the function, or nullptr if not found.
 */
inline void* GladLoader(const char* name)
{
	return reinterpret_cast<void*>(glfwGetProcAddress(name));
}

inline float AsDecibels(const float gain)
{
	return std::log10f(gain);
}

inline float AsGain(const float dB)
{
	return std::powf(10.f, dB / 20.f);

}
}
