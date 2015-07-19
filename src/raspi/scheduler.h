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



#ifndef SCHEDULER_H_ZETRFAXG
#define SCHEDULER_H_ZETRFAXG

#include <memory>

#include <QString>

#include "database_manager.h"
#include "MAC_device_parser.h"
#include "../protocol_definitions/communication_structs.h"

template< int Granularity = 5>
class Scheduler
{
public:
	Scheduler (DatabaseManager * db_manager_ptr,
			MACDeviceParser * MAC_parser_ptr) :
		db_manager_ptr_(db_manager_ptr),
		MAC_parser_ptr_(MAC_parser_ptr)
	{}

	~Scheduler () {}

	// Schedule next collection start for the given device
	std::unique_ptr<rendezvous_answer> ScheduleNextCollectionStart(const QString device_id);

private:
	DatabaseManager *db_manager_ptr_;
	MACDeviceParser *MAC_parser_ptr_;

	// The hour is devided into minute blocks of size Granularity
	static const unsigned int granularity_min_ = Granularity;
};

#endif /* end of include guard: SCHEDULER_H_ZETRFAXG */
