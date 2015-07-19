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

#include <future>
#include <chrono>
#include <iostream>
#include <memory>
#include <ratio>
#include <utility>
#include <vector>

#include <QString>
#include <QJsonArray>

#include "database_manager.h"
#include "MAC_device_parser.h"
#include "scheduler.h"
#include "../protocol_definitions/communication_structs.h"


template<int Granularity>
std::unique_ptr<rendezvous_answer> 
Scheduler<Granularity>::ScheduleNextCollectionStart(const QString device_id)
{
	std::unique_ptr<rendezvous_answer> result_ptr(new rendezvous_answer);

	const std::chrono::system_clock::time_point current_time = 
		std::chrono::system_clock::now();

	//const std::vector< std::pair<std::string,std::chrono::system_clock::time_point> > 
	const auto returned_vector = std::move(
			db_manager_ptr_->FetchCollectionStartTimes(current_time));
	std::cout << "Fetched " << returned_vector.size() << " collection start times" << std::endl;

	std::chrono::system_clock::time_point last_time_point;
	if(!returned_vector.empty())
		last_time_point = std::move(returned_vector.front().second);
	else
		last_time_point = current_time;

	std::chrono::minutes duration_mins_epoch =  
		std::chrono::duration_cast< std::chrono::minutes >(last_time_point.time_since_epoch());
	
	//std::cout << "duration length: " << duration_mins_epoch.count() << std::endl;
	duration_mins_epoch += std::chrono::minutes(Granularity) - (duration_mins_epoch % std::chrono::minutes(Granularity));
	//std::cout << "duration length after increase: " << duration_mins_epoch.count() << std::endl;

	const auto difference = std::chrono::duration_cast<std::chrono::seconds>(
			duration_mins_epoch - current_time.time_since_epoch());

	static const int guard_seconds = 10;
	// are we now too close to scheduled start time? -then add another interval
	if(difference.count() <= guard_seconds)
		duration_mins_epoch += std::chrono::minutes(Granularity);

	const auto scheduled_time = std::chrono::system_clock::time_point(duration_mins_epoch);

	//std::cout << "fetched time: " << last_time_point.time_since_epoch().count() << std::endl;
	//std::cout << "scheduled time: " << scheduled_time.time_since_epoch().count() << std::endl;
	db_manager_ptr_->ScheduledTimeToDatabase(device_id, scheduled_time);
	std::cout << "Pushed scheduled time " << scheduled_time.time_since_epoch().count() << " to database" << std::endl;
	db_manager_ptr_->TimeConvertToDeviceTime(scheduled_time, 
			&(result_ptr->collection_start_time));
	return std::move(result_ptr);
}

template class 	Scheduler<5>;
