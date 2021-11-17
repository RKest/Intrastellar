#include "Core/timer.h"

db Clock::RemainingTime()
{
	return m_delay.count() - std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint).count();
}

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

void Timer::SetScalngFactor(const db arg)
{
	if(s_scalingFactor != arg)
	{
		s_scalingChangeFactor = s_scalingFactor * arg;
		scalingFactor = arg;
	}
}

// Clock::Clock(db &clockDelay, timePt_t &latestFrameTimePoint)
//  : clockDelayDB(clockDelay), clockDelay(clockDelay), latestFrameTimePoint(latestFrameTimePoint), lastRecordedPoint(latestFrameTimePoint)
// {
// }

// bool Clock::IsItTime(const db scalingFactor)
// {
// 	if(std::chrono::duration_cast<milliDuration_t>(latestFrameTimePoint - lastRecordedPoint) > clockDelay)
// 	{
// 		clockDelay = static_cast<milliDuration_t>(clockDelayDB / scalingFactor);
// 		lastRecordedPoint = latestFrameTimePoint;
// 		return true;
// 	}
// 	else
// 		return false;
// }

// db Clock::RemainingTime()
// {
// 	return clockDelay.count() - std::chrono::duration_cast<milliDuration_t>(latestFrameTimePoint - lastRecordedPoint).count();
// }

// Timer::Timer(Text &text, PlayerStats &stats)
// 	: text(text)
// {
// 	lastFramePt = _clock::now();
// 	clocks.push_back(Clock(stats.shotDelay, lastFramePt));
// 	clocks.push_back(Clock(stats.enemySpawnRate, lastFramePt));
// }
// void Timer::SetScalingFactor(const db arg)
// {
// 	if(scalingFactor != arg)
// 	{
// 		const db remainingScaleChange = scalingFactor * arg;
// 		for(Clock &clock: clocks)
// 		{
// 			clock.clockDelay = static_cast<milliDuration_t>(clock.RemainingTime() * remainingScaleChange);
// 		}
// 		scalingFactor = arg;
// 	}
// }
// bool Timer::IsItTime(ClocksEnum onWhichClock)
// {
// 	return clocks[onWhichClock].IsItTime(scalingFactor);
// }

// db Timer::RemainingTime(ui heapClockId)
// {
// 	assert(heapClocks.size() >= heapClockId && heapClocks[heapClockId] != nullptr);
// 	return heapClocks[heapClockId]->RemainingTime();
// }

// void Timer::RenderFPS()
// {
// 	const ui fpsValuesSize = decl_cast(fpsValuesSize, pastFPSValues.size());
// 	if(!fpsValuesSize)
// 		return;

// 	const ui averageFPS = std::accumulate(pastFPSValues.begin(), pastFPSValues.end(), 0) / fpsValuesSize;
// 	text.Render("FPS: " + std::to_string(averageFPS), static_cast<ft>(SCREEN_WIDTH) - 150.0f, static_cast<ft>(SCREEN_HEIGHT) - 30.0f, 0.5f, glm::vec3(1));
// }

// void Timer::RecordFrame()
// {
// 	timePt_t newFramePt = _clock::now();
// 	lastFrameDuration = newFramePt - lastFramePt;
// 	lastFramePt = newFramePt;

// 	//FPS
// 	fpsDuration += lastFrameDuration;
// 	framesThisSecond++;
// 	if(fpsDuration > milliDuration_t(1000.0))
// 	{
// 		fpsDuration = milliDuration_t(0.0);
// 		pastFPSValues.push_back(framesThisSecond);
// 		if(pastFPSValues.size() >= maxPolledFrames)
// 			pastFPSValues.pop_front();
// 		framesThisSecond = 0;
// 	}
// }

// void Timer::InitHeapClock(ui &heapClockId, db &clockDelay)
// {
// 	heapClockId = newestHeapClockId++;
// 	heapClocks.push_back(new Clock(clockDelay, lastFramePt));
// }

// void Timer::DestroyHeapClock(const ui heapClockId)
// {
// 	delete heapClocks[heapClockId];
// 	heapClocks[heapClockId] = nullptr;
// }

// bool Timer::HeapIsItTime(const ui heapClockId)
// {
// 	assert(heapClocks.size() >= heapClockId);
// 	assert(heapClocks[heapClockId] != nullptr);
// 	return heapClocks[heapClockId]->IsItTime(scalingFactor);
// }

// Timer::~Timer()
// {
// 	for (auto cl : heapClocks)
// 		delete cl;
// 	heapClocks.clear();
// }
