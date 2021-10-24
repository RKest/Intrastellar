#include "Core/timer.h"

Clock::Clock(db &clockDelay, timePt &latestFrameTimePoint)
 : clockDelayDB(clockDelay), clockDelay(clockDelay), latestFrameTimePoint(latestFrameTimePoint), lastRecordedPoint(latestFrameTimePoint)
{
}

bool Clock::IsItTime(const db scalingFactor)
{
	if(std::chrono::duration_cast<milliDuration>(latestFrameTimePoint - lastRecordedPoint) > clockDelay)
	{
		clockDelay = static_cast<milliDuration>(clockDelayDB / scalingFactor);
		lastRecordedPoint = latestFrameTimePoint;
		return true;
	}
	else
		return false;
}

db Clock::RemainingTime()
{
	return clockDelay.count() - std::chrono::duration_cast<milliDuration>(latestFrameTimePoint - lastRecordedPoint).count();
}

Timer::Timer(Text &text, PlayerStats &stats)
	: text(text)
{
	lastFramePt = _clock::now();
	clocks.push_back(Clock(stats.shotDelay, lastFramePt));
	clocks.push_back(Clock(stats.enemySpawnRate, lastFramePt));
}
void Timer::SetScalingFactor(const db arg)
{
	if(scalingFactor != arg)
	{
		const db remainingScaleChange = scalingFactor * arg;
		for(Clock &clock: clocks)
		{
			clock.clockDelay = static_cast<milliDuration>(clock.RemainingTime() * remainingScaleChange);
		}
		scalingFactor = arg;
	}
}
bool Timer::IsItTime(ClocksEnum onWhichClock)
{
	return clocks[onWhichClock].IsItTime(scalingFactor);
}

db Timer::RemainingTime(ui heapClockId)
{
	assert(heapClocks.size() >= heapClockId && heapClocks[heapClockId] != nullptr);
	return heapClocks[heapClockId]->RemainingTime();
}

void Timer::RenderFPS()
{
	const ui fpsValuesSize = decl_cast(fpsValuesSize, pastFPSValues.size());
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
	return heapClocks[heapClockId]->IsItTime(scalingFactor);
}

Timer::~Timer()
{
	for (auto cl : heapClocks)
		delete cl;
	heapClocks.clear();
}
