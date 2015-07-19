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


#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#include <QIODevice>
//#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "database_manager.h"
#include "serial_communication.h"
#include "../protocol_definitions/communication_structs.h"


void SerialCommunicator::PerformCommunication(QIODevice * bt_socket_ptr, const QString & peer_name)
{
	unsigned int request_number = 0;
	bool socket_no_error = true;
	enum message_enum { OKAY_MSG = 0,  INIT_MSG = 1, DATA_MSG = 2, DUMP_MSG = 3, TIME_MSG = 4, TEST_MSG = 5, FINI_MSG = 6 };

	while(request_number < MAX_REQUESTS && socket_no_error)
	{
		//char command_str[MAX_COMMAND_LENGTH+1] = {'\0'};
		char command_str[1] = {0};

		// Beginning of communication first request
		if(request_number == 0)
		{
			bt_socket_ptr->putChar(OKAY_MSG);
		}

		// wait for request response upto TIMEOUT_MS
		//socket_no_error = ReceiveNChars(command_str, bt_socket_ptr,
		//		TIMEOUT_MS, MAX_COMMAND_LENGTH);
		socket_no_error = ReceiveNChars(command_str, bt_socket_ptr,
				TIMEOUT_MS, 1);


		// check for CASE A - slave has sent 'INIT' 
		//if(!strncmp(INIT_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		if(command_str[0] == INIT_MSG)
		{
			//DEBUG
			std::cout << "INIT requested" << std::endl;
			// start scheduling handling for the requesting device
			std::future<std::unique_ptr<rendezvous_answer>> future_schedule =
				std::async(
					std::launch::async,
					&Scheduler<5>::ScheduleNextCollectionStart, 
					scheduler_, 
					peer_name);

			bt_socket_ptr->putChar(OKAY_MSG);

			const auto answer_unique_ptr( std::move(future_schedule.get()) );
			//TODO change to 5 * 60 seconds value after DEBUG
			answer_unique_ptr->interval_length_seconds = 300;
			bt_socket_ptr->write((char *) answer_unique_ptr.get(), 
					sizeof(rendezvous_answer));

			struct timestamp stamp;
			DatabaseManager::TimeConvertToDeviceTime(std::chrono::system_clock::now(),
					&stamp);
			bt_socket_ptr->write((char *) &stamp, sizeof(timestamp));
		}

		// check for CASE B1 - slave has sent 'DATA'
		//else if(!strncmp(DATA_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		else if(command_str[0] == DATA_MSG)
		{
			std::cout << "DATA requested" << std::endl;
			std::future<std::unique_ptr<rendezvous_answer>> future_schedule =
				std::async(
					std::launch::async,
					&Scheduler<5>::ScheduleNextCollectionStart, 
					scheduler_, 
					peer_name);

			bt_socket_ptr->putChar(OKAY_MSG);

			//unsigned int counter = 0;
			//while(counter < 36)
			//{
			//	char c = 0xFF;
			//	bt_socket_ptr->waitForReadyRead(1000);
			//	bt_socket_ptr->getChar(&c);
			//	std::cout << "counter:" << counter <<" value: " << (unsigned int) c << std::endl;
			//	++counter;
			//}

			ReceiveAndStoreTemperatures(bt_socket_ptr, peer_name);

			auto answer_unique_ptr( move(future_schedule.get()) );
			//TODO change to 5 * 60 seconds value after DEBUG
			answer_unique_ptr->interval_length_seconds = 300;
			bt_socket_ptr->write((char *) answer_unique_ptr.get(), 
					sizeof(rendezvous_answer));
			bt_socket_ptr->waitForBytesWritten(500);
			break;
		}
		// check for CASE B2 - slave has sent 'DUMP'
		//else if(!strncmp(DUMP_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		else if(command_str[0] == DUMP_MSG)
		{
			std::cout << "DUMP requested" << std::endl;
			bt_socket_ptr->putChar(OKAY_MSG);

			ReceiveAndStoreTemperatures(bt_socket_ptr, peer_name);

			//std::thread thread = std::thread(&DatabaseManager::PushRendezvousEvent,
			//		db_manager_ptr_, peer_name, 
			//		std::chrono::system_clock::now());
			//thread.detach();
			emit RendezvousEvent(peer_name, std::move(std::chrono::system_clock::now()));
			break;
		}
		// check for CASE C - slave has sent 'TIME'
		//else if(!strncmp(TIME_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		else if(command_str[0] == TIME_MSG)
		{
			std::cout << "TIME requested" << std::endl;
			bt_socket_ptr->putChar(OKAY_MSG);

			struct timestamp stamp;
			DatabaseManager::TimeConvertToDeviceTime(std::chrono::system_clock::now(),
					&stamp);
			bt_socket_ptr->write((char *) &stamp, sizeof(timestamp));
			emit TimeEvent(peer_name, std::move(std::chrono::system_clock::now()));
		}
		// check for CASE D - slave has sent 'TEST'
		//else if(!strncmp(TEST_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		else if(command_str[0] == TEST_MSG)
		{
			std::cout << "TEST requested" << std::endl;
			std::shared_ptr<struct timestamp> stamp_ptr(new timestamp);
			std::shared_ptr<struct temperature_reading> temperatures_ptr(new temperature_reading);

			ReceiveNChars((char *) stamp_ptr.get(), bt_socket_ptr, TIMEOUT_MS, sizeof(timestamp));
			ReceiveNChars((char *) temperatures_ptr.get(), bt_socket_ptr, TIMEOUT_MS, sizeof(temperature_reading));

			//auto db_manager_thread = std::thread(&DatabaseManager::HandleTestData, db_manager_ptr_,
			//		peer_name,
			//		std::move(stamp_ptr), std::move(temperatures_ptr));
			//db_manager_thread.detach();
			emit PushTestToDBManager(peer_name, stamp_ptr, temperatures_ptr);
		}
		//else if(!strncmp(FINI_MSG_STR, command_str, MAX_COMMAND_LENGTH))
		else if(command_str[0] == FINI_MSG)
		{
			std::cout << "FINI received" << std::endl;
			break;
		}
		// request command string not recognized - error CASE
		else
		{
			std::cout << "Error while parsing command" << std::endl;
			emit ErrorEvent(peer_name, std::chrono::system_clock::now(), 0);
			// TODO refine error handling in case the received command is not recognized 
			socket_no_error = false;
		}

		++request_number;
	}

	// shutdown communication
	// bt_socket_ptr->disconnectFromService();
	std::cout << "leaving serial handler" << std::endl;
	bt_socket_ptr->close();

}


