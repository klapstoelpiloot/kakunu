/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <iostream>
#include "InputHandler.h"

// Constructor
InputHandler::InputHandler(bool _exitonenter) :
	exitonenter(_exitonenter),
	exitsignal(false),
	thread(nullptr)
{
	// Create a separate thread to handle input
	thread = new std::thread(&InputHandler::Thread, this);
}

// Destructor
InputHandler::~InputHandler()
{
	// Clean up
	thread->detach();
	delete thread;
	thread = nullptr;
}

// This is the thread that waits for a signal and handles it.
void InputHandler::Thread()
{
	while(true)
	{
		if(exitonenter)
		{
			// Check if ENTER is pressed
			if(std::cin.get() == '\n')
			{
				exitsignal = true;
				return;
			}
		}
		else
		{
			// Ignore all input
			std::cin.ignore();
		}
	}
}

