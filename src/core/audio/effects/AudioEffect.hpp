#pragma once
#include "IMGUI/imgui.h"

namespace MT::Core::Audio
{
class AudioEffect
{
public:
	virtual ~AudioEffect() = default;

	virtual float Process(float sample, float deltaTime) = 0;

	virtual void Reset() {}

	virtual bool RenderUI() = 0;


	virtual void OnAudioLengthChanged(Duration length) {}
};
}
