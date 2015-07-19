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

#include <iostream>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <QCoreApplication>
#include <QObject>
#include <QThread>

#include "bluetooth_manager.h"
#include "database_manager.h"
#include "MAC_device_parser.h"
#include "scheduler.h"
#include "../protocol_definitions/communication_structs.h"


int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		std::cout << "no devices found - nothing to do - quit" << std::endl;
		return EXIT_SUCCESS;
	}

	static const std::string filename("devices_mapping.txt");

	QCoreApplication app(argc, argv);

	MACDeviceParser parser(filename);

	DatabaseManager db_manager("mydb", 
			"test_user", "passwd_1234", 
			"localhost", "/query", "/write", 8086, 
			parser);

	Scheduler<5> scheduler(&db_manager,
			&parser);

	BluetoothManager bt_manager(&parser, &scheduler, &db_manager);

	db_manager.Init(&app);

	// TESTS
	//parser.ParseForDevices();
	//std::cout << "number of found known devices by parser: " << 
	//	parser.devices().size() << std::endl;
	//
	//static const QString device_test_id("3f:13:a5:2b:13:1c");

	//static const std::chrono::system_clock::time_point 
	//	current_time = std::chrono::system_clock::now();
	//struct timestamp ts;
	//std::chrono::system_clock::time_point converted_time;
	//db_manager.TimeConvertToDeviceTime(current_time, &ts);

	//db_manager.TimeConvertToHostTime(ts, &converted_time);

	//std::time_t current_c_time = std::chrono::system_clock::to_time_t(current_time);
	//std::time_t converted_c_time = std::chrono::system_clock::to_time_t(converted_time);
	//std::cout << std::ctime(&current_c_time) << std::endl;
	//std::cout << std::ctime(&converted_c_time) << std::endl;

	//std::shared_ptr<temperature_readings_header> temperatures_header_ptr(
	//		new temperature_readings_header);

	//static const unsigned int NUMBER_OF_TEST_SAMPLES = 100;
	//temperatures_header_ptr->number_of_readings = NUMBER_OF_TEST_SAMPLES;
	//db_manager.TimeConvertToDeviceTime(current_time, &(temperatures_header_ptr->start_time));
	//temperatures_header_ptr->interval_length_seconds = 60 * 5;

	//std::shared_ptr<std::vector<temperature_reading>>test_data(new std::vector<temperature_reading>);
	//test_data->reserve(100);
	//for (unsigned int i = 0; i < NUMBER_OF_TEST_SAMPLES; ++i) {
	//	const unsigned int temp_1 = i;
	//	const unsigned int temp_2 = i+1;
	//	const unsigned int temp_3 = i+2;
	//	const unsigned int temp_4 = i+3;

	//	test_data->emplace_back(temperature_reading({{(unsigned char) temp_1, 
	//				(unsigned char) temp_2, 
	//				(unsigned char) temp_3, 
	//				(unsigned char) temp_4}}));
	//}

	//std::thread t(&DatabaseManager::PushValuesToDatabase, &db_manager,
	//			device_test_id, 
	//			std::move(temperatures_header_ptr),
	//			std::move(test_data));

	//db_manager.PushValuesToDatabase(device_test_id,
	//		std::move(temperatures_header_ptr),
	//		std::move(test_data));

	//db_manager.ScheduledTimeToDatabase(device_test_id,
	//		current_time);

	//db_manager.FetchCollectionStartTimes(current_time);
	//scheduler.ScheduleNextCollectionStart(device_test_id);


	if( bt_manager.CheckLocalBluetoothDevice() )
	{
		std::cout << "Local Bluetooth device is available" << std::endl;
		bt_manager.Init(&app, argc, argv);
	}
	else{
		std::cout << "No Local Bluetooth device available" << std::endl;
		return EXIT_FAILURE;
	}

	if(db_manager.open_network_replies())
		app.exec();

	return EXIT_SUCCESS;
}

