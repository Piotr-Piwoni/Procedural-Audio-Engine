#pragma once
#include <cmath>
#include <numbers>

#include "ModulatorTypes.hpp"

namespace MT::Core::Audio
{
class Modulator
{
public:
	float Process(const float deltaTime)
	{
		m_Phase += m_Frequency * deltaTime;

		float wave = 0.f;
		switch (m_WaveType)
		{
		// Simple sine wave is used as an example in this modulator function.
		case ModulatorWave::SINE:
			{
				wave = std::sinf(m_Phase * 2.f * std::numbers::pi_v<float>);
				break;
			}
		}


		return wave * m_Depth;
	}


	void SetFrequency(const float fq) { m_Frequency = fq; }
	[[nodiscard]] float GetFrequency() const { return m_Frequency; }

	void SetDepth(const float depth) { m_Depth = depth; }
	[[nodiscard]] float GetDepth() const { return m_Depth; }

	void SetTarget(const ModulatorTarget target) { m_Target = target; }
	[[nodiscard]] ModulatorTarget GetTarget() const { return m_Target; }

	void SetWaveType(const ModulatorWave type) { m_WaveType = type; }
	[[nodiscard]] ModulatorWave GetWaveType() const { return m_WaveType; }

private:
	float m_Frequency{5.f};
	float m_Depth{0.1f};
	float m_Phase{0.f};
	ModulatorTarget m_Target{ModulatorTarget::FREQUENCY};
	ModulatorWave m_WaveType{ModulatorWave::SINE};
};
}
