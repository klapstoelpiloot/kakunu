/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <errno.h>
#include <string.h>
#include <iostream>
#include <time.h>
#ifdef PIGPIO_IF2
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif
#include "../KakuNu/MicroClock.h"
#include "RFTransmitter.h"

// Constructor
RFTransmitter::RFTransmitter()
{
}

// Destructor
RFTransmitter::~RFTransmitter()
{
}

// This transmits the specified pulses
void RFTransmitter::Send(int pidevice, int pin, const std::vector<uint>& times, int repeat)
{
	this->pi = pidevice;
	this->pin = pin;

	// Setup pin
	#ifdef PIGPIO_IF2
		if(set_mode(pidevice, pin, PI_OUTPUT) != 0)
			std::cout << "Error setting up pin: " << strerror(errno) << std::endl;
	#else
		if(gpioSetMode(pin, PI_OUTPUT) != 0)
			std::cout << "Error setting up pin: " << strerror(errno) << std::endl;
	#endif

	// Let the pin be in a low state for a while before sending
	SetPinLevel(0);
	Sleep(LOW_LEAD_TIME);

	for(int r = 0; r < repeat; r++)
	{
		for(size_t i = 0; i < (times.size() - 1); i += 2)
		{
			// Send a pulse with high time and the with low time
			SetPinLevel(1);
			Sleep(times[i]);
			SetPinLevel(0);
			Sleep(times[i + 1]);
		}
	}
}

// Sets the pin output level
void RFTransmitter::SetPinLevel(uint level)
{
	#ifdef PIGPIO_IF2
		if(gpio_write(pi, pin, level) != 0)
			std::cout << "Error writing to pin: " << strerror(errno) << std::endl;
	#else
		if(gpioWrite(pin, level) != 0)
			std::cout << "Error writing to pin: " << strerror(errno) << std::endl;
	#endif
}

// Sleep for a specified number of microseconds
void RFTransmitter::Sleep(uint us)
{
	struct timespec ts, rem;
	ts.tv_sec  = 0;
	ts.tv_nsec = us * 1000;
	while(clock_nanosleep(CLOCK_REALTIME, 0, &ts, &rem))
		ts = rem;
}


