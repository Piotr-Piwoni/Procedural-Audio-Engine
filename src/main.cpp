#include <Audioclient.h>
#include <Windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <print>

#include "core/Application.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/Window.hpp"


int main()
{
	const MT::Core::Window window(1280, 720, "Musical Trunk - PAE");
	if (!window.Ptr)
		return EXIT_FAILURE;


	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(result))
		return EXIT_FAILURE;

	IMMDeviceEnumerator* deviceEnumerator{nullptr};
	result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  nullptr,
							  CLSCTX_ALL,
							  __uuidof(IMMDeviceEnumerator),
							  IID_PPV_ARGS_Helper(&deviceEnumerator));
	if (FAILED(result))
		return EXIT_FAILURE;

	IMMDevice* device{nullptr};
	result = deviceEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole, &device);
	if (FAILED(result))
	{
		std::cerr << "Failed to get default audio device.\n";
		deviceEnumerator->Release();
		return EXIT_FAILURE;
	}

	IAudioClient* audioClient{nullptr};
	result = device->Activate(__uuidof(IAudioClient),
							  CLSCTX_ALL,
							  nullptr,
							  IID_PPV_ARGS_Helper(&audioClient));
	if (FAILED(result))
	{
		std::cerr << "Failed to activate the audio client.\n";
		device->Release();
		deviceEnumerator->Release();
		return EXIT_FAILURE;
	}

	WAVEFORMATEX* mixFormat{nullptr};
	result = audioClient->GetMixFormat(&mixFormat);
	if (FAILED(result))
	{
		std::cerr << "Failed to get mix format.\n";
		audioClient->Release();
		device->Release();
		deviceEnumerator->Release();
		return EXIT_FAILURE;
	}

	std::println("Channels: {}\nSampleRate: {}\nBits per sample: {}",
				 mixFormat->nChannels, mixFormat->nSamplesPerSec,
				 mixFormat->wBitsPerSample);

	result = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
									 0, 0, 0, mixFormat, nullptr);
	if (FAILED(result))
		std::cerr << "IAudioClient::Initialize failed!\n";
	std::println("Audio client initialized successfully!");


	MT::Core::ImGuiLayer imGuiLayer(window.Ptr.get());
	const auto app = std::make_unique<MT::Application>(window.Ptr.get());
	while (!window.ShouldClose())
	{
		glfwPollEvents();

		app->Update();

		imGuiLayer.BeginFrame();
		app->Render();

		window.SwapBuffers(&imGuiLayer);

	}

	audioClient->Release();
	device->Release();
	deviceEnumerator->Release();
	CoUninitialize();
	return EXIT_SUCCESS;
}
