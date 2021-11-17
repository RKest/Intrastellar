#pragma once
#include "_config.h"
#include "Core/text.h"
#include "Core/stats.h"
#include <chrono>
#include <ratio>
#include <list>
#include <numeric>
#include <functional>

using g_clock_t = std::chrono::steady_clock;
using milliDuration_t = std::chrono::duration<db, std::milli>;
using timePt_t = g_clock_t::time_point;
template <typename... T>
using clockCB_t = std::function<void(T...)>;

struct Timer
{
	friend class Clock;
	static void RecordFrame();
	static void SetScalngFactor(cosnt db arg);
	template <typename T>
	inline static T Scale(T num)
	{
		return (num * static_cast<T>(s_durationSinceLastFrame.count() * s_scalingFactor));
	}
private:
	inline static timePt_t s_lastFramePt = g_clock_t::now();
	inline static milliDuration_t s_durationSinceLastFrame;

	// Scaling
	inline static db s_scalingFactor;
	inline static db s_scalingChangeFactor{ 1.0 };
	inline static bool s_wasScalingFactorChanged{};

	// FPS
	inline static milliDuration_t s_fpsDuration = milliDuration_t( 0.0 );
	inline static std::list<ui> s_pastFPSValues;
	inline static ui s_framesThisSecond{};
	inline static const ui s_maxPolledFrames{ 5 };
};

template <typename... T>
class Clock
{
public:
	Clock() = default;
	Clock(const db delay, const clockCB_t<T...> cb)
		: m_delayDB(delay), m_cb(cb), m_delay(delay) {}
	inline void Inspect(T... ts)
	{
		if (Timer::s_wasScalingFactorChanged)
			m_delay = static_cast<milliDuration_t>(RemainingTime() * s_scalingChangeFactor);

		if (std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint) > m_delay)
		{
			m_delay = static_cast<milliDuration_t>(m_delay / Timer::s_scalingFactor);
			m_latestTimePoint = Timer::s_lastFramePt;
			m_cb(ts...);
		}
	}
	inline void ManualInspect(auto cb)
	{
		if (Timer::s_wasScalingFactorChanged)
			m_delay = static_cast<milliDuration_t>(RemainingTime() * Timer::s_scalingChangeFactor);

		if (std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint) > m_delay)
		{
			m_delay = static_cast<milliDuration_t>(m_delay / Timer::s_scalingFactor);
			m_latestTimePoint = Timer::s_lastFramePt;
			cb();
		}
	}
	db RemainingTime();

private:
	const db m_delayDB;
	const clockCB_t<T...> m_cb;
	milliDuration_t m_delay;
	timePt_t m_latestTimePoint;
};
