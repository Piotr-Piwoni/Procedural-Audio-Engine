#pragma once

namespace MT::Core::Audio
{
class AudioEffect
{
public:
	virtual ~AudioEffect() = default;

	virtual float Process(float sample, float deltaTime) = 0;

	virtual void Reset() {}

	virtual void OnAudioLengthChanged(Duration length) {}
};
}
