/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <iostream>
#include <string>
#include <sstream>
#include "KakuDecoder.h"

// Constructor
KakuDecoder::KakuDecoder() :
	stopprocessingthread(false)
{
	// Start the background thread
	processingthread = std::thread(std::bind(&KakuDecoder::ProcessingThread, this));
}

// Destructor
KakuDecoder::~KakuDecoder()
{
	// Stop the background thread
	stopprocessingthread = true;
	threadsignal.Signal();
	processingthread.join();
}

// The thread for processing
void KakuDecoder::ProcessingThread()
{
	while(true)
	{
		receivemutex.lock();
		if(receivedtimes.size() == 0)
		{
			// There is no work to do.
			// Wait for a signal to indicate there is new work to do.
			receivemutex.unlock();
			threadsignal.Wait();

			// Stop processing?
			if(stopprocessingthread)
				return;

			receivemutex.lock();
		}

		// Copy a set of times to process
		std::vector<uint> times = receivedtimes.front();
		receivedtimes.pop();
		receivemutex.unlock();

		// Start crunching these numbers
		Decode(times);
	}
}

// This starts decoding a message
void KakuDecoder::DecodeMessage(const std::vector<uint>& times, uint64 starttime)
{
	std::lock_guard<std::mutex> lock(receivemutex);
	receivedtimes.push(times);
	threadsignal.Signal();
}


/*
	'0' subbit:
	 __ 
	|  |___

	|--|--|
	 T  T


	'1' subbit:
	 __ 
	|  |______________

	|--|-------------|
	 T       5T


	'START' signal:
	 __             
	|  |________________________________

	|--|-------------------------------|
	 T              10T


	'STOP' signal:
	 __             
	|  |________________________________ _ _ _ _____

	|--|-------------------------------- - - - ----|
	 T                      40T

	T ~ 250 us (Short)
	5T ~ 1250 us (Long)
	10T ~ 2500 us (ExtraLong)
	40T ~ 10 ms (MegaLong)

	Bit encoding: 0 bit = subbits 0 1
	              1 bit = subbits 1 0
	For example, the bits 0111 are encoded as 01101010.
	The extended protocol for dimmers contains additional bit combinations:
				  2 bit = subbits 0 0
				  3 bit = subbits 1 1
*/
// This crunches the numbers. This runs in the processing thread.
void KakuDecoder::Decode(const std::vector<uint>& times)
{
	// Check if we have received a minimum number of times to allow parsing
	if(times.size() < 6)
	{
		if(errorcallback != nullptr)
			errorcallback("Message could not be decoded. Insufficient data received.");
		return;
	}

	// First change the times into a code scheme which is easier to process.
	std::vector<Timecode> timecodes;
	timecodes.reserve(times.size());
	for(uint t : times)
	{
		if((t >= MIN_SHORT_US) && (t <= MAX_SHORT_US))
			timecodes.push_back(Timecode::Short);
		else if((t >= MIN_LONG_US) && (t <= MAX_LONG_US))
			timecodes.push_back(Timecode::Long);
		else if((t >= MIN_EXTRALONG_US) && (t <= MAX_EXTRALONG_US))
			timecodes.push_back(Timecode::ExtraLong);
		else if(t >= MIN_MEGALONG_US)
			timecodes.push_back(Timecode::MegaLong);
		else
		{
			if(errorcallback != nullptr)
				errorcallback("Message could not be decoded. Invalid timings received.");
			return;
		}
	}

	// Find the start of the message.
	// This consists of a short high and an extralong low. Because the timecodes are alternating high and low signals,
	// we scan through these with a step size of 2 so that 'i' is always at a high signal.
	size_t startindex = 0;
	for(size_t i = 0; i < (timecodes.size() - 2); i += 2)
	{
		if((timecodes[i] == Timecode::Short) && (timecodes[i + 1] == Timecode::ExtraLong))
		{
			startindex = i + 2;
			break;
		}
	}

	// The message never starts at index 0, because the start marker
	// (right before the actual message) is 2 signals long.
	if(startindex == 0)
	{
		if(errorcallback != nullptr)
			errorcallback("Message could not be decoded. Start marker not found.");
		return;
	}

	// Parse the timecodes into subbits.
	// Again, we do this in steps of 2 signals (a high and a low) because
	// all subbits and the end marker come in pairs.
	bool endmarkerfound = false;
	std::vector<int> subbits;
	subbits.reserve((timecodes.size() - startindex) / 2);
	for(size_t i = startindex; i < (timecodes.size() - 1); i += 2)
	{
		Timecode t1 = timecodes[i];
		Timecode t2 = timecodes[i + 1];
		
		if((t1 == Timecode::Short) && (t2 == Timecode::Short))
			subbits.push_back(0);
		else if((t1 == Timecode::Short) && (t2 == Timecode::Long))
			subbits.push_back(1);
		else if((t1 == Timecode::Short) && (t2 == Timecode::MegaLong))
		{
			endmarkerfound = true;
			break;
		}
		else
		{
			if(errorcallback != nullptr)
				errorcallback("Message could not be decoded. Invalid signals received.");
			return;
		}
	}

	// Check if the end marker is actually received.
	if(!endmarkerfound)
	{
		if(errorcallback != nullptr)
			errorcallback("Message could not be decoded. End marker not found.");
		return;
	}

	// Pair the subbits into bits.
	std::vector<int> bits;
	bits.reserve(subbits.size() / 2);
	for(size_t i = 0; i < (subbits.size() - 1); i += 2)
	{
		int sb1 = subbits[i];
		int sb2 = subbits[i + 1];

		if((sb1 == 0) && (sb2 == 1))
			bits.push_back(0);
		else if((sb1 == 1) && (sb2 == 0))
			bits.push_back(1);
		else if((sb1 == 0) && (sb2 == 0))
			bits.push_back(2);
		else
			bits.push_back(3);
	}

	if(resultcallback != nullptr)
	{
		// Make the result and invoke the callback
		std::stringstream str;
		for(int b : bits)
			str << b;
		resultcallback(str.str());
	}
}
