/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include "Tools.h"
#include "Synchronizer.h"

class KakuDecoder
{
private:

	// Timings
	// Anyhting in between these ranges is unsure and thus invalid.
	// Anything longer than MIN_MEGALONG_US is considered mega long.
	const uint MIN_SHORT_US = 100;
	const uint MAX_SHORT_US = 500;
	const uint MIN_LONG_US = 900;
	const uint MAX_LONG_US = 1800;
	const uint MIN_EXTRALONG_US = 2000;
	const uint MAX_EXTRALONG_US = 3200;
	const uint MIN_MEGALONG_US = 5000;

	// Coding scheme for timings
	enum class Timecode : int
	{
		Short = 0,
		Long = 1,
		ExtraLong = 2,
		MegaLong = 3
	};

	// To alleviate the callback from RFReceiver, we store the received data
	// in an array and process the data in a separate thread.
	std::queue<std::vector<uint>> receivedtimes;
	std::mutex receivemutex;

	// The thread for processing
	std::thread processingthread;
	Synchronizer threadsignal;
	std::atomic<bool> stopprocessingthread;
	void ProcessingThread();

	// This crunches the numbers
	void Decode(const std::vector<uint>& times);

	// Callbacks invoked for the results
	std::function<void(const std::string& result)> resultcallback;
	std::function<void(const std::string& message)> errorcallback;

public:

	// Constructor / destructor
	KakuDecoder();
	virtual ~KakuDecoder();

	// This starts decoding a message
	void DecodeMessage(const std::vector<uint>& times, uint64 starttime);

	// Getters/setters
	void SetResultCallback(std::function<void(const std::string& result)> f) { resultcallback = f; }
	void SetErrorCallback(std::function<void(const std::string& message)> f) { errorcallback = f; }
};
