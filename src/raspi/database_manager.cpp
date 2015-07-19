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

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <ratio>
#include <tuple>
#include <vector>

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include "database_manager.h" 
#include "../protocol_definitions/communication_structs.h"

// character string definitions
const char *DatabaseManager::database_key = "database";
const char *DatabaseManager::retention_policy_key = "retentionPolicy";
const char *DatabaseManager::retention_policy_value = "myret";
const char *DatabaseManager::device_id_key = "device_id";
const char *DatabaseManager::device_tag_key = "device_tag";
const char *DatabaseManager::tags_key = "tags";
const char *DatabaseManager::points_key = "points";
const char *DatabaseManager::name_key = "measurement";
const char *DatabaseManager::name_value = "temperature_C";
const char *DatabaseManager::fields_key = "fields";
const char *DatabaseManager::value_key = "value";
const char *DatabaseManager::values_key = "values";
const char *DatabaseManager::time_key = "time";
const char *DatabaseManager::precision_key = "precision";
const char *DatabaseManager::precision_value = "s";
const char *DatabaseManager::sensor_key = "sensor_id";
const char *DatabaseManager::sensor_id_1_value = "sensor_1";
const char *DatabaseManager::sensor_id_2_value = "sensor_2";
const char *DatabaseManager::sensor_id_3_value = "sensor_3";
const char *DatabaseManager::sensor_id_4_value = "sensor_4";
const char *DatabaseManager::type_key = "type";
const char *DatabaseManager::type_collection_start = "start";
const char *DatabaseManager::collection_events = "collection_events";
const char *DatabaseManager::error_events = "error_events";
const char *DatabaseManager::type_error_event = "error";
const char *DatabaseManager::events = "events";
const char *DatabaseManager::type_event_init = "init";
const char *DatabaseManager::type_event_rendezvous = "rendezvous";
const char *DatabaseManager::type_event_time = "time";


void DatabaseManager::Init(const QCoreApplication * qapp)
{
	connect(this, SIGNAL(AllFinished()), qapp, SLOT(quit()));
	nam_ = std::unique_ptr<QNetworkAccessManager>(new QNetworkAccessManager); 
}

void DatabaseManager::PushValuesToDatabase(const QString device_id, 
		std::shared_ptr<temperature_readings_header> temperatures_header_ptr,
		std::shared_ptr<std::vector<temperature_reading>> collected_data)
{
	
	// get additional tag for database entry
	const std::unordered_map<std::string, std::string>::const_iterator device_id_tag = 
		parser_.devices().find(device_id.toLower().toStdString());

	// data collection begin timestamp 
	std::chrono::system_clock::time_point sample_time;
	TimeConvertToHostTime(temperatures_header_ptr->start_time, &sample_time);

	// sampling interval setup
	const std::chrono::duration<unsigned int, std::ratio<1,1> > 
		interval_seconds(temperatures_header_ptr->interval_length_seconds);

	const QString database_value(db_name_.c_str());

	// iterate over all datapoints, create JSON objects and send them to the database
	for (const auto & temperature_data : *collected_data) {

		QJsonObject json_write_object;
		QJsonObject json_common_tag_object;
		QJsonArray json_points_array;

		// create highest level
		json_write_object.insert(database_key, database_value);
		json_write_object.insert(retention_policy_key, retention_policy_value);
		json_common_tag_object.insert(device_id_key, device_id);

		if ( device_id_tag != parser_.devices().end() )
			json_common_tag_object.insert(
					device_tag_key, device_id_tag->second.c_str());

		json_write_object.insert(tags_key, json_common_tag_object);
		json_write_object.insert(time_key, qint64(
					std::chrono::duration_cast<std::chrono::seconds>
					(sample_time.time_since_epoch()).count())
				);

		json_write_object.insert(precision_key, precision_value);


		QJsonObject json_point_1, json_point_2, 
			    json_point_3, json_point_4, 
			    json_field_1, json_field_2, 
			    json_field_3, json_field_4;

		QJsonObject json_tags_object_1, json_tags_object_2,
			    json_tags_object_3, json_tags_object_4;

		QJsonObject json_sensor_tag_1, json_sensor_tag_2, 
			    json_sensor_tag_3, json_sensor_tag_4;

		std::tuple<double, double, double, double> converted_temperature_values;
		TemperatureReadingToValues(temperature_data, &converted_temperature_values);

		json_point_1.insert(name_key, name_value);
		json_tags_object_1.insert(sensor_key, sensor_id_1_value);
		json_point_1.insert(tags_key, json_tags_object_1);
		json_field_1.insert(value_key, 
				std::move(std::get<0>(converted_temperature_values)));

		json_point_1.insert(fields_key, json_field_1);
		json_points_array.append(json_point_1);

		json_point_2.insert(name_key, name_value);
		json_tags_object_2.insert(sensor_key, sensor_id_2_value);
		json_point_2.insert(tags_key, json_tags_object_2);
		json_field_2.insert(value_key, 
				std::move(std::get<1>(converted_temperature_values)));

		json_point_2.insert(fields_key, json_field_2);
		json_points_array.append(json_point_2);

		json_point_3.insert(name_key, name_value);
		json_tags_object_3.insert(sensor_key, sensor_id_3_value);
		json_point_3.insert(tags_key, json_tags_object_3);
		json_field_3.insert(value_key, 
				std::move(std::get<2>(converted_temperature_values)));

		json_point_3.insert(fields_key, json_field_3);
		json_points_array.append(json_point_3);

		json_point_4.insert(name_key, name_value);
		json_tags_object_4.insert(sensor_key, sensor_id_4_value);
		json_point_4.insert(tags_key, json_tags_object_4);
		json_field_4.insert(value_key, 
				std::move(std::get<3>(converted_temperature_values)));

		json_point_4.insert(fields_key, json_field_4);
		json_points_array.append(json_point_4);

		json_write_object.insert(points_key, json_points_array);
		QJsonDocument json_document(json_write_object);
		//cout << json_document.toJson().toStdString() << std::endl;

		sample_time += interval_seconds;

		JsonToDatabase(json_document);
	}
}