bool SerialCommunicator::ReceiveNChars(char * receive_buffer, 
		QIODevice * socket_ptr, const int timeout_ms, const long N)
{
	std::cout << "in receive N chars" << std::endl;
	bool socket_no_error = true;
	long num_received = 0;

	// wait for request response upto TIMEOUT_MS
	for (unsigned int i = 0; i < N; ++i) {
		socket_no_error = socket_ptr->waitForReadyRead(timeout_ms);
		//if(socket_no_error)
		if(socket_no_error)
		{
			socket_ptr->getChar(receive_buffer + i);
			++num_received;
			//std::cout << "successfully read character: " << (unsigned char) receive_buffer[i] << std::endl;
		}
		else
		{
			std::cout << socket_ptr->errorString().toStdString() << std::endl;
		}
	}

	std::cout << "expected bytes: " << N << std::endl <<
		"received bytes: " << num_received << std::endl;
	//if(!socket_no_error)
	//	std::cout << socket_ptr->errorString().toStdString() << std::endl;

	return socket_no_error;
}


bool SerialCommunicator::ReceiveAndStoreTemperatures(QIODevice * socket_ptr, const QString & peer_name, const int timeout_ms)
{		
	// TODO add error handling if receiving failed
	// receive data header
	std::cout << "receive temperatures header" << std::endl;
	std::shared_ptr<struct temperature_readings_header> temperature_hdr_ptr(new temperature_readings_header);
	bool socket_no_error = ReceiveNChars( (char *) temperature_hdr_ptr.get(),
			socket_ptr, timeout_ms, sizeof(temperature_readings_header));

	const unsigned int number_of_readings = temperature_hdr_ptr->number_of_readings;

	std::cout << "receive temperature data" << std::endl;

	std::shared_ptr<std::vector<temperature_reading>> 
		collected_data(new std::vector<temperature_reading>(number_of_readings));
	// receive data
	socket_no_error = ReceiveNChars( (char *) collected_data->data(), 
			socket_ptr, timeout_ms, sizeof(temperature_reading) * number_of_readings);

	emit PushValuesToDB(peer_name, 
			std::move(temperature_hdr_ptr),
			std::move(collected_data));

	return socket_no_error;
}

