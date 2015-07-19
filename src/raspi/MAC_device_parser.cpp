// BeeWarm - Freie Universität Berlin - AG Neurobiologie
//
// Copyright © 2015 Benjamin Aschenbrenner
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <iostream>

#include "MAC_device_parser.h"


int MACDeviceParser::ParseForDevices() 
{
	int device_counter = 0;

	// regular expression MAC identifier followed by device name separated by blanks
	std::regex regex(
			R"(([[:xdigit:]]{2}:[[:xdigit:]]{2}:[[:xdigit:]]{2}:[[:xdigit:]]{2}:[[:xdigit:]]{2}:[[:xdigit:]]{2})[[:blank:]]+([_/\w\\]+)[[:blank:]]+([/\w\\]+))");
	std::smatch smatch;
	std::string line;

	std::ifstream file(filename_);

	while(std::getline(file, line)) {
		if(std::regex_search(line, smatch, regex))
		{
			//std::cout << smatch[1] << " " << smatch[2] << " " << smatch[3] << std::endl;
			devices_[smatch[1]] = std::move(smatch[2]);
			file_paths_[smatch[1]] = std::move(smatch[3]);
			++device_counter;
		}
	}

	file.close();

	return device_counter;
}


