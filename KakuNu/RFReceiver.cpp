/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <assert.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#ifdef PIGPIO_IF2
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif
#include "RFReceiver.h"
#include "MicroClock.h"

// Global RFReceiver instance
RFReceiver* rfreceiverinstance = nullptr;

// Global interrupt callback
#ifdef PIGPIO_IF2
void RFReceiverPinChangeCallback(int pi, uint pin, uint level, uint tick, void* userdata)
#else
void RFReceiverPinChangeCallback(int pin, int level, uint tick, void* userdata)
#endif
{
	RFReceiver* ptr = reinterpret_cast<RFReceiver*>(userdata);
	ptr->PinChangeCallback(static_cast<uint>(level), tick);
}

// Constructor
RFReceiver::RFReceiver() :
	pin(0),
	pi(0),
	hcallback(0),
	startduration(DEFAULT_START_DURATION_US),
	endduration(DEFAULT_END_DURATION_US),
	minmessagetimes(DEFAULT_MIN_MESSAGE_TIMES),
	starttime(0),
	lasttime(0),
	laststate(0)
{
	// Make sure we are a singleton instance
	assert(rfreceiverinstance == nullptr);
	rfreceiverinstance = this;

	// Allocate memory for timings
	times.reserve(MAX_MESSAGE_TIMES);
}

// Destructor
RFReceiver::~RFReceiver()
{
	rfreceiverinstance = nullptr;
}

// This starts receiving on the specified pin
void RFReceiver::Start(int pidevice, int inputpin)
{
	std::lock_guard<std::mutex> lock(mutex);
	pi = pidevice;
	pin = inputpin;
	
	// Setup listening interrupt on input pin
	#ifdef PIGPIO_IF2
		hcallback = callback_ex(pi, pin, EITHER_EDGE, &RFReceiverPinChangeCallback, reinterpret_cast<void*>(this));
		if(hcallback < 0)
			std::cout << "Error setting up RFReceiverPinChangeCallback interrupt: " << strerror(errno) << std::endl;
	#else
		if(gpioSetISRFuncEx(pin, EITHER_EDGE, 0, &RFReceiverPinChangeCallback, reinterpret_cast<void*>(this)) != 0)
			std::cout << "Error setting up RFReceiverPinChangeCallback interrupt: " << strerror(errno) << std::endl;
	#endif
}

// Stops the receiver
void RFReceiver::Stop()
{
	std::lock_guard<std::mutex> lock(mutex);

	// Clean up
	#ifdef PIGPIO_IF2
		callback_cancel(static_cast<uint>(hcallback));
	#else
		gpioSetISRFuncEx(pin, EITHER_EDGE, 0, nullptr, nullptr);
	#endif
}

// Interrupt callback when pin state changes.
// This is not a real interrupt, but is simply called by wiringPi on a separate thread.
// To prevent race conditions, we must employ the proper synchronizations!
void RFReceiver::PinChangeCallback(uint level, uint tick)
{
	std::lock_guard<std::mutex> lock(mutex);

	// This is all about a change of state. If I get this interrupt for the same state twice,
	// then someone (pigpio programmer or raspbian programmer) fucked up his logic and that's
	// why I have to put this check here. I am disappointed.
	if(level == laststate)
		return;

	// Convert the time to our time in microseconds since the start of the clock.
	uint64 time = microclock.ConvertTime(tick);

	// If we are looking for the start of a new message...
	if(times.size() == 0)
	{
		// A new message can only begin with a rising edge
		if((starttime == 0) && (level > 0))
		{
			// Time the first rising edge
			starttime = time;
		}
		else if((starttime > 0) && (level == 0))
		{
			// First falling edge must be a minimum duration from the first rising edge
			if(((time - starttime) >= startduration) && ((time - starttime) < endduration))
			{
				// Potential message start. Start keeping times.
				times.push_back(static_cast<uint>(time - starttime));
			}
			else
			{
				// Just a fluke, ignore it.
				starttime = 0;
			}
		}
		else
		{
			// This occurs either when we have a falling edge without starttime or
			// a raising edge with starttime. Neither is a promising scenario.
			starttime = 0;
		}
	}
	else
	{
		// Add the time to the list
		times.push_back(static_cast<uint>(time - lasttime));

		// If there was a long time since the last change,
		// or the max number of message times has been reached,
		// then we should start with a new message.
		if(((time - lasttime) > endduration) || (times.size() == MAX_MESSAGE_TIMES))
		{
			// If this message looks legit, then invoke the callback!
			if(times.size() >= minmessagetimes)
			{
				if(msgcallback != nullptr)
					msgcallback(times, starttime);
			}

			// Start recording a new message
			times.clear();
			if(level > 0)
				starttime = time;
			else
				starttime = 0;
		}
	}

	// Note the last time and state
	lasttime = time;
	laststate = level;
}
