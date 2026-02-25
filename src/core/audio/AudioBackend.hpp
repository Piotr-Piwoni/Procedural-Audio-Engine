#pragma once
#include <Audioclient.h>
#include <Windows.h>
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
