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

#include <Wire.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "communication_structs.h" 

// RTC definitions
static const char RTC_I2C_ADDRESS = 0x68;
static const int RTC_INTERRUPT_PIN = 2; // pin 2
static const int RTC_INTERRUPT_NR = 0;

// temperature sensors definitions
static const int TEMP_SENSORS_POWER_PIN = 9; // pin 9
static const int TEMP_SENSORS_INPUT_1 = A0; // pin 23
static const int TEMP_SENSORS_INPUT_2 = A1; // pin 24
static const int TEMP_SENSORS_INPUT_3 = A2; // pin 25
static const int TEMP_SENSORS_INPUT_4 = A3; // pin 26

// Bleutooth definitions
static const int BT_POWER_PIN = 11; // pin 11

enum node_state_t { INIT, DATA, DUMP, TIME, SLEEP, TEST, COLLECT };
enum message_enum { OKAY_MSG = 0,  INIT_MSG = 1, DATA_MSG = 2, DUMP_MSG = 3, TIME_MSG = 4, TEST_MSG = 5, FINI_MSG = 6 };

volatile node_state_t node_state = INIT;

char receive_array[sizeof(rendezvous_answer) + sizeof(timestamp)] = {0};

struct temperature_readings_header temperature_data_header;
static const unsigned int MAX_NUMBER_OF_READINGS = 150;
unsigned char temperature_readings[sizeof(temperature_reading) * MAX_NUMBER_OF_READINGS] = {0};

struct timestamp current_alarm_times;

void isr_RTC_alarm()
{
	detachInterrupt(RTC_INTERRUPT_NR);
	node_state = COLLECT;

        //pinMode(13, OUTPUT);
        //digitalWrite(13, HIGH);

}

void SleepNWakeOnUSART()
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();

	sleep_mode();
	sleep_disable();
	power_all_enable();

}

void DeepSleepWakeOnRTC()
{
	// Attach interrupts (collect measurements)
	digitalWrite(13, LOW);
	pinMode(13, INPUT);

        pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);
        delay(200);
	attachInterrupt(0, isr_RTC_alarm, LOW);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	sleep_disable();

	detachInterrupt(RTC_INTERRUPT_NR);
        ClearRTCAlarmFlags();
        
        node_state = COLLECT;

}


void PowerOnBTAndWaitForMasterOK()
{
			// set GPIO pin VCC of BT module to Output and HIGH
			// power up BT module
			pinMode(BT_POWER_PIN, OUTPUT);
			digitalWrite(BT_POWER_PIN, HIGH);

			// setup BT modlue serial connection
			Serial.begin(9600);
			// wait for receive Master OK
			// take a nap while nothing happens on UART
			while( !Serial.available() )
			{
				SleepNWakeOnUSART();
			}

			unsigned char master_message;
			master_message = (unsigned char)  Serial.read();
}

void PowerOffBT()
{
	Serial.end();
	digitalWrite(BT_POWER_PIN, LOW);
	pinMode(BT_POWER_PIN, INPUT);
}


void setup()
{

	// Configure the analog reference voltage source
	// to be the internal 1.1V source.
	analogReference(INTERNAL);

	node_state = INIT;

	PowerOnBTAndWaitForMasterOK();
}

