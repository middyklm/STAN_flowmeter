/*
 * ds3231RTC.c
 *
 * Created: 20/02/2016 9:25:47 PM
 *  Author: cra24x
 * the code here is from https://www.ccsinfo.com/forum/viewtopic.php?t=50256 by sshahryiar 
 * and been modified by pascal Craw to work with the i2c.h files from williams:avr programming https://github.com/hexagon5un/AVR-Programming
 */ 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include "DS3231.h"
#include "i2c.h"
#include "logging.h"

extern void uart_puts (int uart_port,const char *send);
extern void uart_printBinaryByte(int uart_port,uint8_t byte);



void DS3231_init(){
	LogDebug("DS3231_init() started\r\n");
	/* sets the microcontroller pin connected to the ds3231 to input, to receive the signal	*/
 	DS3231_INT_DDR &= ~(1<<DS3231_INT_PIN);		
 	DS3231_INT_PORT |= (1<<DS3231_INT_PIN);
	
	DS3231_Write(controlREG, 0x05); // bit 2 (INTCN) and 1 (A1IE) set high to enable interrupt control and alarm1 interrupt enable respectively
	DS3231_Write(statusREG, 0x00);	//
	// ds3231 data for debugging

	LogDebug("DS3231_init() complete\r\n");
}


void DS3231PrintRegisterValue(unsigned char register_address){
	uart_printBinaryByte(0,DS3231_Read(register_address));
	uart_puts(0,"\r\n");
}



unsigned char bcd_to_decimal(unsigned char d){
	return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}


unsigned char decimal_to_bcd(unsigned char d){
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}


unsigned char DS3231_Read(unsigned char address){     //working after being converted to work with the i2c.h  files from "williams:avr programming"
	unsigned char value = 0;
	i2cStart();					//I2C_start();
	i2cSend(DS3231_Write_addr);	//I2C_write(DS3231_Write_addr);
	i2cSend(address);			//I2C_write(address);
	i2cStart();					//I2C_start();
	i2cSend(DS3231_Read_addr);	//I2C_write(DS3231_Read_addr);
	value = i2cReadNoAck();		//value = I2C_read(0);
	i2cStop();					//I2C_stop();
	return value;
}


void DS3231_Write(unsigned char address, unsigned char value){ //working after being converted to work with the i2c.h  files from "williams:avr programming"
	
	i2cStart();					// 	I2C_start();
	i2cSend(DS3231_Write_addr);	// 	I2C_write(DS3231_Write_addr); // Select device and choose write mode (Middy's notes)
	i2cSend(address);			// 	I2C_write(address);			  // Select write address
	i2cSend(value);				// 	I2C_write(value);			  // Write data to selected address, after each write, wait for complete, included in i2cSend()
	i2cStop();					// 	I2C_stop();
}


void printTimestring(){
	int seconds;
	int minutes;
	int hours;
	
	char secondsString[3];
	char minutesString[3];
	char hoursString[3];
	
	seconds = DS3231_Read(secondREG);
	seconds = bcd_to_decimal(seconds);
	minutes = DS3231_Read(minuteREG);
	minutes = bcd_to_decimal(minutes);
	hours	= (0x3F & DS3231_Read(hourREG));
	hours	= bcd_to_decimal(hours);
	// converting int to strings
	snprintf (secondsString, 3, "%02d", seconds ); 
	snprintf (minutesString, 3, "%02d", minutes );
	snprintf (hoursString, 3, "%02d", hours );
	//prints current time
	printf("RTC time %s:%s:%s\n",hoursString,minutesString,secondsString);
}

 
void printDatestring(){
	unsigned int year;		//variables to hold data from ds3231 registers
	unsigned int month;
	unsigned int date;
	unsigned int day;
	
	char yearString [3];	// strings to hold converted data from ds3231
	char monthString [3];
	char dateString [3];
	//char dayString [2];
	
	year = DS3231_Read(yearREG);				// reading data registers
	year = bcd_to_decimal(year);				// converting register data to int
	month = (0x1F & DS3231_Read(monthREG));
	month = bcd_to_decimal(month);
	date = (0x3F & DS3231_Read(dateREG));
	date = bcd_to_decimal(date);
	day = (0x07 & DS3231_Read(dayREG));
	day = bcd_to_decimal(day);
	
	snprintf (yearString, 5, "%02d", year ); // converting int to strings with leading zeros and fixed width
	snprintf (monthString, 3, "%02d", month ); 
	snprintf (dateString, 3, "%02d", date ); 
	// print the date data as string
	printf("RTC date %s/%s/%s\n",dateString,monthString,yearString);
}
	
	
void setTime(unsigned char hSet, unsigned char mSet, unsigned char sSet){
	//unsigned char tmp = 0;
	DS3231_Write(secondREG, (decimal_to_bcd(sSet)));
	DS3231_Write(minuteREG, (decimal_to_bcd(mSet)));
	DS3231_Write(hourREG, (0x3F & (decimal_to_bcd(hSet))));
}


