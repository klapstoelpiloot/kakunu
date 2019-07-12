/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <condition_variable>
#include <thread>
#include <mutex>
#include <chrono>

// Provides a mechanism for simple worker thread synchronization
class Synchronizer final
{
private:

	// Members used for synchronization
	std::mutex mutex;
	std::condition_variable cv;
	bool wakeup;

public:

	// Constructor
	Synchronizer() : wakeup(false) { }

	// This waits indefinitely until a signal is given
	void Wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		cv.wait(lock, [&]{ return wakeup; });
		wakeup = false;
	}

	// This waits until a signal is given or until the specified number of milliseconds have passed.
	// Returns False when the timeout was reached or True when the thread was signalled.
	bool Wait(int timeout_ms)
	{
		std::unique_lock<std::mutex> lock(mutex);
		bool result = cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&]{ return wakeup; });
		wakeup = false;
		return result;
	}

	// This wakes up the waiting threads
	void Signal()
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			wakeup = true;
		}
		cv.notify_all();
	}
};