void loop()
{
        //Serial.write("AT");
        //delay(1000);
	//if(Serial.available())
        //{
	//	//Serial.write(Serial.read());
        //        if(Serial.read() == 'O')
        //        {
        //          if(Serial.read() == 'K')
        //            digitalWrite(13, !digitalRead(13));
        //        }
        //}
        //delay(1000);
        //Serial.write("Hello\n");
        //delay(1000);
	//while(true)
	//{
	//	Serial.write((unsigned char *) &temperature_data_header, sizeof(temperature_readings_header));
	//	Serial.write(0xFF);
	//}

        
	// setup BT modlue serial connection

	const struct timestamp * time_ptr;
	const struct rendezvous_answer * receive_ptr;
        struct temperature_reading * data_reading_ptr;
        
	switch (node_state) {
		case INIT:
		{
			// master connected!
			// send back INIT request to master
			Serial.write(INIT_MSG);

			// Master OKAY
			Serial.readBytes(receive_array, 1);
			// Master rendezvous answer and new time
			Serial.readBytes(receive_array, sizeof(rendezvous_answer) + 
					sizeof(timestamp));

			// update received time in RTC module
			time_ptr = (const timestamp * const) (receive_array + sizeof(rendezvous_answer));

			// end communication
			Serial.write(FINI_MSG);
			SetRTCTime(*time_ptr);
			Serial.flush();

			receive_ptr = (const rendezvous_answer * const) receive_array;
                        delay(10);
                        ClearRTCAlarmFlags();
                        delay(10);
			SetRTCAlarmFromTimestamp(receive_ptr->collection_start_time);
                        delay(10);
			EnableRTCAlarm1();

			temperature_data_header.start_time = receive_ptr->collection_start_time;
			temperature_data_header.interval_length_seconds = receive_ptr->interval_length_seconds;
			temperature_data_header.number_of_readings = 0;

			current_alarm_times.seconds = receive_ptr->collection_start_time.seconds;
			current_alarm_times.minutes = receive_ptr->collection_start_time.minutes;
			current_alarm_times.hour = receive_ptr->collection_start_time.hour;
			
                        //delay(10);
                        PowerOffBT();

			node_state = SLEEP;
			break;
		}
		case DUMP:
		{
			// master connected!
			// send back DUMP request to master
			Serial.write(DUMP_MSG);

			// receive master OKAY
			Serial.readBytes(receive_array, 1);

			// send out data
			Serial.write((unsigned char *) &temperature_data_header, sizeof(temperature_readings_header));
			Serial.write(temperature_readings, temperature_data_header.number_of_readings * sizeof(temperature_reading));
			Serial.flush();

			PowerOffBT();

			// zero out old temperature data
			temperature_data_header.number_of_readings = 0;
			memset(temperature_readings, 0, sizeof(temperature_readings));

			SetNextRTCAlarm();
			node_state = SLEEP;
			break;
		}
		case DATA:
		{
			// master connected!
			// send back DATA request to master  
			Serial.write(DATA_MSG);
			Serial.flush();

			// receive master OKAY 
			Serial.readBytes(receive_array, 1);
			delay(10);

			// commence data transmission
			//Serial.write((char *) &temperature_data_header, sizeof(temperature_readings_header));
			for(unsigned int i = 0; i < sizeof(temperature_readings_header); ++i)
			{
				Serial.write( ((char *) &temperature_data_header)[i] );
				Serial.flush();
				delay(15);
			}
			//Serial.write(temperature_readings, temperature_data_header.number_of_readings * sizeof(temperature_reading));
			const unsigned int bytes_to_send = temperature_data_header.number_of_readings * sizeof(temperature_reading);
			for(unsigned int i = 0; i <  bytes_to_send; ++i)
			{
				Serial.write(temperature_readings[i]);
				Serial.flush();
				delay(15);
			}
			Serial.readBytes(receive_array, sizeof(rendezvous_answer));
			PowerOffBT();

			receive_ptr = (rendezvous_answer *) receive_array;

			// set RTC alarm time as received
			SetRTCAlarmFromTimestamp(receive_ptr->collection_start_time);
			delay(10);
			EnableRTCAlarm1();

			temperature_data_header.start_time = receive_ptr->collection_start_time;
			temperature_data_header.interval_length_seconds = receive_ptr->interval_length_seconds;
			temperature_data_header.number_of_readings = 0;

			current_alarm_times.seconds = receive_ptr->collection_start_time.seconds;
			current_alarm_times.minutes = receive_ptr->collection_start_time.minutes;
			current_alarm_times.hour = receive_ptr->collection_start_time.hour;

			// zero out temperature data
			temperature_data_header.number_of_readings = 0;
			memset(temperature_readings, 0, sizeof(temperature_readings));
			node_state = SLEEP;
			break;
		}
		case TIME:
		{
			// send back TIME request to master
			Serial.write(TIME_MSG);
			Serial.flush();
			// receive master OKAY 
			Serial.readBytes(receive_array, 1);
			Serial.readBytes(receive_array, sizeof(timestamp));

			// update received time in RTC module
			time_ptr = (timestamp *) (receive_array);
			SetRTCTime(*time_ptr);
			delay(50);

			node_state = DATA;
			break;
		}
		case COLLECT:
		{
                        //pinMode(13, OUTPUT);
                        //digitalWrite(13, HIGH);

			// switch on temperature sensors
			pinMode(TEMP_SENSORS_POWER_PIN, OUTPUT);
			digitalWrite(TEMP_SENSORS_POWER_PIN, HIGH);

			// do other stuff until temperature sensors are stabilized

			data_reading_ptr = 
				(temperature_reading *) (temperature_readings + (temperature_data_header.number_of_readings * sizeof(temperature_reading)));
                        
                        temperature_data_header.number_of_readings += 1;
                        
			// memory full? if so we need to transmit the saved data next
			if(temperature_data_header.number_of_readings >= MAX_NUMBER_OF_READINGS)
			{
				PowerOnBTAndWaitForMasterOK();
				node_state = TIME;
			}
			else
			{
				// set next alarm timer for wakeup
				SetNextRTCAlarm();
				node_state = SLEEP;

                                pinMode(13, OUTPUT);
                                digitalWrite(13, !digitalRead(13));
			}

			// now read temperature values 
			ReadTemperatures(data_reading_ptr);
			// switch off temperature sensors
			digitalWrite(TEMP_SENSORS_POWER_PIN, LOW);

			break;
		}
		case SLEEP:
		{
			DeepSleepWakeOnRTC();
			break;
		}
		default:
                        break;
		// undefined state
	}
}

