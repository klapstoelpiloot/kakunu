/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <iostream>
#include <errno.h>
#include <string.h>
#include <functional>
#ifdef PIGPIO_IF2
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif
#include "MicroClock.h"
#include "RFReceiver.h"
#include "KakuDecoder.h"
#include "SignalHandler.h"
#include "InputHandler.h"

// ccxopts raises some warnings (and rightfully so) about integer conversion
// but instead of messing with the source code I chose to silence these warnings.
// TODO: Update this source code when the author has a proper fix.
// https://github.com/jarro2783/cxxopts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "cxxopts.hpp"
#pragma GCC diagnostic pop

using namespace std::placeholders;

// This lists the available options on the command line and parses the given options.
// Using the ParseResult we can easily determine what options were specified.
cxxopts::ParseResult ParseCommandLineOptions(int& argc, char**& argv)
{
	try
	{
		// List the available options
		cxxopts::Options options("kakunu", "KakuNu: KlikAanKlikUit listening tool");
		options
			.add_options()
			("help", "Shows information about the command line options.")
			("p", "BCM GPIO pin to listen on", cxxopts::value<int>()->default_value("27"));
		options.custom_help("[options...]");

		// Parse the arguments with these options
		cxxopts::ParseResult cmdargs = options.parse(argc, argv);

		// If the user is just asking for help,
		// output the available command line options...
		if(cmdargs.count("help"))
		{
			std::cout << options.help() << std::endl;
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

// This outputs results to std out
void OutputResults(const std::string& str)
{
	std::cout << str << std::endl;
}

// Main program entry
int main(int argc, char* argv[])
{
	RFReceiver receiver;
	KakuDecoder decoder;
	int pi = 0;

	// Parse command line options
	char** nargv = argv;
	cxxopts::ParseResult cmdargs = ParseCommandLineOptions(argc, nargv);

	// Set up the signal handler
	// This MUST be done before ANY threads are created, because it sets some
	// settings on the main thread that must apply (inherit) for all other threads!
	SignalHandler sighandler;

	// Set up the input handler
	InputHandler inputhandler(true);

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

	// Setup decoder
	decoder.SetResultCallback(std::bind(&OutputResults, _1));
	decoder.SetErrorCallback(std::bind(&OutputResults, _1));

	// Start the RF receiver
	int pin = cmdargs["p"].as<int>();
	std::cout << "Listening on pin " << pin << ". Press ENTER to exit." << std::endl;
	receiver.SetMessageCallback(std::bind(&KakuDecoder::DecodeMessage, &decoder, _1, _2));
	receiver.Start(pi, pin);

	// Sleep this thread until exit request is signalled
	while(!sighandler.GetExitSignal() && !inputhandler.GetExitSignal())
	{
		// Sleep for 100ms
		time_sleep(0.1);
	}

	// Clean up
	receiver.Stop();
	#ifdef PIGPIO_IF2
		pigpio_stop(pi);
	#else
		gpioTerminate();
	#endif
	std::cout << "Bye!" << std::endl;
	return 0;
}
