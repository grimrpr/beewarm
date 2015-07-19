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



#ifndef SERIAL_COMMUNICATION_H_RCSZ7HS1
#define SERIAL_COMMUNICATION_H_RCSZ7HS1

#include <QObject>
#include <QIODevice>
#include <QMetaType>

#include "database_manager.h"
#include "scheduler.h"
#include "../protocol_definitions/communication_structs.h"

static const char OKAY_MSG_STR[] = {0};
static const char INIT_MSG_STR[] = {1};
static const char DATA_MSG_STR[] = {2};
static const char DUMP_MSG_STR[] = {3};
static const char TIME_MSG_STR[] = {4};
static const char TEST_MSG_STR[] = {5};
static const char FINI_MSG_STR[] = {6};

static const int TIMEOUT_MS = 8000;
static const unsigned int MAX_COMMAND_LENGTH = 5;
static const unsigned int MAX_REQUESTS = 4;

Q_DECLARE_METATYPE(std::chrono::system_clock::time_point);
Q_DECLARE_METATYPE(temperature_readings_header);

class SerialCommunicator : public QObject
{
	Q_OBJECT
public:
	SerialCommunicator (DatabaseManager *db_manager_ptr, Scheduler<5> *scheduler):
		db_manager_ptr_(db_manager_ptr),
		scheduler_(scheduler)
	{
		qRegisterMetaType<std::chrono::system_clock::time_point>();
		qRegisterMetaType<temperature_readings_header>();
		connect(this, SIGNAL(PushValuesToDB(const QString, 
						std::shared_ptr<temperature_readings_header>, 
						std::shared_ptr<std::vector<temperature_reading>>)), 
				db_manager_ptr_, SLOT(PushValuesToDatabase(const QString, 
						std::shared_ptr<temperature_readings_header>, 
						std::shared_ptr<std::vector<temperature_reading>>)));

		connect(this, SIGNAL(TimeEvent(const QString, std::chrono::system_clock::time_point)),
				db_manager_ptr_, SLOT(PushTimeRequestEvent(const QString, 
						std::chrono::system_clock::time_point)));
	}

	~SerialCommunicator () {}

	// Takes a connected socket and handles communication with the other end.
	// If a data dump is received it is passed to the database manager.
	void PerformCommunication(QIODevice * bt_socket_ptr, const QString & peer_name);
private:

	// Receives from socket N bytes and timeouts for each received byte after timeout in ms.
	bool ReceiveNChars(char * receive_buffer, QIODevice * socket_ptr, const int timeout_ms, const long N);
	bool ReceiveAndStoreTemperatures(QIODevice * socket_ptr, const QString & peer_name, const int timeout_ms = TIMEOUT_MS);

public slots:

signals:
	void PushValuesToDB(const QString device_id, std::shared_ptr<temperature_readings_header> temp_header,
			std::shared_ptr<std::vector<temperature_reading>> collected_data);
	void PushTestToDBManager(const QString device_id, std::shared_ptr<timestamp> stamp,
			std::shared_ptr<temperature_reading> collected_data);
	void TimeEvent(const QString, const std::chrono::system_clock::time_point time_point);
	void RendezvousEvent(const QString, const std::chrono::system_clock::time_point time_point);
	void ErrorEvent(const QString, const std::chrono::system_clock::time_point time_point, const int err_val);

private:
	DatabaseManager * db_manager_ptr_;
	Scheduler<5> * scheduler_;

public:

};

#endif /* end of include guard: SERIAL_COMMUNICATION_H_RCSZ7HS1 */
