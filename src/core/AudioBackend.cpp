#include "AudioBackend.hpp"

#include <iostream>
#include <print>


/**
 * @brief Creates and initialises the WASAPI audio backend.
 *
 * This function initialises COM for the current thread, enumerates the default
 * audio output device, activates an IAudioClient, retrieves the system mix
 * format, and acquires an IAudioRenderClient for audio playback.
 *
 * On failure, any partially initialised resources are released and an
 * appropriate BackendError is returned.
 *
 * @param comInitMode COM initialisation mode (e.g. COINIT_MULTITHREADED).
 * @return BackendError::NONE on success, otherwise a specific error code.
 */
MT::Core::BackendError MT::Core::AudioBackend::Create(const DWORD comInitMode)
{
	// Initialize the COM library for the calling thread.
	HRESULT result = CoInitializeEx(nullptr, comInitMode);
	if (FAILED(result))
	{
		std::cerr << "Failed to initialize the COM objects!\n";
		return BackendError::COM_INIT;
	}

	// Create the multimedia device enumerator to query audio endpoints.
	result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  nullptr,
							  CLSCTX_ALL,
							  __uuidof(IMMDeviceEnumerator),
							  IID_PPV_ARGS_Helper(&m_DeviceEnumerator));
	if (FAILED(result))
	{
		std::cerr << "Failed to create the device enumerator!\n";
		return BackendError::DEVICE_ENUM;
	}

	// Retrieve the default audio rendering device.
	result = m_DeviceEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole, &m_Device);
	if (FAILED(result))
	{
		std::cerr << "Failed to get default audio device!\n";
		Shutdown();
		return BackendError::DEVICE;
	}

	// Activate the audio client interface used to manage the audio stream.
	result = m_Device->Activate(__uuidof(IAudioClient),
								CLSCTX_ALL,
								nullptr,
								IID_PPV_ARGS_Helper(&m_AudioClient));
	if (FAILED(result))
	{
		std::cerr << "Failed to activate the audio client!\n";
		Shutdown();
		return BackendError::CLIENT_ACTIVE;
	}

	// Query the system mix format.
	result = m_AudioClient->GetMixFormat(&m_MixFormat);
	if (FAILED(result))
	{
		std::cerr << "Failed to get the mix format!\n";
		Shutdown();
		return BackendError::FORMAT;
	}

	// Initialize the audio client in shared mode using the system mix format.
	result = m_AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
									   0, 0, 0,
									   m_MixFormat,
									   nullptr);
	if (FAILED(result))
	{
		std::cerr << "Failed to initialize the audio client!\n";
		return BackendError::CLIENT_INIT;
	}

	std::println("Audio client initialized successfully!");

	// Obtain the render client used to write audio into the buffer.
	result = m_AudioClient->GetService(__uuidof(IAudioRenderClient),
									   IID_PPV_ARGS_Helper(&m_RenderClient));
	if (FAILED(result))
	{
		std::cerr << "Failed to get the audio render client!\n";
		return BackendError::RENDER_CLIENT;
	}

	return BackendError::NONE;
}

/**
 * @brief Shuts down the audio backend and releases all resources.
 *
 * Releases all COM interfaces, frees the mix format memory, and uninitialises
 * COM for the current thread. After calling this function, the backend is no
 * longer usable.
 */
void MT::Core::AudioBackend::Shutdown() const
{
	if (m_RenderClient)
		m_RenderClient->Release();
	if (m_AudioClient)
		m_AudioClient->Release();
	if (m_Device)
		m_Device->Release();
	if (m_DeviceEnumerator)
		m_DeviceEnumerator->Release();
	if (m_MixFormat)
		CoTaskMemFree(m_MixFormat);

	CoUninitialize();
}

/**
 * @brief Starts audio playback.
 *
 * Retrieves the size of the audio buffer in frames and begins streaming audio
 * to the output device.
 */
void MT::Core::AudioBackend::StartPlayback()
{
	if (FAILED(m_AudioClient->GetBufferSize(&m_BufferFrameCount)))
	{
		std::cerr << "Failed to get buffer size!\n";
		return;
	}

	if (FAILED(m_AudioClient->Start()))
		std::cerr << "Failed to start playback!\n";
}

/**
 * @brief Retrieves the system mix format.
 *
 * @return Pointer to a constant WAVEFORMATEX structure describing the audio
 *         format, or nullptr if the format is uninitialised.
 */
const WAVEFORMATEX* MT::Core::AudioBackend::GetFormat() const
{
	if (!m_MixFormat)
	{
		std::cerr << "Mix format is uninitialized!\n";
		return nullptr;
	}
	return m_MixFormat;
}

/**
 * @brief Acquires an audio buffer for writing samples.
 *
 * Queries the number of available frames and retrieves a pointer to the render
 * buffer. The returned AudioBuffer must be released using ReleaseBuffer() once
 * writing is complete.
 *
 * @return AudioBuffer containing the sample pointer and frame count, or an
 *         empty AudioBuffer on failure.
 */
MT::Core::AudioBuffer MT::Core::AudioBackend::GetBuffer() const
{
	if (!m_RenderClient)
	{
		std::cerr << "Render client is uninitialized!\n";
		return {};
	}

	float* data;
	const auto availableFrames = GetFramesAvailable();
	if (availableFrames.has_value())
	{
		if (FAILED(m_RenderClient->GetBuffer(availableFrames.value(),
			reinterpret_cast<BYTE**>(&data))))
		{
			std::cerr << "Failed to obtain the buffer!\n";
			return {};
		}

		return {.Data = data, .Frames = availableFrames.value()};
	}

	std::cerr << availableFrames.error() << '\n';
	return {};
}

/**
 * @brief Releases a previously acquired audio buffer.
 *
 * @param buffer Number of frames that were written to the buffer.
 */
void MT::Core::AudioBackend::ReleaseBuffer(const uint32_t buffer) const
{
	if (!m_RenderClient)
	{
		std::cerr << "Render client is uninitialized!\n";
		return;
	}

	if (FAILED(m_RenderClient->ReleaseBuffer(buffer, 0)))
		std::cerr << "Failed to release the buffer!\n";
}


/**
 * @brief Calculates the number of frames currently available for writing.
 *
 * Computes the available frame count by subtracting the current padding from
 * the total buffer size.
 *
 * @return An expected containing the number of writable frames, or an error
 *         message if the query fails.
 */
std::expected<uint32_t, std::string>
MT::Core::AudioBackend::GetFramesAvailable() const
{
	if (!m_AudioClient)
		return std::unexpected<std::string>("Audio client is uninitialized!");
	if (m_BufferFrameCount == 0)
		return std::unexpected<std::string>("Frame buffer is empty!");

	uint32_t padding = 0;
	if (FAILED(m_AudioClient->GetCurrentPadding(&padding)))
		return std::unexpected<std::string>(
				"Failed to retrieve the current padding.");

	return m_BufferFrameCount - padding;
}
