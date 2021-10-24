#pragma once
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

struct Clock
{
	Clock(db &clockDelay, timePt &latestFrameTimePoint);
	bool IsItTime(const db scalingFactor);
	db RemainingTime();

	db &clockDelayDB;
	milliDuration clockDelay;
	timePt &latestFrameTimePoint;
	timePt lastRecordedPoint;
};

class Timer
{
public:
	Timer(Text &text, PlayerStats &stats);
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
	void SetScalingFactor(const db arg);
	template<typename T>
	inline T Scale(T number)
	{
		return (number * static_cast<T>(lastFrameDuration.count() * scalingFactor));
	}
	~Timer();

protected:
private:
	Text &text;

	timePt lastFramePt;
	milliDuration lastFrameDuration;

	std::vector<Clock> clocks;
	ui newestHeapClockId = 0;
	std::vector<Clock *> heapClocks;

	db scalingFactor = 1.0;

	//FPS
	milliDuration fpsDuration = milliDuration(0.0);
	std::list<ui> pastFPSValues;
	ui framesThisSecond = 0;
	const ui maxPolledFrames = 5;
};