void setClockfromString(char command_value_string_passed[]){
	// takes a "hhmmss" string and sets the rtc time
	printf ("command_value_string_passed = %s and is %d long\n", command_value_string_passed,strlen(command_value_string_passed) );
	// check that string length is 7 chars long hhmmss plus the null terminator
	if (strlen(command_value_string_passed) == 6){
		printf("correct string length for setting time has been received\n");
		// reads the string passed into function and parsis and converts it to unsigned chars
		unsigned char hours = ((command_value_string_passed[0] - '0') *10)  +  (command_value_string_passed[1] - '0');
		unsigned char minutes = ((command_value_string_passed[2] - '0') *10)  +  (command_value_string_passed[3] - '0');
		unsigned char seconds = ((command_value_string_passed[4] - '0') *10)  +  (command_value_string_passed[5] - '0');
		printf("time sent to RTC = %d:%d:%d\n", hours,minutes,seconds );
		//sends passed command to RTC to set time
		setTime(hours, minutes, seconds);
		printTimestring();
	}
	else{
	printf("ERROR: incorrect string length for setting time has been received *** clock time NOT set ***");
	}
}


void setDate( unsigned char dateSet, unsigned char monthSet, unsigned char yearSet){
	DS3231_Write(dateREG, (decimal_to_bcd(dateSet)));
	DS3231_Write(monthREG, (decimal_to_bcd(monthSet)));
	DS3231_Write(yearREG, (decimal_to_bcd(yearSet)));
}


void setDatefromString(char command_value_string_passed[]){
	//// takes a "ddmmyy" string and sets the rtc date
	printf ("command_value_string_passed = %s and is %d long\n", command_value_string_passed,strlen(command_value_string_passed) );
	// check that string length is 7 chars long ddmmyy plus the null terminator
	if (strlen(command_value_string_passed) == 6){
		printf("correct string length for setting date has been received\n");
		// reads the string passed into function and parsis and converts it to unsigned chars
		unsigned char day = ((command_value_string_passed[0] - '0') *10)  +  (command_value_string_passed[1] - '0');
		unsigned char month = ((command_value_string_passed[2] - '0') *10)  +  (command_value_string_passed[3] - '0');
		unsigned char year = ((command_value_string_passed[4] - '0') *10)  +  (command_value_string_passed[5] - '0');
		printf("date sent to RTC = %d/%d/%d\n", day,month,year );
		//sends passed command to RTC to set time
		setDate( day, month	, year);
		printDatestring();
	}
	else{
		printf("ERROR: incorrect string length for setting date has been received *** clock date NOT set ***");
	}
}


float getTemp(){
	register float t = 0.0;
	unsigned char lowByte = 0;
	signed char highByte = 0;
	lowByte = DS3231_Read(tempLSBREG);
	highByte = DS3231_Read(tempMSBREG);
	lowByte >>= 6;
	lowByte &= 0x03;
	t = ((float)lowByte);
	t *= 0.25;
	t += highByte;
	return t;
}


void printRTCtemp(void){
	uart_puts(0,"printRTCtemp() started\r\n");
	char rtcTempstr[6];					// str to hold the
	float rtcTemp;						// float to hold the temp from rtc
	rtcTemp = getTemp();				// gets the temp as float from rtc
	dtostrf(rtcTemp,4,1,rtcTempstr);		// converts the float retuned from the rtc getTemp() function and turns it into a string at rtcTempstr
	uart_puts(0,rtcTempstr);
	uart_puts(0,"\r\n");
}




