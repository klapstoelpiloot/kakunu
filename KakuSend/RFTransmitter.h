/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <vector>
#include "../KakuNu/Tools.h"

class RFTransmitter
{
private:

	// Constants
	const uint LOW_LEAD_TIME = 5000;

	// The output pin on which to transmit
	int pin;

	// Handles (only used for pigpio_if2)
	int pi;

	// Sleep for a specified number of microseconds
	void Sleep(uint us);

	// Sets the pin output level
	void SetPinLevel(uint level);

public:

	// Constructor / destructor
	RFTransmitter();
	virtual ~RFTransmitter();

	// This transmits the specified pulses
	void Send(int pidevice, int pin, const std::vector<uint>& times, int repeat);
};

