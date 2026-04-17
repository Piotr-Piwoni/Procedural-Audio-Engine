#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>

#include "AudioEffect.hpp"
#include "../../../Utilities/TypeDefinitions.hpp"

namespace MT::Core::Audio
{
class FadeEffect : public AudioEffect
{
public:
	explicit FadeEffect(const float inPointSeconds = 1.f,
						const float outPointSeconds = 1.f,
						const float audioLengthSeconds = 0.f) :
		m_FadeInPoint{inPointSeconds},
		m_FadeOutPoint{outPointSeconds},
		m_AudioLength{audioLengthSeconds} {}

	float Process(const float sample, const float deltaTime) override
	{
		m_Time += Duration(deltaTime);

		float envelope = 1.f;

		// Fade-in.
		if (m_FadeInPoint.count() > 0.f && m_Time < m_FadeInPoint)
			envelope = m_Time.count() / m_FadeInPoint.count();

		// Fade-out.
		if (m_AudioLength.count() > 0.f && m_FadeOutPoint.count() > 0.f &&
			m_Time > m_AudioLength - m_FadeOutPoint)
		{
			const float remaining = (m_AudioLength - m_Time).count();
			envelope = std::min(envelope, remaining / std::max(
												  m_FadeOutPoint.count(),
												  0.0001f));
		}

		if (m_Time >= m_AudioLength && m_AudioLength.count() > 0.f)
			Reset();

		return sample * std::clamp(envelope, 0.f, 1.f);
	}

	void Reset() override { m_Time = Duration::zero(); }

	bool RenderUI() override
	{
		ImGui::PushID(this);

		if (ImGui::CollapsingHeader("Fade Effect"))
		{
			if (ImGui::Button("X"))
			{
				Reset();
				ImGui::PopID();
				return false;
			}

			float inPoint = m_FadeInPoint.count();
			const float max = m_AudioLength.count() > 0.f
							  ? m_AudioLength.count() : FLT_MAX;
			if (ImGui::DragFloat("Fade Start (seconds)", &inPoint, 1.f, 0.f,
								 max, "%0.3f", ImGuiSliderFlags_AlwaysClamp))
				SetFadeInPoint(inPoint);

			if (m_AudioLength.count() > 0.f)
			{
				float outPoint = m_FadeOutPoint.count();
				if (ImGui::DragFloat("Fade Out (seconds)", &outPoint, 1.f, 0.f,
									 m_AudioLength.count(), "%0.3f",
									 ImGuiSliderFlags_AlwaysClamp))
					SetFadeOutPoint(outPoint);
			}
		}

		ImGui::PopID();
		return true;
	}


	void SetFadeInPoint(const float timeStamp)
	{
		if (timeStamp < 0.f)
		{
			std::cerr << "The provided Fade-In timestamp "
					"can't be lower than 0!\n";
			return;
		}

		m_FadeInPoint = Duration(timeStamp);
	}

	[[nodiscard]] Duration GetFadeInPoint() const { return m_FadeInPoint; }

	void SetFadeOutPoint(float timeStamp)
	{
		// If the provided fade-out is larger than the length clip, set it to that length.
		if (m_AudioLength.count() > 0.f)
			timeStamp = std::min(timeStamp, m_AudioLength.count());
		m_FadeOutPoint = Duration(timeStamp);
	}

	[[nodiscard]] Duration GetFadeOutPoint() const { return m_FadeOutPoint; }

	void OnAudioLengthChanged(const Duration length) override
	{
		m_AudioLength = length;
	}

private:
	Duration m_FadeInPoint{0.f};
	Duration m_FadeOutPoint{0.f};
	// 0 for "m_AudioLength" means the track is infinite.
	Duration m_AudioLength{0.f};
	Duration m_Time{0.f};
};
}
