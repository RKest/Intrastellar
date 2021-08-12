#ifndef TIMER_H
#define TIMER_H
#include "_config.h"
#include "Core/text.h"
#include <chrono>
#include <ratio>
#include <list>
#include <numeric>

using _clock = std::chrono::steady_clock;
using milliDuration = std::chrono::duration<db, std::milli>;
using timePt = _clock::time_point;

class Timer
{
public:
	Timer(Text &text, const db enemySpawnFrequency, const db shootingFrequency);
	void RenderFPS();
	void RecordFrame();
	bool IsItTimeForShot();
	bool IsItTimeForSpawn();
	db Scale(db number);

	~Timer();

protected:
private:
	Text &text;

	const milliDuration enemySpawnFrequency;
	const milliDuration shootingFrequency;

	timePt lastFramePt;
	milliDuration lastFrameDuration;
	
	timePt lastShotPt;
	timePt lastSpawnPt;

	milliDuration fpsDuration = milliDuration(0.0);
	std::list<ui> pastFPSValues;
	ui framesThisSecond = 0;
	const ui maxPolledFrames = 5;
};


#endif
