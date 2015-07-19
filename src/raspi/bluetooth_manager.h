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


#ifndef BLUETOOTH_MANAGER_H_REVZMP9V
#define BLUETOOTH_MANAGER_H_REVZMP9V

#include <string>
#include <deque>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QCoreApplication>
#include <QList>
#include <QObject>

#include "database_manager.h"
#include "MAC_device_parser.h"
#include "scheduler.h"
#include "serial_communication.h"


// Starts the local Bluetooth device and handles search and discovery of remote Bluetooth services.
// Example usage:
// 	BluetoothManager bt_manager;
// 	bt_manager.Init();
class BluetoothManager : public QObject
{
	Q_OBJECT
public:
	// Creates BluetoothManager that checks the known devices provided by the parser.
	BluetoothManager (MACDeviceParser * device_file_parser_ptr,
			Scheduler<5> * scheduler_ptr, 
			DatabaseManager * database_manager_ptr) :
		device_file_parser_ptr_(device_file_parser_ptr),
		serial_communicator_(database_manager_ptr, scheduler_ptr)
	{}

	~BluetoothManager () {}

	// Reads known devices from file, powers up local Bluetooth device and
	// starts initial Bluetooth service discovery.
	void Init(QCoreApplication *qapp, const int argc, char *argv[]);

	// Launches the Bluetooth Service Discovery looking only for services given in filter_list.
	void DiscoverServices(const QList<QBluetoothUuid> &uuid_filter_list = {QBluetoothUuid::SerialPort});

	// Returns true if the local Bluetooth device is available.
	bool CheckLocalBluetoothDevice() const;

	const MACDeviceParser & DeviceFileParser()
	{
		return *device_file_parser_ptr_;
	}

public slots:
	// This is function is called whenever a new Bluetooth service is discovered.
	void ServiceDiscoveredHandler(const QBluetoothServiceInfo & service);
	void ServiceDiscoveryFinished();
	void RestartDiscovery();
	void ProcessNextMACAddress(const int argc);

signals:
	void StartMACAddressProcess(const int argc);
	void quitapp();

private:
	MACDeviceParser *device_file_parser_ptr_;
	SerialCommunicator serial_communicator_;
	QBluetoothLocalDevice local_bt_device_;
	QBluetoothServiceDiscoveryAgent bt_service_agent_;
	//QBluetoothDeviceDiscoveryAgent bt_device_agent_;
	QBluetoothSocket bt_socket_;
	std::deque<QBluetoothServiceInfo> service_queue_;
	std::vector<const char *> argv_;
};


#endif /* end of include guard: BLUETOOTH_MANAGER_H_REVZMP9V */