void printAlarm1string(){
	int seconds;
	int minutes;
	int hours;
	int date;

	char secondsString[3];
	char minutesString[3];
	char hoursString[3];
	char dateString[3];
	

	seconds = (DS3231_Read(alarm1secREG) & 0b01111111);
	seconds = bcd_to_decimal(seconds);
	minutes = (DS3231_Read(alarm1minREG) & 0b01111111);
	minutes = bcd_to_decimal(minutes);
	hours	= (0x3F & DS3231_Read(alarm1hrREG));
	hours	= bcd_to_decimal(hours);
	date	= (0x3F & DS3231_Read(alarm1dateREG));
	date	= bcd_to_decimal(date);
	
	
	// converting int to strings
	snprintf (secondsString, 3, "%02d", seconds );
	snprintf (minutesString, 3, "%02d", minutes );
	snprintf (hoursString, 3, "%02d", hours );
	snprintf (dateString, 3, "%02d", date );
	//prints current time
	printf("RTC Alarm1 set for %s:%s:%s on the %s day of the month\n",hoursString,minutesString,secondsString, dateString);
}

void ds3231Alarm1settime(unsigned char hSet, unsigned char mSet, unsigned char sSet, unsigned char dateSet){

	int A1M1 = DS3231_Read(alarm1secREG) & 0x80;		// records the sate of the alarm mask bits
	int A1M2 = DS3231_Read(alarm1minREG) & 0x80;
	int A1M3 = DS3231_Read(alarm1hrREG) & 0x80;
	int A1M4 = DS3231_Read(alarm1dateREG) & 0xC0;		// holds the date/day selection bit also
	
	DS3231_Write(alarm1secREG, ( A1M1  |  (decimal_to_bcd(sSet)))); // adds the previously stored bits from the alarm mask bits back to the new set time value and sends them to the RTC
	DS3231_Write(alarm1minREG, ( A1M2  |  (decimal_to_bcd(mSet))));
	DS3231_Write(alarm1hrREG,  (A1M3   |  ( 0x3F  &  (decimal_to_bcd(hSet)))));
	DS3231_Write(alarm1dateREG,(A1M4   |  ( 0x3F  &  (decimal_to_bcd(dateSet)))));
	// print alarm time to be sure new time is correct
	printAlarm1string();
	ds3231Alarm1on();
}



void ds3231Alarm1DateAndTimeMatch(){  /* clear all the alarm register mask bits to activate the alarm1 on time and date match*/

	//ds3231PrintAlarm1RegisterValues();

	DS3231_Write(alarm1secREG, 	(DS3231_Read(alarm1secREG) & 0x7f)); // removes the MSB from the register value (sets it to zero) of alarm 1 mask bits and sends it back to the RTC
	DS3231_Write(alarm1minREG,  (DS3231_Read(alarm1minREG) & 0x7f));
	DS3231_Write(alarm1hrREG,   (DS3231_Read(alarm1hrREG) & 0x7f));
	DS3231_Write(alarm1dateREG, (DS3231_Read(alarm1dateREG) & 0x3f));
	
	ds3231PrintAlarm1RegisterValues();
	// print alarm time to be sure new time is correct
	printAlarm1string();
	
}



void setAlarm1string(char command_value_string_passed[]){	
	// takes a "hhmmssDD" string and sets the rtc time
	printf ("command_value_string_passed = %s and is %d long\n", command_value_string_passed,strlen(command_value_string_passed) );
	// check that string length is 9 chars long (hhmmss plus the null terminator)
	if (strlen(command_value_string_passed) == 8){
		printf("correct string length (hhmmssDD) for setting time and day of month has been received\n");
		// reads the string passed into function, parses and converts it to unsigned chars
		unsigned char hours = ((command_value_string_passed[0] - '0') *10)  +  (command_value_string_passed[1] - '0');
		unsigned char minutes = ((command_value_string_passed[2] - '0') *10)  +  (command_value_string_passed[3] - '0');
		unsigned char seconds = ((command_value_string_passed[4] - '0') *10)  +  (command_value_string_passed[5] - '0');
		unsigned char date = ((command_value_string_passed[6] - '0') *10)  +  (command_value_string_passed[7] - '0');
		printf("time and day sent to RTC = %d:%d:%d on the %d day of the month\n", hours,minutes,seconds,date);
		//sends passed command to RTC to set time
		ds3231Alarm1settime(hours,minutes,seconds,date);
	}
	else{
		printf("ERROR: incorrect string length for setting alarm time and date has been received *** alarm NOT set ***");
	}
}

	
void ds3231PrintAlarm1RegisterValues(){
	uart_puts(0,"DS3231_init() - REG 07x (secs) value is ");
	DS3231PrintRegisterValue(0x7);

	uart_puts(0,"DS3231_init() - REG 08x (mins) value is ");
	DS3231PrintRegisterValue(0x8);
	
	uart_puts(0,"DS3231_init() - REG 09x (hour) value is ");
	DS3231PrintRegisterValue(0x9);
	
	uart_puts(0,"DS3231_init() - REG 0Ax (Date) value is ");
	DS3231PrintRegisterValue(0xA);
}


