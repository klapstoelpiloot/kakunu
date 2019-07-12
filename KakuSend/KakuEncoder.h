/*
	Copyright (c) 2019 Pascal van der Heiden, www.codeimp.com.
	This software is released under MIT license.
*/
#pragma once
#include <vector>
#include <string>
#include "../KakuNu/Tools.h"

class KakuEncoder
{
private:

	// Constants
	const uint SHORT_US = 250;
	const uint LONG_US = SHORT_US * 5;
	const uint EXTRALONG_US = SHORT_US * 10;
	const uint MEGALONG_US = SHORT_US * 40;

public:

	// Constructor / destructor
	KakuEncoder();
	virtual ~KakuEncoder();

	// Encodes bits to pulse durations
	std::string Encode(std::string bits, std::vector<uint>& timesout);
};
