#include "Core/timer.h"

Clock::Clock(db &clockDelay, timePt &latestFrameTimePoint)
 : clockDelayDB(clockDelay), clockDelay(clockDelay), latestFrameTimePoint(latestFrameTimePoint), lastRecordedPoint(latestFrameTimePoint)
{
}

bool Clock::IsItTime()
{
	if(std::chrono::duration_cast<std::chrono::milliseconds>(latestFrameTimePoint - lastRecordedPoint) > clockDelay)
	{
		clockDelay = static_cast<milliDuration>(clockDelayDB);
		lastRecordedPoint = latestFrameTimePoint;
		return true;
	}
	else
		return false;
}

db Clock::RemainingTime()
{
	return clockDelayDB - std::chrono::duration_cast<std::chrono::milliseconds>(latestFrameTimePoint - lastRecordedPoint).count();
}

Timer::Timer(Text &text, PlayerStats &stats)
	: text(text)
{
	lastFramePt = _clock::now();
	clocks.push_back(Clock(stats.shotDelay, lastFramePt));
	clocks.push_back(Clock(stats.enemySpawnRate, lastFramePt));
}

bool Timer::IsItTime(ClocksEnum onWhichClock)
{
	return clocks[onWhichClock].IsItTime();
}

db Timer::RemainingTime(ui heapClockId)
{
	assert(heapClocks.size() >= heapClockId && heapClocks[heapClockId] != nullptr);
	return heapClocks[heapClockId]->RemainingTime();
}

void Timer::RenderFPS()
{
	const ui fpsValuesSize = pastFPSValues.size();
	if(!fpsValuesSize)
		return;

	const ui averageFPS = std::accumulate(pastFPSValues.begin(), pastFPSValues.end(), 0) / fpsValuesSize;
	text.Render("FPS: " + std::to_string(averageFPS), static_cast<ft>(SCREEN_WIDTH) - 150.0f, static_cast<ft>(SCREEN_HEIGHT) - 30.0f, 0.5f, glm::vec3(1));
}

void Timer::RecordFrame()
{
	timePt newFramePt = _clock::now();
	lastFrameDuration = newFramePt - lastFramePt;
	lastFramePt = newFramePt;

	//FPS
	fpsDuration += lastFrameDuration;
	framesThisSecond++;
	if(fpsDuration > milliDuration(1000.0))
	{
		fpsDuration = milliDuration(0.0);
		pastFPSValues.push_back(framesThisSecond);
		if(pastFPSValues.size() >= maxPolledFrames)
			pastFPSValues.pop_front();
		framesThisSecond = 0;
	}
}



db Timer::Scale(db number)
{
	return number * lastFrameDuration.count();
}

void Timer::InitHeapClock(ui &heapClockId, db &clockDelay)
{
	heapClockId = newestHeapClockId++;
	heapClocks.push_back(new Clock(clockDelay, lastFramePt));
}

void Timer::DestroyHeapClock(const ui heapClockId)
{
	delete heapClocks[heapClockId];
	heapClocks[heapClockId] = nullptr;
}

bool Timer::HeapIsItTime(const ui heapClockId)
{
	assert(heapClocks.size() >= heapClockId && heapClocks[heapClockId] != nullptr);
	return heapClocks[heapClockId]->IsItTime();
}

Timer::~Timer()
{
	for (auto cl : heapClocks)
		delete cl;
	heapClocks.clear();
}