void ds3231Alarm1on(){
	//set bit 0 to high to allow the alarm flag to assert INT/SQW pin
	DS3231_Write(controlREG,(DS3231_Read(controlREG)) | 0b00000001);
	//print alarm bit state
	printf("Alarm bit = %i\r\n", (DS3231_Read(controlREG) & 0b00000001) );	
	uart_puts(0,"RTC Alarm 1 ON");
	// print register data for debugging
// 	uart_puts(0,"DS3231_init() - controlREG value is ");
// 	DS3231PrintRegisterValue(0xE);
// 	uart_puts(0,"DS3231_init() - statusREG  value is ");
// 	DS3231PrintRegisterValue(0xF);
	//ds3231PrintAlarm1RegisterValues();
	//ds3231Alarm1DateAndTimeMatch(); // sets alarm 1 mode to go when date and time match
}


void ds3231Alarm1off(){
	//set bit 0 to high to allow the alarm flag to assert INT/SQW pin
	DS3231_Write(controlREG,(DS3231_Read(controlREG)) & ~(0b00000001));
	//print alarm bit state
	printf("Alarm bit = %i\r\n", (DS3231_Read(controlREG) & 0b00000001) );	
	// print register data for debugging
	uart_puts(0,"DS3231_init() - controlREG value is ");
	//DS3231PrintRegisterValue(0xE);
	//uart_puts(0,"DS3231_init() - statusREG  value is ");
	//DS3231PrintRegisterValue(0xF);
}




char * TimeDateString(){
	unsigned int year;		//variables to hold data from ds3231 registers
	unsigned int month;
	unsigned int date;
	unsigned int day;
	unsigned int seconds;
	unsigned int minutes;
	unsigned int hours;
	
	char secondsString[3];
	char minutesString[3];
	char hoursString[3];
	char yearString [3];	// strings to hold converted data from ds3231
	char monthString [3];
	char dateString [3];
	
	char *dateTime = "000000,000000"; // format = ddmmyy,hhmmss
	//dateTime = malloc (sizeof (char) * 14);
	//char dayString [2];
	
	
	year = DS3231_Read(yearREG);				// reading data registers
	year = bcd_to_decimal(year);				// converting register data to int
	month = (0x1F & DS3231_Read(monthREG));
	month = bcd_to_decimal(month);
	date = (0x3F & DS3231_Read(dateREG));
	date = bcd_to_decimal(date);
	day = (0x07 & DS3231_Read(dayREG));
	day = bcd_to_decimal(day);
	seconds = DS3231_Read(secondREG);
	seconds = bcd_to_decimal(seconds);
	minutes = DS3231_Read(minuteREG);
	minutes = bcd_to_decimal(minutes);
	hours	= (0x3F & DS3231_Read(hourREG));
	hours	= bcd_to_decimal(hours);
		
	// converting int to strings
	snprintf (secondsString, 3, "%02d", seconds );
	snprintf (minutesString, 3, "%02d", minutes );
	snprintf (hoursString, 3, "%02d", hours );
	snprintf (yearString, 5, "%02d", year ); // converting int to strings with leading zeros and fixed width
	snprintf (monthString, 3, "%02d", month );
	snprintf (dateString, 3, "%02d", date );
	
	
	snprintf (dateTime, 16, "%02d%02d%02d_%02d%02d%02d", date,month,year,hours,minutes,seconds );
	//printf("%s\r\n",dateTime);

	return dateTime;
}

void PrintTimeDateString(){
	printf("testing %s \r\n", (TimeDateString()));
}

