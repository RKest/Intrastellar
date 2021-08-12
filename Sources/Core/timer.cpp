#include "Core/timer.h"

Timer::Timer(Text &text, const db enemySpawnFrequency, const db shootingFrequency) 
	: enemySpawnFrequency(enemySpawnFrequency), shootingFrequency(shootingFrequency), text(text)
{
	lastFramePt = _clock::now();
	lastShotPt = lastFramePt;
	lastSpawnPt = lastFramePt;
};

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


bool Timer::IsItTimeForShot()
{
	if(std::chrono::duration_cast<std::chrono::milliseconds>(lastFramePt - lastShotPt) > shootingFrequency)
	{
		lastShotPt = _clock::now();
		return true;
	}
	else
		return false;
}

bool Timer::IsItTimeForSpawn()
{
	if(std::chrono::duration_cast<std::chrono::milliseconds>(lastFramePt - lastSpawnPt) > enemySpawnFrequency)
	{
		lastSpawnPt = _clock::now();
		return true;
	}
	else
		return false;
}

db Timer::Scale(db number)
{
	return number * lastFrameDuration.count();
}

Timer::~Timer()
{
}
