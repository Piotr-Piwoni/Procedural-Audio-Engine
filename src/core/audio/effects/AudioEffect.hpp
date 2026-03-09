#pragma once

namespace MT::Core::Audio
{
class AudioEffect
{
public:
	virtual ~AudioEffect() = default;

	virtual float Generate(float sample, float deltaTime) = 0;

	virtual void Reset() {}
};
}
