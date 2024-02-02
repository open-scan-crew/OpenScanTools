#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <condition_variable>

namespace Baduit_Timer
{
	using Duration = decltype(std::chrono::system_clock::now() - std::chrono::system_clock::now());

	/*
	** You can have the duration between now and the last reset of the clock.
	** It can't be paused.
	*/
	class SimpleClock
	{
	public:
		SimpleClock() { reset(); };

		SimpleClock(const SimpleClock&) = default;
		SimpleClock& operator=(const SimpleClock&) = default;

		SimpleClock(SimpleClock&&) = default;
		SimpleClock& operator=(SimpleClock&&) = default;

		virtual ~SimpleClock() = default;

		void		reset() { _first = std::chrono::system_clock::now(); }

		Duration	getTimeDuration() { return std::chrono::system_clock::now() - _first; }

		template<typename T>
		auto		getDuractionAs() { return std::chrono::duration_cast<T>(getTimeDuration()); }

		int64_t		getTimeNanoCount() { return getDuractionAs<std::chrono::nanoseconds>().count(); }

	private:
		std::chrono::system_clock::time_point _first;
	};

	/*
	** You can have the duration between now and the last reset of the clock.
	** It can be paused.
	** Default status at construction: started.
	*/
	class AdvancedClock
	{
	public:
		AdvancedClock() { reset(); }

		AdvancedClock(const AdvancedClock&) = default;
		AdvancedClock& operator=(const AdvancedClock&) = default;

		AdvancedClock(AdvancedClock&&) = default;
		AdvancedClock& operator=(AdvancedClock&&) = default;

		virtual ~AdvancedClock() = default;

		void		reset()
		{
			_run_timer.reset();
			_pause_timer.reset();
			_paused_time = {};
		}

		auto		getTimeDuration()
		{
			return _run_timer.getTimeDuration() - getTotalPauseTime();
		}

		template<typename T>
		auto		getDuractionAs()
		{
			return _run_timer.getDuractionAs<T>() - getTotalPauseTimeAs<T>();
		}

		int64_t		getTimeNanoCount()
		{
			return _run_timer.getTimeNanoCount() - getTotalPauseTimeNanoCount();
		}

		void		pause()
		{
			if (!_pause_timer.has_value())
				_pause_timer.emplace();
		}

		void		start()
		{
			if (_pause_timer.has_value())
			{
				_paused_time += _pause_timer->getTimeDuration();
				_pause_timer.reset();
			}
		}

		Duration	getTotalPauseTime()
		{
			return (_pause_timer.has_value()) ? _paused_time + _pause_timer->getTimeDuration() : _paused_time;
		}

		template<typename T>
		auto 	getTotalPauseTimeAs() { return std::chrono::duration_cast<T>(getTotalPauseTime()); }


		int64_t	getTotalPauseTimeNanoCount() { return getTotalPauseTimeAs<std::chrono::nanoseconds>().count(); }

	private:
		SimpleClock							_run_timer;
		std::optional<SimpleClock>			_pause_timer;
		Duration							_paused_time;
	};

	/*
	** Make a thread sleep, same as std::this_thread::sleep_for but cancelable
	*/
	class Sleeper
	{
	public:
		Sleeper() = default;

		Sleeper(const Sleeper&) = delete;
		Sleeper& operator=(const Sleeper&) = delete;

		Sleeper(Sleeper&&) = delete;
		Sleeper& operator=(Sleeper&&) = delete;

		virtual ~Sleeper()
		{
			cancel_all();
		}

		template<typename D>
		Sleeper(const D& duration)
		{
			sleep(duration);
		}

		template<typename D>
		void	operator()(const D& duration)
		{
			sleep(duration);
		}

		template<typename D>
		void	sleep(const D& duration)
		{
			std::unique_lock lock(_mutex);
			_cv.wait_for(lock, duration);
		}

		// Use it if you want to awake only one thread sleeping
		void	cancel_one()
		{
			_cv.notify_one();
		}

		// Use it if you cant to awake all the sleeping threads (one or more)
		void	cancel_all()
		{
			_cv.notify_all();
		}

	private:
		std::mutex				_mutex;
		std::condition_variable	_cv;
	};

	/*
	** Execute an action at the end of the choosed duration.
	** For the moment it can't be paused.
	*/
	class ThreadTimer
	{
	public:
		ThreadTimer() = default;

		ThreadTimer(const ThreadTimer&) = delete;
		ThreadTimer& operator=(const ThreadTimer&) = delete;

		ThreadTimer(ThreadTimer&&) = delete;
		ThreadTimer& operator=(ThreadTimer&&) = delete;

		template<typename D, typename Cb>
		ThreadTimer(const D& duration, Cb&& cb)
		{
			start(duration, std::move(cb));
		}

		virtual ~ThreadTimer()
		{
			stop();
		}

		template<typename D, typename Cb>
		auto	start(const D& duration, Cb&& cb)
		{
			auto thread_cb =
				[this, duration, moved_cb = std::move(cb)]()
			{
				_sleeper(duration);
				if (!_stop)
					return moved_cb();
				else
					return decltype(moved_cb())();
			};
			auto task = std::packaged_task<decltype(thread_cb())()>(std::forward<decltype(thread_cb)>(thread_cb));
			auto future = task.get_future();

			_thread = std::thread(std::move(task));
			return future;
		}

		// by calling this function while the timer is started, future used as result for the callback is unitialized
		void	stop()
		{
			_stop = true;
			_sleeper.cancel_all();
			if (_thread.joinable())
				_thread.join();
		}

	private:
		std::thread			_thread;
		std::atomic<bool>	_stop = false;
		Sleeper				_sleeper;
	};

	/*
	** Execute an action at the end of the choosed duration in a loop.
	** For the moment it can't be paused.
	*/
	class LoopThreadTimer
	{
	public:
		LoopThreadTimer() = default;

		LoopThreadTimer(const LoopThreadTimer&) = delete;
		LoopThreadTimer& operator=(const LoopThreadTimer&) = delete;

		LoopThreadTimer(LoopThreadTimer&&) = delete;
		LoopThreadTimer& operator=(LoopThreadTimer&&) = delete;

		template<typename D, typename Cb>
		LoopThreadTimer(const D& duration, Cb&& cb)
		{
			start(duration, std::move(cb));
		}

		virtual ~LoopThreadTimer()
		{
			stop();
		}

		template<typename D, typename Cb>
		void	start(const D& duration, Cb&& cb)
		{
			auto thread_cb =
				[this, duration, moved_cb = std::move(cb)]()
			{
				while (!_stop)
				{
					_sleeper(duration);
					if (!_stop)
						moved_cb();
				}
			};
			auto task = std::packaged_task<decltype(thread_cb())()>(std::forward<decltype(thread_cb)>(thread_cb));

			_thread = std::thread(std::move(task));
		}

		void	stop()
		{
			_stop = true;
			_sleeper.cancel_all();
			if (_thread.joinable())
				_thread.join();
		}

	private:
		std::thread			_thread;
		std::atomic<bool>	_stop{ false };
		Sleeper				_sleeper;
	};

}