void SetNextRTCAlarm()
{
        //pinMode(13, OUTPUT);
        //digitalWrite(13, !digitalRead(13));

	const unsigned int seconds_of_next_alarm = 
		temperature_data_header.interval_length_seconds + 
		10*(current_alarm_times.seconds >> 4) + (current_alarm_times.seconds & 0x0F);
	const unsigned int minutes_to_add = seconds_of_next_alarm / 60;
	current_alarm_times.seconds = ((seconds_of_next_alarm % 60) / 10) << 4 | ((seconds_of_next_alarm % 60) % 10);

	const unsigned int minutes_of_next_alarm = 
		minutes_to_add + 
		10*(current_alarm_times.minutes >> 4) + (current_alarm_times.minutes & 0x0F);
	const unsigned int hours_to_add = minutes_of_next_alarm / 60;
	current_alarm_times.minutes = ((minutes_of_next_alarm % 60) / 10) << 4 | ((minutes_of_next_alarm % 60) % 10);

	const unsigned int hours_of_next_alarm = 
		hours_to_add + 
		10*(current_alarm_times.hour >> 4) + (current_alarm_times.hour & 0x0F);
	current_alarm_times.hour = ((hours_of_next_alarm % 24) / 10) << 4 | ((hours_of_next_alarm % 24) % 10);

	SetRTCAlarmFromTimestamp(current_alarm_times);
}


void EnableRTCAlarm1()
{
	Wire.begin();
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0E);
	Wire.write(0x05);
	Wire.endTransmission();
}


// Sets Alarm 1 of RTC according to stamp.
// Enables interrupt on hours, minutes and seconds match.
void SetRTCAlarmFromTimestamp(const timestamp & stamp)
{
	Wire.begin();
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x07);
        Wire.write(stamp.seconds);
	Wire.write(stamp.minutes);
	Wire.write(stamp.hour);
	Wire.write(0x80 | stamp.date);
	Wire.endTransmission();
}


void SetRTCTime(const timestamp & stamp)
{
	Wire.begin();
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x00);
	Wire.write(((unsigned char *)&stamp) + 1, sizeof(timestamp) - 1);
	Wire.endTransmission();
}


void ClearRTCAlarmFlags()
{
	Wire.begin();
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0x0F);
        Wire.write(0);
	Wire.endTransmission();
}

void RTCTime(timestamp * stamp)
{
	//TODO
}

void ReadTemperatures(struct temperature_reading * const data_reading_ptr)
{	
	int temp_value;

	delay(50);
	temp_value = analogRead(TEMP_SENSORS_INPUT_1);
	data_reading_ptr->temperatures_packed[0] |= (temp_value & 0xFF);
	data_reading_ptr->temperatures_packed[1] |= (temp_value >> 8);

	temp_value = analogRead(TEMP_SENSORS_INPUT_2);
	data_reading_ptr->temperatures_packed[1] |= (temp_value & 0x3F) << 2;
	data_reading_ptr->temperatures_packed[2] |= (temp_value >> 6);

	temp_value = analogRead(TEMP_SENSORS_INPUT_3);
	data_reading_ptr->temperatures_packed[2] |= (temp_value & 0x0F) << 4;
	data_reading_ptr->temperatures_packed[3] |= (temp_value >> 4);

	temp_value = analogRead(TEMP_SENSORS_INPUT_4);
	data_reading_ptr->temperatures_packed[3] |= (temp_value & 0x03) << 6;
	data_reading_ptr->temperatures_packed[4] |= (temp_value >> 2);

}