void DatabaseManager::HandleTestData(const QString device_id,
		std::shared_ptr<timestamp> device_time, 
		std::shared_ptr<temperature_reading> temperatures)
{
	std::chrono::system_clock::time_point time_point;
	TimeConvertToHostTime(*device_time, &time_point);

	std::tuple<double, double, double, double> temperature_values;
	TemperatureReadingToValues(*temperatures, &temperature_values);

	const std::unordered_map<std::string, std::string>::const_iterator device_id_tag = 
		parser_.devices().find(device_id.toLower().toStdString());

	std::cout << "device id: " << device_id.toStdString() <<  std::endl <<
		"device tag: " << device_id_tag->second << std::endl <<
		"temperature 1: " << std::get<0>(temperature_values) << std::endl << 
		"temperature 2: " << std::get<1>(temperature_values) << std::endl << 
		"temperature 3: " << std::get<2>(temperature_values) << std::endl <<
		"temperature 4: " << std::get<3>(temperature_values) << std::endl << 
		std::endl;

}

const std::vector< std::pair<std::string, std::chrono::system_clock::time_point> >
		DatabaseManager::FetchCollectionStartTimes
		(const std::chrono::system_clock::time_point time_point)
{
	QUrl query_url;
	query_url.setScheme("http");
	query_url.setHost(db_url_host_.c_str());
	query_url.setPath(db_query_path_.c_str());
	query_url.setPort(db_port_);

	QUrlQuery url_query_part;
	url_query_part.addQueryItem("db", db_name_.c_str());

	url_query_part.addQueryItem("q", "SELECT "+QUrl::toPercentEncoding("*")+
			" FROM collection_events WHERE time > now"+
			QUrl::toPercentEncoding("()")+
			"and time < now"+QUrl::toPercentEncoding("()")+
			" " +QUrl::toPercentEncoding("+") + " 12h");

	query_url.setQuery(url_query_part);
	//std::cout << query_url.toEncoded(QUrl::FullyEncoded).toStdString() << std::endl;

	QNetworkAccessManager local_NAM;

	QNetworkReply *reply = local_NAM.get(QNetworkRequest(query_url));
	QEventLoop event_loop;

	connect(reply, SIGNAL(finished()), &event_loop, SLOT(quit()) );
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, 
			SLOT(ErrorReplySlot(QNetworkReply::NetworkError)));

	// loop until reply finished signal arrives
	event_loop.exec();

	QJsonDocument response_document = QJsonDocument::fromJson(reply->readAll());

	std::cout << "response:\n" << 
		response_document.toJson().toStdString() << std::endl;
	
	//const auto response =  response_document.object().constFind("error");

	std::vector< std::pair<std::string, 
		std::chrono::system_clock::time_point> > result;

	// error message?
	//if(!response->isNull())
	//{
	//	std::cout << "got error response" << std::endl;
	//	return result;
	//}

	const QJsonArray results_json_array = response_document.object().constFind("results")->toArray();
	const QJsonArray series_json_array = results_json_array.first().toObject().constFind("series")->toArray();

	for (const auto & json_value : series_json_array) {
		const QString device_id_parsed(
				json_value.toObject().constFind(tags_key)->toObject().constFind(device_id_key)->toString());

		if(json_value.toObject().constFind(values_key) != json_value.toObject().constEnd())
		{
			const QString timestamp_parsed(
					json_value.toObject().constFind(values_key)->toArray().last().toArray().first().toString());

			const auto date_time = QDateTime::fromString(timestamp_parsed, Qt::DateFormat::ISODate);
			const std::time_t date_time_unix = date_time.toTime_t();
			const std::chrono::system_clock::time_point parsed_time_point = std::chrono::system_clock::from_time_t(date_time_unix);

			result.emplace_back(device_id_parsed.toStdString(), parsed_time_point);

			std::cout << "device_id: " << device_id_parsed.toStdString() << " "
				"timestamp: " << timestamp_parsed.toStdString() << std::endl;
			const auto time_seconds = std::chrono::duration_cast<std::chrono::seconds>(parsed_time_point.time_since_epoch()).count();
			std::cout << "timestamp converted to duration: " <<  time_seconds << "s" << std::endl;
		}
	}

	// sort result entries according to timestamps
	std::sort(result.begin(), result.end(), [](const std::pair<std::string, std::chrono::system_clock::time_point> & pair_a, 
				const std::pair<std::string, std::chrono::system_clock::time_point> &pair_b)
			{return pair_a.second < pair_b.second;});

	for (const auto & result_pair : result) {
		const auto time_seconds = std::chrono::duration_cast<std::chrono::seconds>(result_pair.second.time_since_epoch()).count();
		std::cout << "device_id: " << result_pair.first << " time point: " << time_seconds << std::endl;
	}

	delete(reply);

	return result;
}

