/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include "SignalHandler.h"

// Constructor
SignalHandler::SignalHandler() :
	exitsignal(false),
	thread(nullptr)
{
	sigset_t signal_set;
	
	// Block all signals for the current (main) thread.
	// This must be done before ANY threads are created,
	// so that all threads inherit these settings.
	sigfillset(&signal_set);
	pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

	// Create a separate thread to handle signals
	thread = new std::thread(&SignalHandler::Thread, this);
}

// Destructor
SignalHandler::~SignalHandler()
{
	// Join the signal handling thread
	pthread_kill(thread->native_handle(), SIGUSR1);
	thread->join();

	// Clean up
	delete thread;
	thread = nullptr;
}

// This is the thread that waits for a signal and handles it.
void SignalHandler::Thread()
{
	sigset_t signal_set;
	siginfo_t sig;

	while(true)
	{
		// Wait for a signal and handle it
		sigfillset(&signal_set);
		sigwaitinfo(&signal_set, &sig);
		switch(sig.si_signo)
		{
			// Exit signals from OS
			case SIGQUIT:
				std::cout << "Received SIGQUIT signal." << std::endl;
				exitsignal = true;
				return;

			case SIGINT:
				// Only accept SIGINT from TTY
				if(sig.si_pid == 0)
				{
					std::cout << "Received SIGINT signal." << std::endl;
					exitsignal = true;
					return;
				}
				break;

			case SIGTERM:
				std::cout << "Received SIGTERM signal." << std::endl;
				exitsignal = true;
				return;

			case SIGHUP:
				std::cout << "Received SIGHUP signal." << std::endl;
				exitsignal = true;
				return;

			// This is raised by the destructor when this thread should stop
			case SIGUSR1:
				return;

			// We ignore all other signals
			default:
				break;
		}
	}
}

