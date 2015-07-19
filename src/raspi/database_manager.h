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


#ifndef DATABASE_MANAGER_H_I0NKFFHI
#define DATABASE_MANAGER_H_I0NKFFHI

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include <QCoreApplication>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

#include "MAC_device_parser.h"
#include "../protocol_definitions/communication_structs.h"


class DatabaseManager : public QObject
{
	Q_OBJECT
public:
	DatabaseManager (
			const std::string & database_name,
			const std::string & db_user,
			const std::string & db_password,
			const std::string & db_url_host,
			const std::string & db_query_path,
			const std::string & db_write_path,
			const int db_port,
			const MACDeviceParser & parser) : 
		db_name_(database_name),
		db_user_(db_user),
		db_password_(db_password),
		db_url_host_(db_url_host),
		db_query_path_(db_query_path),
		db_write_path_(db_write_path),
		db_port_(db_port),
		parser_(parser),
		open_network_replies_(0)
	{
	}

public slots:
	void Init(const QCoreApplication * qapp);

	void PushValuesToDatabase(const QString device_id, 
			std::shared_ptr<temperature_readings_header> temperature_readings_header,
			std::shared_ptr<std::vector<temperature_reading>> collected_data);

	// Handle test output of timestamp and temperatures
	void HandleTestData(const QString device_id,
			std::shared_ptr<timestamp> device_time, 
			std::shared_ptr<temperature_reading> temperatures);

	// Fetch all collection start times after given query_timestamp
	const std::vector< std::pair<std::string, std::chrono::system_clock::time_point> >
		FetchCollectionStartTimes(const std::chrono::system_clock::time_point time_point);

	void ScheduledTimeToDatabase(const QString device_id, 
			const std::chrono::system_clock::time_point time_point);

	// Pushes an error event to database.
	void PushErrorEvent(const QString device_id, 
			const std::chrono::system_clock::time_point time_point,
			const int error_value);

	// Pushes event to database in case a node requested to be initialized.
	void PushInitEvent(const QString device_id, 
			const std::chrono::system_clock::time_point time_point);

	// Pushes event to database in case a node dumped data.
	void PushRendezvousEvent(const QString device_id,
			const std::chrono::system_clock::time_point time_point);

	// Pushes event to database in case a node requested to be initialized.
	void PushTimeRequestEvent(const QString device_id, 
			const std::chrono::system_clock::time_point time_point);

	void PostReplyFinishedSlot(QNetworkReply * reply);
	void ReplyFinishedSlot();
	void ErrorReplySlot(QNetworkReply::NetworkError error_code);
signals:
	void AllFinished();

public:
	int open_network_replies() {return open_network_replies_;};
	// Definitions for time conversion from BCD of DS3231 RTC style into RFC3339 style
	static void TimeConvertToDeviceTime(const std::chrono::system_clock::time_point &time_point,
			timestamp *timestamp_struct);
	static void TimeConvertToHostTime(const timestamp &timestamp_struct, 
			std::chrono::system_clock::time_point * system_time);
	static void TemperatureReadingToValues(const temperature_reading & temperatures,
			std::tuple<double, double, double, double> *converted_values);

protected:
	QJsonDocument CreateDatabaseEventJson(
			const QString & device_id, 
			const QString & series_name,
			const QString & event_type,
			const int value,
			const std::chrono::system_clock::time_point & time_point);

	void JsonToDatabase(const QJsonDocument & json_doc);

	void JsonToDatabaseNAM(const QJsonDocument & json_doc);
protected:
	// JSON static definitions
	static const char *database_key;
	static const char *retention_policy_key;
	static const char *retention_policy_value;
	static const char *device_id_key;
	static const char *device_tag_key;
	static const char *tags_key;
	static const char *points_key;
	static const char *name_key;
	static const char *name_value;
	static const char *fields_key;
	static const char *value_key;
	static const char *values_key;
	static const char *time_key;
	static const char *precision_key;
	static const char *precision_value;
	static const char *sensor_key;
	static const char *sensor_id_1_value;
	static const char *sensor_id_2_value;
	static const char *sensor_id_3_value;
	static const char *sensor_id_4_value;
	static const char *type_key;
	static const char *type_collection_start;
	static const char *collection_events;
	static const char *error_events;
	static const char *type_error_event;
	static const char *events;
	static const char *type_event_init; 
	static const char *type_event_rendezvous; 
	static const char *type_event_time; 

	// Time conversion definitions
	// seconds
	static const unsigned char MASK_DECIMAL_1_SEC = 0x0f;
	static const unsigned char SHIFT_DECIMAL_10_SEC = 4;
	static const unsigned char MASK_DECIMAL_10_SEC = 0x70;
	// minutes
	static const unsigned char MASK_DECIMAL_1_MIN = 0x0f;
	static const unsigned char SHIFT_DECIMAL_10_MIN = 4;
	static const unsigned char MASK_DECIMAL_10_MIN = 0x70;
	// hours
	static const unsigned char MASK_DECIMAL_1_HOUR = 0x0f;
	static const unsigned char MASK_DECIMAL_10_HOUR = 0x10;
	static const unsigned char SHIFT_DECIMAL_10_HOUR = 4;
	static const unsigned char MASK_DECIMAL_20_HOUR = 0x20;
	static const unsigned char SHIFT_DECIMAL_20_HOUR = 5;
	static const unsigned char MASK_DECIMAL_24_HOUR = 0x40;
	// day of month
	static const unsigned char MASK_DECIMAL_1_DATE = 0x0f;
	static const unsigned char SHIFT_DECIMAL_10_DATE = 4;
	static const unsigned char MASK_DECIMAL_10_DATE = 0x30;
	// month
	static const unsigned char MASK_DECIMAL_1_MONTH = 0x0f;
	static const unsigned char SHIFT_DECIMAL_10_MONTH = 4;
	static const unsigned char MASK_DECIMAL_10_MONTH = 0x10;
	// year
	static const unsigned char MASK_DECIMAL_1_YEAR = 0x0f;
	static const unsigned char SHIFT_DECIMAL_10_YEAR = 4;
	static const unsigned char MASK_DECIMAL_10_YEAR = 0xf0;

private:
	const std::string db_name_;
	const std::string db_user_;
	const std::string db_password_;
	const std::string db_url_host_;
	const std::string db_query_path_;
	const std::string db_write_path_;
	const int db_port_;
	const MACDeviceParser & parser_;
	int open_network_replies_;
	std::unique_ptr<QNetworkAccessManager> nam_;
};


#endif /* end of include guard: DATABASE_MANAGER_H_I0NKFFHI */

