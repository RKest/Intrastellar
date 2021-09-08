#ifndef TIMER_H
#define TIMER_H
#include "_config.h"
#include "Core/text.h"
#include "Core/stats.h"
#include <chrono>
#include <ratio>
#include <list>
#include <numeric>

using _clock = std::chrono::steady_clock;
using milliDuration = std::chrono::duration<db, std::milli>;
using timePt = _clock::time_point;

class Clock
{
public:
	Clock(db &clockDelay, timePt &latestFrameTimePoint);
	bool IsItTime();
	db RemainingTime();
private:
	db &clockDelayDB;
	milliDuration clockDelay;
	timePt &latestFrameTimePoint;
	timePt lastRecordedPoint;
};

class Timer
{
public:
	Timer(Text &text, Stats &stats);
	enum ClocksEnum : ui
	{
		SHOT_CLOCK,
		SPAWN_CLOCK,
		NO_CLOCKS
	};

	void InitHeapClock(ui &heapClockId, db &clockDelay);
	void DestroyHeapClock(const ui clockId);
	bool HeapIsItTime(const ui heapClockId);
	db RemainingTime(ui heapClockId);

	void RenderFPS();
	void RecordFrame();
	bool IsItTime(ClocksEnum onWhichClock);
	db Scale(db number);
	~Timer();

protected:
private:
	Text &text;

	timePt lastFramePt;
	milliDuration lastFrameDuration;

	std::vector<Clock> clocks;
	ui newestHeapClockId = 0;
	std::vector<Clock *> heapClocks;

	//FPS
	milliDuration fpsDuration = milliDuration(0.0);
	std::list<ui> pastFPSValues;
	ui framesThisSecond = 0;
	const ui maxPolledFrames = 5;
};


#endif
