/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <thread>
#include <atomic>

class SignalHandler final
{
private:

	// Members
	std::atomic<bool> exitsignal;
	std::thread* thread;

public:

	// Constructor/destructor
	SignalHandler();
	~SignalHandler();

	// This is the thread that waits for a signal and handles it.
	void Thread();

	// Getters/setters
	bool GetExitSignal() const { return exitsignal; }
};
