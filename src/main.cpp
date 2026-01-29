#include <Audioclient.h>
#include <Windows.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <print>
#include <random>
#include <thread>

#include "core/Application.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/Window.hpp"


int main()
{
	const MT::Core::Window window(1280, 720, "Musical Trunk - PAE");
	if (!window.Ptr)
		return EXIT_FAILURE;


	// Init COM (Component Object Model).
	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(result))
		return EXIT_FAILURE;

	// "IMMDeviceEnumerator" lists audio devices.
	IMMDeviceEnumerator* deviceEnumerator = nullptr;
	result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  nullptr,
							  CLSCTX_ALL,
							  __uuidof(IMMDeviceEnumerator),
							  IID_PPV_ARGS_Helper(&deviceEnumerator));
	if (FAILED(result))
		return EXIT_FAILURE;

	// Get default audio device. eRender -> Output, eConsole -> (use-case) normal app.
	IMMDevice* device = nullptr;
	result = deviceEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole, &device);
	if (FAILED(result))
	{
		std::cerr << "Failed to get default audio device.\n";
		deviceEnumerator->Release();
		return EXIT_FAILURE;
	}

	// Lets us communicate with the audio device.
	IAudioClient* audioClient = nullptr;
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

	WAVEFORMATEX* mixFormat = nullptr;
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


	// Actually write audio into buffer.
	IAudioRenderClient* renderClient = nullptr;
	audioClient->GetService(__uuidof(IAudioRenderClient),
							IID_PPV_ARGS_Helper(&renderClient));

	uint32_t bufferFrameCount = 0;
	audioClient->GetBufferSize(&bufferFrameCount);
	audioClient->Start();

	MT::Core::ImGuiLayer imGuiLayer(window.Ptr.get());
	const auto app = std::make_unique<MT::Application>(window.Ptr.get());
	while (!window.ShouldClose())
	{
		glfwPollEvents();

		app->Update();

		uint32_t padding = 0;
		audioClient->GetCurrentPadding(&padding);
		uint32_t framesAvailable = bufferFrameCount - padding;

		if (framesAvailable == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		float* buffer = nullptr;
		renderClient->GetBuffer(framesAvailable,
								reinterpret_cast<BYTE**>(&buffer));

		static std::mt19937 gen(std::random_device{}());
		static std::uniform_real_distribution dist(-1.0f, 1.0f);
		for (uint32_t i = 0; i < framesAvailable; i++)
		{
			float sample = dist(gen);
			buffer[i * mixFormat->nChannels + 0] = sample;
			if (mixFormat->nChannels > 1)
				buffer[i * mixFormat->nChannels + 1] = sample;

		}

		imGuiLayer.BeginFrame();
		app->Render();

		renderClient->ReleaseBuffer(framesAvailable, 0);
		window.SwapBuffers(&imGuiLayer);

	}

	audioClient->Stop();
	renderClient->Release();
	audioClient->Release();
	device->Release();
	deviceEnumerator->Release();
	CoUninitialize();
	return EXIT_SUCCESS;
}
