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
#include <iostream>
#include <string>
#include <queue>

#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QSerialPort>
#include <QString>

#include "bluetooth_manager.h"
#include "MAC_device_parser.h"


void BluetoothManager::Init(QCoreApplication *qapp, const int argc, char *argv[])
{
	// Parse known devices asynchronously.
	auto known_devices_future = std::async(std::launch::async, &MACDeviceParser::ParseForDevices, device_file_parser_ptr_ );

	// Start local Bluetooth Device.
	local_bt_device_.powerOn();

	connect(this, SIGNAL(quitapp()), qapp, SLOT(quit()));
	connect(this, SIGNAL(StartMACAddressProcess(const int)),
				this, SLOT(ProcessNextMACAddress(const int)));

	argv_.reserve(argc - 1);
	for (int i = 1; i < argc; ++i) {
		argv_.emplace_back(argv[i]);
	}

	known_devices_future.get();

	emit StartMACAddressProcess(argc - 2);

}

void BluetoothManager::ProcessNextMACAddress(const int argc)
{	
	if(argc >= 0)
	{
		std::cout << "processing bluetooth device" << std::endl;
		// valid bluetooth device?
		if( device_file_parser_ptr_->devices().count(QString(argv_[argc]).toLower().toStdString()) || 
			device_file_parser_ptr_->devices().count(QString(argv_[argc]).toUpper().toStdString()))
		{
			std::cout << "accept connection for bluetooth device" << std::endl;

			QSerialPort serial_port;
			//serial_port.setPortName("/dev/rfcomm1");
			serial_port.setPortName((device_file_parser_ptr_->file_paths().at(argv_[argc]).c_str()));
			serial_port.setBaudRate(QSerialPort::Baud9600);
			serial_port.setStopBits(QSerialPort::OneStop);
			serial_port.setDataBits(QSerialPort::Data8);
			serial_port.setParity(QSerialPort::NoParity);

			if(!serial_port.open(QIODevice::ReadWrite))
			{
				std::cout << serial_port.errorString().toStdString() << std::endl;
				return;
			}
			else
				std::cout << "port opened" << std::endl;

			//serial_communicator_.PerformCommunication(&serial_port, "20:15:04:10:26:60");
			serial_communicator_.PerformCommunication(&serial_port, argv_[argc]);
			serial_port.close();
		}
		emit StartMACAddressProcess(argc - 1);
	}
	else
	{
		std::cout << "proccessed everything exit!" << std::endl;
		emit quitapp();
	}
}


void BluetoothManager::DiscoverServices(const QList<QBluetoothUuid> &uuid_filter_list)
{
	// Agent, search for SerialPort services only!
	bt_service_agent_.clear();
	bt_service_agent_.setUuidFilter(uuid_filter_list);

	std::cout << "start discovery" << std::endl;
	// Start bluetooth service discovery agent (non-blocking).
	bt_service_agent_.start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
	//bt_service_agent_.start();
}

bool BluetoothManager::CheckLocalBluetoothDevice() const
{
	std::cout << "local Bluetooth address: " << 
		local_bt_device_.address().toString().toStdString() << std::endl;

	return local_bt_device_.isValid();
}

// slots:
void BluetoothManager::ServiceDiscoveredHandler(const QBluetoothServiceInfo & service)
{
	std::cout << "Service has been discovered." << std::endl;
	std::cout << "address:\t\t " << service.device().address().toString().toStdString() << std::endl;
	std::cout << "service description:\t " << service.serviceDescription().toStdString() << std::endl;
	std::cout << "service name:\t\t " << service.serviceName().toStdString() << std::endl;
	//std::cout << "service provider:\t " << service.serviceProvider().toStdString() << std::endl;
	std::cout << std::endl;

	if( device_file_parser_ptr_->devices().count(service.device().address().toString().toLower().toStdString()) || 
			device_file_parser_ptr_->devices().count(service.device().address().toString().toUpper().toStdString()))
	{
		std::cout << "Service will be processed." << std::endl;
		service_queue_.push_back(service);
	}
	else
	{
		std::cout << "Service will not be processed." << std::endl;
	}
}

void BluetoothManager::ServiceDiscoveryFinished()
{
	std::cout << "Service discovery has finished." << std::endl;

	// handle requests
	for (const auto & request : service_queue_) {
		QBluetoothSocket bt_socket;
		bt_socket.connectToService(request);
		serial_communicator_.PerformCommunication(&bt_socket, bt_socket.peerAddress().toString());
		bt_socket.close();
	}

	service_queue_.clear();
	bt_service_agent_.clear();
	const QString hc_06_serviceUuid = "00001101-0000-1000-8000-00805F9B34FB";
	QBluetoothUuid hc_bt_uuid(hc_06_serviceUuid);
	DiscoverServices({hc_bt_uuid, QBluetoothUuid::SerialPort, QBluetoothUuid::Rfcomm});
}

void BluetoothManager::RestartDiscovery()
{
	std::cout << "restart discovery" << std::endl;
	service_queue_.clear();
	bt_service_agent_.clear();
	const QString hc_06_serviceUuid = "00001101-0000-1000-8000-00805F9B34FB";
	QBluetoothUuid hc_bt_uuid(hc_06_serviceUuid);
	DiscoverServices({hc_bt_uuid, QBluetoothUuid::SerialPort, QBluetoothUuid::Rfcomm});
}
