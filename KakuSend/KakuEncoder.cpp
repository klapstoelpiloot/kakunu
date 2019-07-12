/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#include <assert.h>
#include "KakuEncoder.h"

// Constructor
KakuEncoder::KakuEncoder()
{
}

// Destructor
KakuEncoder::~KakuEncoder()
{
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

// Encodes bits to pulse durations
std::string KakuEncoder::Encode(std::string bits, std::vector<uint>& timesout)
{
	timesout.clear();

	// Every 'bit' in the code should be either 0, 1, 2 or 3.
	// Convert them to integers for easier parsing.
	std::vector<int> bitcodes;
	for(const char& c : bits)
	{
		int i = static_cast<int>(c);

		// Validate each bit code
		if((i < 48) || (i > 51))
			return "Unable to encode bitcode. Invalid bits.";

		bitcodes.push_back(i - 48);
	}

	// Add start pulses
	timesout.push_back(SHORT_US);
	timesout.push_back(EXTRALONG_US);

	// Make pulses for the bit codes
	for(int b : bitcodes)
	{
		switch(b)
		{
			case 0:
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(LONG_US);
				break;

			case 1:
				timesout.push_back(SHORT_US);
				timesout.push_back(LONG_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				break;

			case 2:
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(SHORT_US);
				break;

			case 3:
				timesout.push_back(SHORT_US);
				timesout.push_back(LONG_US);
				timesout.push_back(SHORT_US);
				timesout.push_back(LONG_US);
				break;

			default:
				assert(false);
		}
	}

	// Add end pulses
	timesout.push_back(SHORT_US);
	timesout.push_back(MEGALONG_US);

	// Done
	return std::string();
}

