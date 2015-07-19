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

#ifndef COMMUNICATION_STRUCTS_H_OCVXB0QO
#define COMMUNICATION_STRUCTS_H_OCVXB0QO

// Protocol description Start (connection is established):
//
// Node		<---'OKAY'----		Master
//
// Node		 ---'INIT'---->		Master	[CASE A]
// [--OR--]
// Node		 ---'DATA'---->		Master	[CASE B1]
// [--OR--]
// Node		 ---'DUMP'---->		Master	[CASE B2]
// [--OR--]
// Node		 ---'TIME'---->		Master	[CASE C]
// [--OR--]
// Node		 ---'TEST'---->		Master	[CASE D]
// [--OR--]
// Node		 ---'FINI'---->		Master
//

// *****************************************************
//
// Protocol description continuation for [CASE A]:
//
// Node		<---'OKAY'----		Master
// Node		<---[BIN 2]---		Master
// Node		<---[BIN 0]---		Master
//
// Node		 ---'FINI'---->		Master
// [--OR--]
// [CASE A] | [CASE B1] | [CASE B2] | [CASE C]
//
// Node receives data needed after initialization including time, collection start time and
// rendezvous time.
//
// *****************************************************
// *****************************************************
//
// Protocol description continuation for [CASE B1]:
//
// Node		<---'OKAY'----		Master
// Node		 ---[BIN 1]--->		Master
// Node		<---[BIN 2]---		Master
//
// Node sends collected data out and receives after that a new data collection start time and
// rendezvous time.
//
// *****************************************************
// *****************************************************
//
// Protocol description continuation for [CASE B2]:
//
// Node		<---'OKAY'----		Master
// Node		 ---[BIN 1]--->		Master
//
// Node just sends collected data out.
//
// *****************************************************
// *****************************************************
//
// Protocol description continuation for [CASE C]:
//
// Node		<---'OKAY'----		Master
// Node		<---[BIN 0]---		Master
//
// Node		 ---'FINI'---->		Master
// [--OR--]
// [CASE A] | [CASE B1] | [CASE B2] | [CASE C]
//
// Node receives current time from master.
// *****************************************************
// *****************************************************
//
// Protocol description continuation for [CASE D]:
//
// Node		<---'OKAY'----		Master
// Node		 ---[BIN 0]--->		Master
// Node		 ---[BIN T]--->		Master
//
// Node		 ---'FINIT---->		Master
// [--OR--]
// [CASE A] | [CASE B1] | [CASE B2] | [CASE C]
//
// Node receives current time from master.
// *****************************************************



#include <stdint.h>

// enum message_enum { OKAY_MSG = 0,  INIT_MSG = 1, DATA_MSG = 2, DUMP_MSG = 3, TIME_MSG = 4, TEST_MSG = 5, FINI_MSG = 6 };

// Bundles a readout of all four connected temperature sensors as a 5 byte large datachunk.
// [BIN T]
// 10 bit - Temperature 1  - a 
// 10 bit - Temperature 2  - b 
// 10 bit - Temperature 3  - c 
// 10 bit - Temperature 4  - d 
//
// MSB ..............................................................................  LSB
// d d d d d d d d | d d c c c c c c | c c c c b b b b | b b b b b b a a | a a a a a a a a
//     BYTE 4      |     BYTE 3      |     BYTE 2      |     BYTE 1      |     BYTE 0

struct temperature_reading {
	// four 10-Bit temperature readings packed into 5 Bytes
	unsigned char temperatures_packed[5]; 
};

// Holds a timestamp for the RTC.
// Each member should be formatted as described in the DS3231 datasheet for 
// corresponding registers.
// The member 'cents' is ignored for the currently used hardware. 
//
// [BIN 0]
// cents 	- a
// seconds	- b
// minutes	- c
// hour		- d
// day		- e
// date 	- f
// month	- g
// year		- h
//
// MSB ..............................................................................  LSB
//                                   | h h h h h h h h | g g g g g g g g | f f f f f f f f
//                                   |     BYTE 7      |     BYTE 6      |     BYTE 5
//                                   |                 |                 |
// e e e e e e e e | d d d d d d d d | c c c c c c c c | b b b b b b b b | a a a a a a a a
//     BYTE 4      |     BYTE 3      |     BYTE 2      |     BYTE 1      |     BYTE 0

struct timestamp {
	unsigned char cents;
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hour;
	unsigned char day;
	unsigned char date;
	unsigned char century_month;
	unsigned char year;
};

// Definition of header data sent by Node (Arduino) before the temperature readings are pushed out.
// [BIN 1]
// interval length seconds	- a
// number of readings		- b
// int
// MSB ..............................................................................  LSB
//                                   | b b b b b b b b | b b b b b b b b | b b b b b b b b
//                                   |     BYTE 15     |     BYTE 14     |     BYTE 13
//                                   |                 |                 |
// b b b b b b b b | a a a a a a a a | a a a a a a a a | a a a a a a a a | a a a a a a a a
//     BYTE 12     |     BYTE 11     |     BYTE 10     |     BYTE 9      |     BYTE 8
// BYTE 7 ---------------------------------[BIN 0]--------------------------------- BYTE 0

struct temperature_readings_header {
	timestamp start_time;
	uint32_t interval_length_seconds;
	uint32_t number_of_readings;
	// followed by number_of_readings temperature_reading
};

// Binary part of the Node (Arduino).
// [BIN 2]
//
// collection start time 	- A
// next rendezvous		- B
// interval length in seconds	- C
//
//                 | a a a a a a a a | a a a a a a a a | a a a a a a a a | a a a a a a a a
//                 |     BYTE 11     |     BYTE 10     |     BYTE 9      |     BYTE 8
// B BYTE 15---------------------------------[BIN 0]--------------------------------- BYTE 8
// A BYTE 7 ---------------------------------[BIN 0]--------------------------------- BYTE 0
struct rendezvous_answer {
	timestamp collection_start_time;
	timestamp next_rendezvous;
	uint32_t interval_length_seconds;
};


#endif /* end of include guard: COMMUNICATION_STRUCTS_H_OCVXB0QO */

