#include <chrono>
#include "Baduit/Timer.hpp"
using namespace std::chrono_literals;

#ifndef _CLOCKLOCK_HPP_
#define _CLOCKLOCK_HPP_

class ClockLock
{
public:
	ClockLock(int framePerSeconds)
	{
		resetClock(framePerSeconds);
	}

	bool isTimeLeftOnFrame()
	{
		if (((_microSecondsPerFrame - (std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count())) / 1000.0) > 0)
			return (true);
		return (false);
	}

	void frame()
	{
		if (isPaused == true)
			return;
		++_nbrFrame;
        _end = std::chrono::steady_clock::now();
		long long timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count();
		_totalWorkedMicroSeconds += timeElapsed;
		double time = (_microSecondsPerFrame - timeElapsed) / 1000.0;
        // why switch to milliseconds ?
		Baduit_Timer::Sleeper sleeper(std::chrono::milliseconds((int)time));
		_start = std::chrono::steady_clock::now();
		_totalSleepedMicroSeconds += std::chrono::duration_cast<std::chrono::microseconds>(_start - _end).count();
	}

	void pause()
	{
		_end = std::chrono::steady_clock::now();
		isPaused = true;
		long long timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count();
		_totalWorkedMicroSeconds += timeElapsed;
	}

	void resume()
	{
		++_nbrFrame;
		isPaused = false;
		_start = std::chrono::steady_clock::now();
	}

	int getNbrFrame()
	{
		return (_nbrFrame);
	}

	void resetClock(int framePerSeconds)
	{
		if (framePerSeconds <= 0)
			_framePerSeconds = 1;
		else
			_framePerSeconds = framePerSeconds;
		_microSecondsPerFrame = 1000000 / _framePerSeconds;
		_totalWorkedMicroSeconds = 0;
		_totalSleepedMicroSeconds = 0;
		_nbrFrame = 0;
		_start = std::chrono::steady_clock::now();
	}

	double getTotalTimeSeconds()
	{
		return ((double)(_totalWorkedMicroSeconds + _totalSleepedMicroSeconds) / 1000000.0);
	}

	double getAverageTimePerFrameSeconds()
	{
		return (((double)_totalWorkedMicroSeconds / (double)_nbrFrame) / 1000000.0);
	}

	double getSecondWorked()
	{
		return ((double)_totalWorkedMicroSeconds / 1000000.0);
	}

	double getSecondsSleeped()
	{
		return ((double)_totalSleepedMicroSeconds / 1000000.0);
	}
private:
	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::time_point<std::chrono::steady_clock> _end;
	int _nbrFrame = 0;
	long long _totalWorkedMicroSeconds = 0;
	long long _totalSleepedMicroSeconds = 0;
	int _framePerSeconds = 1;
	int _microSecondsPerFrame = 1000000;
	bool isPaused = false;
};

#endif // !_CLOCKLOCK_HPP_
