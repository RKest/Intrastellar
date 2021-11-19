#include "Core/timer.h"

void Timer::RecordFrame()
{
	timePt_t newFramePt = g_clock_t::now();
	s_durationSinceLastFrame = newFramePt - s_lastFramePt;
	s_lastFramePt = newFramePt;

	//FPS
	s_fpsDuration += s_durationSinceLastFrame;
	s_framesThisSecond++;
	if(s_fpsDuration > milliDuration_t(1000.0))
	{
		s_fpsDuration = milliDuration_t(0.0);
		s_pastFPSValues.push_back(s_framesThisSecond);
		if(s_pastFPSValues.size() >= s_maxPolledFrames)
			s_pastFPSValues.pop_front();
		s_framesThisSecond = 0;
	}
}

void Timer::RenderFPS()
{
	const ui fpsValuesSize = decl_cast(fpsValuesSize, s_pastFPSValues.size());
	if(!fpsValuesSize)
		return;

	const ui averageFPS = std::accumulate(s_pastFPSValues.begin(), s_pastFPSValues.end(), 0) / fpsValuesSize;
	Text::Render("FPS: " + std::to_string(averageFPS), static_cast<ft>(SCREEN_WIDTH) - 150.0f, static_cast<ft>(SCREEN_HEIGHT) - 30.0f, 0.5f, glm::vec3(1));
}

void Timer::SetScalingFactor(const db arg)
{
	if(s_scalingFactor != arg)
	{
		s_scalingChangeFactor = s_scalingFactor * arg;
		s_scalingFactor = arg;
	}
}
