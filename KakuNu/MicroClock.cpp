/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <assert.h>
#ifdef PIGPIO_IF2
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif
#include "MicroClock.h"

// Global instance
MicroClock microclock;

// Constructor
MicroClock::MicroClock() :
	lasttime(0),
	currenttime(0),
	thread(nullptr)
{
}

// Destructor
MicroClock::~MicroClock()
{
	if(thread != nullptr)
	{
		// Stop the background thread
		threadstop.Signal();
		if(thread->joinable())
			thread->join();
		delete thread;
		thread = nullptr;
	}
}

// This starts the clock
void MicroClock::Start(int pidevice)
{
	std::lock_guard<std::mutex> lock(mutex);
	pi = pidevice;

	// Note our start time
	#ifdef PIGPIO_IF2
		lasttime = get_current_tick(pi);
	#else
		lasttime = gpioTick();
	#endif

	// Start the background thread
	assert(thread == nullptr);
	thread = new std::thread(std::bind(&MicroClock::GetTimeThread, this));
}

// This returns the current time in microseconds since Start was called
uint64 MicroClock::GetTime()
{
	std::lock_guard<std::mutex> lock(mutex);

	// Get the microseconds
	#ifdef PIGPIO_IF2
		uint t = get_current_tick(pi);
	#else
		uint t = gpioTick();
	#endif

	// Determine delta time since previous update
	// This takes care of time wrapping around to 0 when the counter passes the 32 bit limit.
	// TODO: Is the block below the same as just doing dt=t-lasttime without if statement?
	uint dt;
	if(t < lasttime)
		dt = (0xFFFFFFFF - lasttime) + t;
	else
		dt = t - lasttime;

	// Add the elapsed time to our internal time
	currenttime += static_cast<uint64>(dt);
	lasttime = t;

	return currenttime;
}

// This converts the given system time (in microseconds) to our time (microseconds since Start was called)
// The specified system time must be in the past (within the last 71 minutes) or the conversion will result
// in an incorrect time.
uint64 MicroClock::ConvertTime(uint systime)
{
	// Make sure we update our current time
	GetTime();

	std::lock_guard<std::mutex> lock(mutex);

	// Calculate how must systime is in the past.
	// If the systime is later than lasttime then lasttime must have
	// wrapped back to 0 in between.
	// TODO: Is the block below the same as just doing dt=lasttime-systime without if statement?
	uint dt;
	if(lasttime < systime)
		dt = (0xFFFFFFFF - systime) + lasttime;
	else
		dt = lasttime - systime;

	// Returns the time relative to the time Start was called.
	return currenttime - static_cast<uint64>(dt);
}

// Thread method
void MicroClock::GetTimeThread()
{
	while(true)
	{
		// If the thread is signalled to stop, leave immediately.
		// This line also automatically delays the thread.
		if(threadstop.Wait(THREAD_INTERVAL_MS))
			return;

		// Get the current time.
		// We must do this at least once in 71 minutes to
		// correctly keep track of longer time periods.
		GetTime();
	}
}
