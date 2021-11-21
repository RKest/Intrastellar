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

enum struct ClockOptions : ui
{
	NOOP = 0b0,
	NOSCALE = 0b1
};

inline static constexpr ui operator&(ClockOptions a, ClockOptions b)
{
	return static_cast<ui>(a) & static_cast<ui>(b);
} 

struct Timer
{
	template <typename... T> friend class Clock;
	static void RecordFrame();
	static void SetScalingFactor(const db arg);
	static void RenderFPS();
	template <typename T>
	inline static T Scale(T num)
	{
		return (num * static_cast<T>(s_durationSinceLastFrame.count() * s_scalingFactor));
	}

	inline static bool s_inspectBool = false;
private:
	inline static ui 				s_nextClockId = 0;
	inline static timePt_t 			s_lastFramePt = g_clock_t::now();
	inline static milliDuration_t 	s_durationSinceLastFrame;

	// Scaling
	inline static db 				s_scalingFactor{ 1.0 };
	inline static db 				s_scalingChangeFactor;
	inline static bool 				s_wasScalingFactorChanged{};

	// FPS
	inline static milliDuration_t 	s_fpsDuration = milliDuration_t( 0.0 );
	inline static std::list<ui> 	s_pastFPSValues;
	inline static ui 				s_framesThisSecond{};
	inline static const ui 			s_maxPolledFrames{ 5 };
};

template <typename... T>
class Clock
{
public:
	Clock(const ClockOptions options = ClockOptions::NOOP)
		: m_clockId(Timer::s_nextClockId++), m_latestTimePoint(Timer::s_lastFramePt), m_options(options)
	{
	}
	Clock(const db delay, const clockCB_t<T...> cb, const ClockOptions options = ClockOptions::NOOP)
		: m_clockId(Timer::s_nextClockId++), m_latestTimePoint(Timer::s_lastFramePt), m_options(options)
	{
		Init(delay, cb);
	}
	Clock(Clock const&) = default;
	Clock(Clock &&) 	= default;
	Clock operator=(Clock &&rhs) 		{ return Clock(rhs); }
	Clock operator=(const Clock &rhs) 	{ return Clock(rhs); }
	inline void Init(const db delay, const clockCB_t<T...> cb)
	{
		m_latestTimePoint = Timer::s_lastFramePt;
		m_delayDB = delay;
		m_delay = milliDuration_t(delay);
		m_cb = cb;
	}
	template <ClockOptions Oops = ClockOptions::NOOP>
	inline bool Inspect(T... ts)
	{
		if constexpr (!(Oops & ClockOptions::NOSCALE))
			m_delay = milliDuration_t(m_delayDB / Timer::s_scalingFactor);
		if(Timer::s_inspectBool)
		{
			LOG("Delay:    ", m_delay.count());
			LOG("Duration: ", std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint).count());
		}
		if (std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint) > m_delay)
		{
			m_latestTimePoint = Timer::s_lastFramePt;
			m_cb(ts...);
			return true;
		}
		return false;
	}
	db RemainingTime()
	{
		return m_delay.count() - std::chrono::duration_cast<milliDuration_t>(Timer::s_lastFramePt - m_latestTimePoint).count();
	}
	inline ui ClockId() { return m_clockId; }

private:
	const ui m_clockId{};
	timePt_t m_latestTimePoint;
	const ClockOptions m_options;
	db m_delayDB{};
	clockCB_t<T...> m_cb{};
	milliDuration_t m_delay{};
};