QJsonDocument DatabaseManager::CreateDatabaseEventJson(
		const QString & device_id, 
		const QString & series_name,
		const QString & event_type,
		const int value,
		const std::chrono::system_clock::time_point & time_point)
{
	// create JSON object with schedule information
	QJsonObject json_object;

	json_object.insert(database_key, db_name_.c_str());
	json_object.insert(retention_policy_key, retention_policy_value);
	json_object.insert(precision_key, precision_value);

	QJsonObject json_tag_object;
	json_tag_object.insert(device_id_key, device_id);
	json_tag_object.insert(type_key, event_type);
	json_object.insert(tags_key, json_tag_object);

	json_object.insert(time_key, qint64(
				std::chrono::duration_cast<std::chrono::seconds>
				(time_point.time_since_epoch()).count()));

	QJsonArray json_value_array;
	QJsonObject json_event_object;
	json_event_object.insert(name_key, series_name);

	QJsonObject json_fields_object;
	json_fields_object.insert(value_key, value);
	json_event_object.insert(fields_key, json_fields_object);

	json_value_array.push_back(json_event_object);
	json_object.insert(points_key, json_value_array);

	return QJsonDocument(json_object);


}

void DatabaseManager::JsonToDatabaseNAM(const QJsonDocument & json_doc)
{
	QUrl write_db_URL;
	write_db_URL.setScheme("http");
	write_db_URL.setHost(db_url_host_.c_str());
	write_db_URL.setPort(db_port_);
	write_db_URL.setPath(db_write_path_.c_str());

	QNetworkRequest database_request(write_db_URL);
	database_request.setHeader(QNetworkRequest::ContentTypeHeader, 
			QVariant("application/json"));

	QNetworkAccessManager local_NAM;
	QNetworkReply* reply = local_NAM.post(
			database_request, json_doc.toJson());
	QEventLoop event_loop;
	connect(reply, SIGNAL(finished()), &event_loop, SLOT(quit()) );
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
			this, 
			SLOT(ErrorReplySlot(QNetworkReply::NetworkError)));

	// loop until reply finished signal arrives
	event_loop.exec();

	delete(reply);
}

