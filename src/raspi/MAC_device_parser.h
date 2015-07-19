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


#ifndef MAC_DEVICE_PARSER_H_NA1TWB4Y
#define MAC_DEVICE_PARSER_H_NA1TWB4Y


#include <string>
#include <unordered_map>

// This class is used to parse a provided file to extract MAC addresses and associated device
// name pairs, provided linewise in the form:
// xx:xx:xx:xx:xx:xx	<dev_name>
// The parsed MAC addresses can be later used to identify Bluetooth devices to which a
// connection should be established to.
class MACDeviceParser
{
public:
	MACDeviceParser () {}

	// Construction with filename of file that will be parsed.
	MACDeviceParser (const std::string & filename) :
		filename_(filename) {}
	~MACDeviceParser (){}
	
	// Performs parsing of file filename_ for MAC addresses and associated device name pairs and
	// returns the total number matched pairs. Each found MAC address is saved as key in
	// map devices_ with its associated device name as value.
	int ParseForDevices();

	// access functions
	const std::string & filename() const { return filename_; }
	const std::unordered_map<std::string, std::string> & devices() const { return devices_; }

	const std::unordered_map<std::string, std::string> & file_paths() const { return file_paths_; }

	// mutators
	void set_filename(const std::string & filename) { filename_ = filename; }

	// erase all entries of devices_ map
	void clear_devices() { devices_.clear(); }


private:
	// filename of file that will be parsed
	std::string filename_;

	// Maps the parsed devices to the associated device names.
	std::unordered_map<std::string, std::string> devices_;
	std::unordered_map<std::string, std::string> file_paths_;

};


#endif /* end of include guard: MAC_DEVICE_PARSER_H_NA1TWB4Y */
