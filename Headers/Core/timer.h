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

//Clock
template <typename... T>
class Clock
{
public:
	Clock() = default;
	Clock(const db delay, const clockCB_t<T...> cb)
		: m_delayDB(delay), m_cb(cb), m_delay(delay) {}
	inline void Inspect(T... ts)
	{
		if (s_wasScalingFactorChanged)
			m_delay = static_cast<milliDuration_t>(RemainingTime() * s_scalingChangeFactor);

		if (std::chrono::duration_cast<milliDuration_t>(s_lastFramePt - m_latestTimePoint) > m_delay)
		{
			m_delay = static_cast<milliDuration_t>(m_delay / s_scalingFactor);
			m_latestTimePoint = s_lastFramePt;
			m_cb(ts...);
		}
	}
	inline void ManualInspect(auto cb)
	{
		if (s_wasScalingFactorChanged)
			m_delay = static_cast<milliDuration_t>(RemainingTime() * s_scalingChangeFactor);

		if (std::chrono::duration_cast<milliDuration_t>(s_lastFramePt - m_latestTimePoint) > m_delay)
		{
			m_delay = static_cast<milliDuration_t>(m_delay / s_scalingFactor);
			m_latestTimePoint = s_lastFramePt;
			cb();
		}
	}
	db RemainingTime();
	static void RecordFrame();
	static void SetScalngFactor(cosnt db arg);
	template <typename T>
	static inline T Scale(T num)
	{
		return (num * static_cast<T>(s_durationSinceLastFrame.count() * s_scalingFactor));
	}

private:
	const db m_delayDB;
	const clockCB_t<T...> m_cb;
	milliDuration_t m_delay;
	timePt_t m_latestTimePoint;

	static timePt_t s_lastFramePt = g_clock_t::now();
	static milliDuration_t s_durationSinceLastFrame;

	// Scaling
	static db s_scalingFactor;
	static db s_scalingChangeFactor{1.0};
	static bool s_wasScalingFactorChanged{};

	// FPS
	static milliDuration_t s_fpsDuration = milliDuration_t(0.0);
	static std::list<ui> s_pastFPSValues;
	static ui s_framesThisSecond{};
	static const ui s_maxPolledFrames{5};
};

using clk = Clock<>;