#pragma once
#ifdef _WIN32
#define NOMINMAX
#endif

#include <Audioclient.h>
#include <expected>
#include <mmdeviceapi.h>
#include <stdint.h>
#include <string>

#include "AudioTypes.hpp"

namespace MT::Core::Audio
{
struct AudioBuffer
{
	float* Data{nullptr};
	uint32_t Frames{0};
};

/**
 * @brief Low-level WASAPI audio backend responsible for device interaction and buffer management.
 *
 * The AudioBackend class provides a thin abstraction over the Windows Audio Session API (WASAPI),
 * handling device initialisation, audio stream control, and buffer access for real-time audio output.
 *
 * Core responsibilities:
 * - Initialise and manage COM-based audio interfaces.
 * - Acquire the default system audio output device.
 * - Configure and maintain the audio client and render client.
 * - Provide access to the system mix format.
 * - Manage audio buffers for writing sample data.
 * - Control playback state (start, stop, pause, resume).
 *
 * Audio workflow:
 * - Create() initialises COM and sets up all required WASAPI interfaces.
 * - GetBuffer() provides a writable buffer for audio samples.
 * - The caller writes interleaved float samples into the buffer.
 * - ReleaseBuffer() submits the written samples to the audio device.
 * - Playback is controlled via StartPlayback(), StopPlayback(), and pause/resume functions.
 *
 * Buffer management:
 * - Uses shared mode with the system mix format.
 * - Dynamically queries available frames to avoid buffer overruns.
 * - Ensures safe acquisition and release of audio buffers.
 *
 * Playback states:
 * - STOPPED: Audio client is reset and not rendering.
 * - PLAYING: Audio stream is actively being rendered.
 * - PAUSED: Audio stream is halted but buffer state is preserved.
 *
 * Design notes:
 * - Intended for real-time audio streaming.
 * - Uses std::expected for internal error reporting where appropriate.
 * - Automatically releases all COM resources on destruction via Shutdown().
 * - Assumes float-based audio data compatible with the system mix format.
 *
 * Typical usage:
 * - Call Create() once to initialise the backend.
 * - Call StartPlayback() to begin streaming.
 * - Repeatedly call GetBuffer() → write samples → ReleaseBuffer().
 * - Control playback using stop/pause/resume functions.
 * - Call Shutdown() (or rely on destructor) to clean up resources.
 *
 * This class is platform-specific (Windows) and forms the foundation for higher-level
 * audio systems such as the AudioEngine.
 */
class AudioBackend
{
public:
	~AudioBackend() { Shutdown(); }

	BackendError Create(DWORD comInitMode);
	void Shutdown() const;

	void StartPlayback();
	void StopPlayback();
	void PausePlayback();
	void ResumePlayback();

	PlaybackState GetPlaybackState() const;

	[[nodiscard]] const WAVEFORMATEX* GetFormat() const;
	[[nodiscard]] AudioBuffer GetBuffer() const;
	void ReleaseBuffer(uint32_t buffer) const;

private:
	std::expected<uint32_t, std::string> GetFramesAvailable() const;

private:
	IMMDeviceEnumerator* m_DeviceEnumerator{nullptr};
	IMMDevice* m_Device{nullptr};
	IAudioClient* m_AudioClient{nullptr};
	WAVEFORMATEX* m_MixFormat{nullptr};
	IAudioRenderClient* m_RenderClient{nullptr};

	uint32_t m_BufferFrameCount{0};
	PlaybackState m_State{PlaybackState::STOPPED};
};
}