void DatabaseManager::JsonToDatabase(const QJsonDocument & json_doc)
{
	QUrl write_db_URL;
	write_db_URL.setScheme("http");
	write_db_URL.setHost(db_url_host_.c_str());
	write_db_URL.setPort(db_port_);
	write_db_URL.setPath(db_write_path_.c_str());

	QNetworkRequest database_request(write_db_URL);
	database_request.setHeader(QNetworkRequest::ContentTypeHeader, 
			QVariant("application/json"));

	++open_network_replies_;
	QNetworkReply* reply = nam_->post(
			database_request, json_doc.toJson());
	connect(reply, SIGNAL(finished()), this, SLOT(ReplyFinishedSlot()));
	connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), 
			this, 
			SLOT(ErrorReplySlot(QNetworkReply::NetworkError)));
}

void DatabaseManager::ScheduledTimeToDatabase(const QString device_id, 
		const std::chrono::system_clock::time_point time_point)
{

	const QJsonDocument json_doc =
		CreateDatabaseEventJson(device_id, collection_events,
				type_collection_start,
				1,
				time_point);

	JsonToDatabaseNAM(json_doc);

}

void DatabaseManager::PushErrorEvent(const QString device_id,
		const std::chrono::system_clock::time_point time_point,
		const int error_value)
{
	const QJsonDocument json_doc =
		CreateDatabaseEventJson(device_id, error_events,
				type_error_event,
				error_value,
				time_point);

	JsonToDatabase(json_doc);
}

void DatabaseManager::PushInitEvent(const QString device_id, 
			const std::chrono::system_clock::time_point time_point)
{
	const QJsonDocument json_doc =
		CreateDatabaseEventJson(device_id, events,
				type_event_init,
				1,
				time_point);

	JsonToDatabase(json_doc);

}

void DatabaseManager::PushRendezvousEvent(const QString device_id,
		const std::chrono::system_clock::time_point time_point)
{
	const QJsonDocument json_doc =
		CreateDatabaseEventJson(device_id, events,
				type_event_rendezvous,
				1,
				time_point);

	JsonToDatabase(json_doc);

}

void DatabaseManager::PushTimeRequestEvent(const QString device_id, 
		const std::chrono::system_clock::time_point time_point)
{
	const QJsonDocument json_doc =
		CreateDatabaseEventJson(device_id, events,
				type_event_time,
				1,
				time_point);

	JsonToDatabase(json_doc);

}

void DatabaseManager::TimeConvertToDeviceTime(
		const std::chrono::system_clock::time_point &time_point,
		timestamp *timestamp_struct)
{
	// timestamp to BCD
	const std::time_t c_time_point = std::chrono::system_clock::to_time_t(time_point);
	const tm * tm_UTC = std::gmtime(&c_time_point);

	timestamp_struct->seconds = ((tm_UTC->tm_sec / 10) << SHIFT_DECIMAL_10_SEC) |
		(tm_UTC->tm_sec % 10);

	timestamp_struct->minutes = ((tm_UTC->tm_min / 10) << SHIFT_DECIMAL_10_MIN) |
		(tm_UTC->tm_min % 10);

	timestamp_struct->hour = ((tm_UTC->tm_hour / 10) << SHIFT_DECIMAL_10_HOUR) | 
			((tm_UTC->tm_hour % 10)) ;

	static const int week_day_offset = 1;
	timestamp_struct->day = tm_UTC->tm_wday + week_day_offset;

	timestamp_struct->date = ((tm_UTC->tm_mday / 10) << SHIFT_DECIMAL_10_DATE) |
		(tm_UTC->tm_mday % 10);

	static const int month_offset = 1;
	timestamp_struct->century_month = (((tm_UTC->tm_mon + month_offset) / 10) << 
			SHIFT_DECIMAL_10_MONTH) |
		((tm_UTC->tm_mon + month_offset) % 10);

	static const int year_offset = 100;
	timestamp_struct->year = ((tm_UTC->tm_year - year_offset) / 10) << 
		SHIFT_DECIMAL_10_YEAR |
		((tm_UTC->tm_year - year_offset) % 10);

}

