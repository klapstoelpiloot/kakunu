/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <vector>
#include <mutex>
#include "Tools.h"

class RFReceiver
{
private:

	// Constants
	// The defaults are good for the 2019 kaku dimmer protocol
	const std::size_t MAX_MESSAGE_TIMES = 200;
	const uint64 DEFAULT_START_DURATION_US = 200;
	const uint64 DEFAULT_END_DURATION_US = 5000;
	const uint DEFAULT_MIN_MESSAGE_TIMES = 64;

	// The input pin on which to listen
	int pin;

	// Handles (only used for pigpio_if2)
	int pi;
	int hcallback;

	// Mutex for thread synchronization
	std::mutex mutex;

	// Delta times of received state changes in microseconds.
	// The first item is the duration of the first high state.
	std::vector<uint> times;

	// Minimum duration of a high state which indicates the start of a message, in microseconds.
	uint64 startduration;

	// Minimum duration of a low state which indicates the end of a message, in microseconds.
	uint64 endduration;

	// Minimum number of timings to record before we consider it a valid message.
	uint minmessagetimes;

	// Absolute time in microseconds of the first rising edge
	// for the current message being received.
	uint64 starttime;

	// Absolute time and state of the most recently received state change.
	uint64 lasttime;
	uint laststate;

	// Callback to invoke when a message has been received.
	std::function<void(const std::vector<uint>&, uint64)> msgcallback;

public:

	// Constructor / destructor
	RFReceiver();
	virtual ~RFReceiver();

	// This starts receiving on the specified pin
	void Start(int pidevice, int inputpin);

	// Stops the receiver
	void Stop();

	// Getters / setters
	void SetStartMessageDuration(uint64 microseconds) { startduration = microseconds; }
	uint64 GetStartMessageDuration() { return startduration; }
	void SetEndMessageDuration(uint64 microseconds) { endduration = microseconds; }
	uint64 GetEndMessageDuration() { return endduration; }
	void SetMinMessageTimes(uint minimumtimes) { minmessagetimes = minimumtimes; }
	uint GetMinMessageTimes() { return minmessagetimes; }
	void SetMessageCallback(std::function<void(const std::vector<uint>&, uint64)> f) { msgcallback = f; }

	// Interrupt callback when pin state changes.
	// Should not be called by users, only by the global RFReceiverPinChangeCallback().
	void PinChangeCallback(uint level, uint tick);
};
