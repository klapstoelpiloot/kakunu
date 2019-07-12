/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <thread>
#include <mutex>
#include "Tools.h"
#include "Synchronizer.h"

/*
	This implements a wrapper for "micros" (wiringPi) to provide a clock
	which keeps time in microseconds and does not wrap around for 579783 years
	since the Start() method is called.
*/
class MicroClock
{
private:

	// Constants
	const int THREAD_INTERVAL_MS = 60000;

	// Handles (only used for pigpio_if2)
	int pi;

	// Last time when the time was checked
	uint lasttime;

	// Current time since Start was called
	uint64 currenttime;

	// Thread which regularly checks time
	std::mutex mutex;
	std::thread* thread;
	Synchronizer threadstop;

	// Thread method
	void GetTimeThread();

public:

	// Constructor / destructor
	MicroClock();
	virtual ~MicroClock();

	// This starts the clock
	void Start(int pidevice);

	// This returns the current time in microseconds since Start was called
	uint64 GetTime();

	// This converts the given system time (in microseconds) to our time (microseconds since Start was called)
	// The specified system time must be in the past (within the last 71 minutes) or the conversion will result
	// in an incorrect time.
	uint64 ConvertTime(uint systime);
};

// Global instance
extern MicroClock microclock;
