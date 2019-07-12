/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <iostream>
#include <errno.h>
#include <string.h>
#ifdef PIGPIO_IF2
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif
#include "../KakuNu/MicroClock.h"
#include "KakuEncoder.h"
#include "RFTransmitter.h"

// ccxopts raises some warnings (and rightfully so) about integer conversion
// but instead of messing with the source code I chose to silence these warnings.
// TODO: Update this source code when the author has a proper fix.
// https://github.com/jarro2783/cxxopts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "../KakuNu/cxxopts.hpp"
#pragma GCC diagnostic pop

// This lists the available options on the command line and parses the given options.
// Using the ParseResult we can easily determine what options were specified.
cxxopts::ParseResult ParseCommandLineOptions(int& argc, char**& argv)
{
	try
	{
		// List the available options
		cxxopts::Options options("kakusend", "KakuSend: KlikAanKlikUit transmitting tool");
		options
			.add_options()
			("help", "Shows information about the command line options.")
			("p", "BCM GPIO pin to transmit on", cxxopts::value<int>()->default_value("17"))
			("r", "Number of times to transmit the message", cxxopts::value<int>()->default_value("4"));
		options.custom_help("bitcode [options...]");

		// Parse the arguments with these options
		cxxopts::ParseResult cmdargs = options.parse(argc, argv);

		// If the user is just asking for help,
		// output the available command line options...
		if((argc < 2) || cmdargs.count("help"))
		{
			std::cout << options.help() << std::endl;
			std::cout << "Example:" << std::endl;
			std::cout << "   kakusend 11010101101011100010110000011000 -r 4" << std::endl;
			exit(0);
		}

		return cmdargs;
	}
	catch(const cxxopts::OptionException& e)
	{
		std::cout << "Error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}

// Main program entry
int main(int argc, char* argv[])
{
	KakuEncoder encoder;
	RFTransmitter transmitter;
	int pi = 0;

	// Parse command line options
	char** nargv = argv;
	cxxopts::ParseResult cmdargs = ParseCommandLineOptions(argc, nargv);

	#ifdef PIGPIO_IF2
		// Setup for use with pigpio_if2 (which locally connects to pigpiod)
		// We use this for most of the debugging, because Visual Studio doesn't
		// allow me to start the process with sudo (yet).
		pi = pigpio_start(nullptr, nullptr);
		if(pi < 0)
		{
			std::cout << "Error setting up pigpio_if2: " << strerror(errno) << std::endl;
			return 1;
		}
	#else
		// Setup for direct use of pigpio (requires sudo)
		if(gpioInitialise() == PI_INIT_FAILED)
		{
			std::cout << "Error setting up pigpio: " << strerror(errno) << std::endl;
			return 1;
		}
	#endif

	// Start the clock
	microclock.Start(pi);

	// Encode the specified bits into time pulses
	std::vector<uint> times;
	std::string code = nargv[1];
	std::string error = encoder.Encode(code, times);
	if(error.size() > 0)
	{
		std::cout << error << std::endl;
		return 1;
	}

	// Send the signal 4 times
	int pin = cmdargs["p"].as<int>();
	int repeat = cmdargs["r"].as<int>();
	std::cout << "Transmitting " << code << " on pin " << pin << "..." << std::endl;
	transmitter.Send(pi, pin, times, repeat);

	// Clean up
	#ifdef PIGPIO_IF2
		pigpio_stop(pi);
	#else
		gpioTerminate();
	#endif
	return 0;
}