void DatabaseManager::TimeConvertToHostTime(const timestamp &timestamp_struct, 
		std::chrono::system_clock::time_point * system_time)
{
	// TODO check if time is plausible
	const int seconds = 10 * 
		((MASK_DECIMAL_10_SEC & timestamp_struct.seconds) >>
		 SHIFT_DECIMAL_10_SEC) + 
		(MASK_DECIMAL_1_SEC & timestamp_struct.seconds);

	const int minutes = 10 * 
		((MASK_DECIMAL_10_MIN & timestamp_struct.minutes) >>
		 SHIFT_DECIMAL_10_MIN) + 
		(MASK_DECIMAL_1_MIN & timestamp_struct.minutes);

	// save in 24h mode
	int hours;
	// 24h mode?
	if (!(MASK_DECIMAL_24_HOUR & timestamp_struct.hour))
	{
		hours = 20*((MASK_DECIMAL_20_HOUR & timestamp_struct.hour) >> 
				SHIFT_DECIMAL_20_HOUR) +
		10*((MASK_DECIMAL_10_HOUR & timestamp_struct.hour) >>
				SHIFT_DECIMAL_10_HOUR) + 
		(MASK_DECIMAL_1_HOUR & timestamp_struct.hour);
	}else {
		hours = 10*(MASK_DECIMAL_10_HOUR & 
				timestamp_struct.hour >> 
				SHIFT_DECIMAL_10_HOUR) + 
			(MASK_DECIMAL_1_HOUR & timestamp_struct.hour);
		// p.m?
		if( MASK_DECIMAL_20_HOUR & timestamp_struct.hour)
		{
			if(hours < 12)
				hours += 12;
		}
		else {
			if(hours == 12)
				hours -= 12;
		}
	}

	const int day_month = 10 *
		((MASK_DECIMAL_10_DATE & timestamp_struct.date) >>
		 SHIFT_DECIMAL_10_DATE) + 
		(MASK_DECIMAL_1_DATE & timestamp_struct.date);

	const int month = 10 * 
		((MASK_DECIMAL_10_MONTH & timestamp_struct.century_month) >>
		 SHIFT_DECIMAL_10_MONTH) +
		(MASK_DECIMAL_1_MONTH & timestamp_struct.century_month);

	const int year = //2000 + 
		10 * ((MASK_DECIMAL_10_YEAR & timestamp_struct.year) >>
				SHIFT_DECIMAL_10_YEAR) + 
		(MASK_DECIMAL_1_YEAR & timestamp_struct.year);

	std::tm timeinfo = std::tm();
	timeinfo.tm_year = year + 100; // since 1900
	timeinfo.tm_mon = month - 1; // since january: 0..11
	timeinfo.tm_mday = day_month;
	timeinfo.tm_hour = hours;
	timeinfo.tm_min = minutes;
	timeinfo.tm_sec = seconds;

	std::time_t time_type = 
		std::move(std::mktime(&timeinfo));

	*system_time = std::move(std::chrono::system_clock::from_time_t(
				std::move(time_type)));

}


void DatabaseManager::TemperatureReadingToValues(const temperature_reading & temperatures,
		std::tuple<double, double, double, double> *converted_values)
{
	static const unsigned int MEASUREMENT_10_BIT_MASK = 0x3FF;

	// correct conversion from raw ADC values to temperature!
	static const double scale_factor = (110.0 / 1024.0);


	const unsigned int temperature_1 = 
		(((unsigned int)temperatures.temperatures_packed[0]) |
		( (unsigned int)temperatures.temperatures_packed[1] << 8) ) &
		MEASUREMENT_10_BIT_MASK;

	const unsigned int temperature_2 = 
		(((unsigned int)temperatures.temperatures_packed[1] >> 2) |
		( (unsigned int)temperatures.temperatures_packed[2] << 6) ) &
		MEASUREMENT_10_BIT_MASK;

	const unsigned int temperature_3 = 
		(((unsigned int)temperatures.temperatures_packed[2] >> 4) |
		( (unsigned int)temperatures.temperatures_packed[3] << 4) ) &
		MEASUREMENT_10_BIT_MASK;

	const unsigned int temperature_4 = 
		(((unsigned int)temperatures.temperatures_packed[3] >> 6)|
		( (unsigned int)temperatures.temperatures_packed[4] << 2) ) &
		MEASUREMENT_10_BIT_MASK;

	std::get<3>(*converted_values) = (temperature_4 * scale_factor);
	std::get<2>(*converted_values) = (temperature_3 * scale_factor);
	std::get<1>(*converted_values) = (temperature_2 * scale_factor);
	std::get<0>(*converted_values) = (temperature_1 * scale_factor);
}

void DatabaseManager::PostReplyFinishedSlot(QNetworkReply * reply)
{
	reply->deleteLater();
}

void DatabaseManager::ReplyFinishedSlot()
{
	--open_network_replies_;
	if(!open_network_replies_)
	{
		emit AllFinished();
	}
}

void DatabaseManager::ErrorReplySlot(QNetworkReply::NetworkError err)
{
	std::cout << "Networking Error: " << err << std::endl;
}

