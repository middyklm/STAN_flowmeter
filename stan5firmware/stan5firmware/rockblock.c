/*
 * iridium.c
 *
 * Created: 29/07/2019 3:37:15 PM
 *  Author: Middy Khong
 */ 
#include "rockblock.h"

#define ROCKBLOCK_UART_PORT 3  // the usart port that the rockblock iridium is connected to 


// test if rockblock is connected by sending "AT" and receiving "OK"
char rockblock_uart_test(void){
	
	uart_puts(0, "testing UART3 Port\r\n"); //tell the debug serial port what we are doing
	uart_puts(ROCKBLOCK_UART_PORT, "AT\r\n"); // send "OK" to the rockblock uart port
	//uart_puts(0,data_in_uart3);
return 1;
}





/*
bool reSendData = false;
char * msgToRB = NULL;

bool sendToRB(char * msgToRockBlock){

	//USART0_disable(); 
	USART_putstring("Message sending to RockBlock is: ");	
	USART_putstring(msgToRockBlock);
	USART_putstring("\r\n");
	_delay_ms(100);
	
	memset(buffer3, 0, 127);					// Clear buffer3
	USART3_putstring("AT\r\n");					// Refill buffer3
	_delay_ms(100);
	
	USART3_putstring(msgToRockBlock);		// Send data to RockBlock, if RockBlock successfully receives the msg, it sends back the exact msg
	USART3_putstring("\r\n");
	_delay_ms(100);
	
	//USART_init();
	msgToRockBlock = NULL;					// Free msgToRockBlock pointer
	return true;
}

bool sendToServer(void){
	USART_putstring("\r\nMessage sent to RockBlock\r\n");
// 	memset(buffer3, 0, 127);					// Clear buffer3
// 	USART3_putstring("AT\r\n");
// 	_delay_ms(50); 
	USART_putstring("Forwarding message to server..\r\n");
	USART3_putstring(fwdToServer);				// Forward message from RockBlock to Server
	USART3_send('\r');
	USART3_send('\n');
	_delay_ms(500);
	return true;
}

bool sendStatus(char *status){
	char *ret0 = NULL, *ret2 = NULL;
	const char * sendSuccessfully0 = "+SBDIX: 0,";
	const char * sendSuccessfully2 = "+SBDIX: 2,";
	
	ret0 = strstr(status, sendSuccessfully0);		// If successfully sent message
	ret2 = strstr(status, sendSuccessfully2);		// If successfully sent message, but location update not accepted
	if(ret0 != NULL || ret2 != NULL){
		USART_putstring("\r\nSuccessfully sent message.\r\n");
		#ifdef LOGGING_ENABLE
			FRAM_record(msgType);
		#endif
		msgType = 0;
		_delay_ms(100);
		
		USART3_putstring("AT+SBDD0\r\n");			// Clear MO buffer
		_delay_ms(500);
// 		memset(buffer3, 0, 127);					// Clear buffer3
// 		USART3_putstring("AT\r\n");	_delay_ms(100);	// Send a command to refill buffer3
		USART3_putstring("AT+CCLK?\r\n");	// request UTC time from Iridium, also update UTC time for AVR, set StrRxFlag3, then AVR sleep
		
// 		USART1_disable();							// Disable USART1 and re-enable INT2 wakeup
// 		
// 		// Enter sleep mode
// 		Iridium_sleep();
// 		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
// 		USART_putstring("Sleep mode ON\r\n");
// 		_delay_ms(30);
// 		sleep_mode_on();
		
		return 0;
	}else{
		USART_putstring("\r\nFailed to send message. Try again in 1 minute..\r\n");	// Retry in 10 seconds
		reSendData= true;
		startTimer1_sec(60);
		Iridium_sleep();
		//_delay_ms(30000);
		
// 		if (reSendData){
// 			memset(buffer3, 0, 127);				// Clear buffer3
// 			USART3_putstring("AT\r\n");
// 			_delay_ms(100);
// 			USART3_putstring(fwdToServer);			// Forward message from RockBlock to Server
// 			USART3_send('\r');
// 			USART3_send('\n');
// 			USART_putstring("\r\nMessage re-sent\r\n");
// 			reSendData = false;					// Clear re-sendData flag
// 			_delay_ms(500);
// 		}
		return 1;
	}// end else
	
}

void reSendMessage(void){
	memset(buffer3, 0, 127);				// Clear buffer3
	USART3_putstring("AT\r\n");
	_delay_ms(100);
	USART3_putstring(fwdToServer);			// Forward message from RockBlock to Server
	USART3_send('\r');
	USART3_send('\n');
	USART_putstring("\r\nMessage re-sent\r\n");
}

bool receiveStatus(char *status){
	bool AttemptToGetMsg_irid = false;
	char connectionStatus[127];
	char *retRx = NULL;
	const char statusDelim = ',';
	uint8_t statusDelimCount = 0;
	uint8_t receivedSuccessfully = 1; char RxStatus[8]; int RxStatusInt = 0;
	
	// Parse connection status
	strncpy ( connectionStatus, status, sizeof(connectionStatus) );
	USART_putstring("\r\n[DEBUG] After detecting +SBDIX. Connection status is:\r\n");
	USART_putstring(connectionStatus);				// Get a string of connection status starting from +SBDIX:....
	USART_send('\r'); USART_send('\n');
		
	// Parse receive status
	USART_putstring("\r\nAttempting to receive messages\r\n");
	
	retRx =strchr(connectionStatus, statusDelim);
	
	while (retRx != NULL)
	{	statusDelimCount++;
		memset(RxStatus, 0, sizeof(RxStatus));					// Clear RxStatus before assigning it with retRx value
		if (statusDelimCount == 2){
			USART_putstring("\r\nFound two commas ','\r\n");
			RxStatus[0] = retRx[2];				// Get status of MT (only 1 digit in string format)
			RxStatusInt = string_to_int(RxStatus);		// Convert RxStatus from a string to an integer number
			USART_putstring("\r\nretRX is:\r\n");
			USART_putstring(retRx);
			USART_putstring("\r\nRx status is:\r\n");
			USART_putstring(RxStatus);
			USART_send('\r');           // Send carriage return
			USART_send('\n');           // Send linefeed
		}
		
		if (statusDelimCount == 5){
			USART_putstring("\r\nFound the last ','. retRX is:\r\n");
			USART_putstring(retRx);
			USART_send('\r');           // Send carriage return
			USART_send('\n');           // Send linefeed
			queuedMsg[0] = 0x00;		// Clean queuedMsg before adding value into it
			queuedMsg[0] = retRx[2];						// Get number of message queued at GSS (server)
			queuedMsgInt = string_to_int(queuedMsg);	// Convert queuedMsg from string to integer
			
		}
		
		retRx = strchr(retRx+1, statusDelim);
		
	}// end while (retRx != NULL)
	
	// Check status
	if (RxStatusInt == receivedSuccessfully){
		USART_putstring("\r\nSuccessfully received a message from the server.\r\n");
		USART_putstring("\r\nNumber of messages queued at GSS: ");
		USART_putstring(queuedMsg);
		USART_send('\r');				// Send carriage return
		USART_send('\n');				// Send linefeed
		
		AttemptToGetMsg_irid = 0;			// Clear attempt to receive message flag
		waitingReceiveStatus = 0;
		USART_putstring("\r\nReading the received message..\r\n");
		readTextData();
		}else{
		USART_putstring("\r\nFailed to receive a message. Re-attempt in 1 minute..\r\n");
		//_delay_ms(30000);
		startTimer1_sec(60);
		Iridium_sleep();
		AttemptToGetMsg_irid = 1;
	}
	
	return AttemptToGetMsg_irid;
}

void tryToGetServerMsg(void){
	USART3_putstring(connectToServerForRx);	// Initiate a SBD connection to server to receive a message
	USART3_send('\r');						// Send carriage return
	USART3_send('\n');						// Send line feed
	USART_putstring("\r\nRequested network connection\r\n");
	_delay_ms(500);
}

void readTextData(void){
	char readTextDataCmd[] = "AT+SBDRT";	
	
	USART3_putstring(readTextDataCmd);
	USART3_send('\r');				// Send carriage return
	USART3_send('\n');				// Send linefeed

}

char * processServerMsg(char * retIrid){
	char * textData_irid = NULL;
	char data[127];

	_delay_ms(1000);	// require a delay to parse message
	// Uncomment to debug
	// USART_putstring("[DEBUG] In iridium, serverMsg is: "); USART_putstring(serverMsg); USART_putstring("\r\n");
	
	// Uncomment to debug
	//USART_putstring("\r\n[DEBUG] Content of message is:\r\n"); USART_putstring(retIrid); USART_send('\r'); USART_send('\n');		

		for (int ii=0; ii<strlen(retIrid); ii++){
			data[ii] = retIrid[ii];			
			if (data[ii]=='K' && data[ii-1]=='O'){
				data[ii] = data[ii-1] = data[ii-2] = data[ii-3] = 0x00; // Delete 'K', 'O', '\n', '\r'
				break;
			}
		}
		
		//textData_irid = retIrid + 9;		
		strcpy ( data, data + 9);		// Cut prefix +SBDRT:\r\n		
		textData_irid = data;			// Change array to pointer so that this string can send through function
		
		#ifdef LOGGING_ENABLE
			LogInfo(textData_irid); 
		#endif
		// Uncomment to debug		
		//USART_putstring("\r\n[DEBUG] In iridium.c, received message:\r\n");	USART_putstring(textData_irid);	USART_send('\r'); USART_send('\n');
		
		USART3_putstring("AT+SBDD0\r\n");			// Clear MO buffer
		_delay_ms(500);
		memset(buffer3, 0, 127);					// Clear buffer3		
		USART3_putstring("AT\r\n");					// Refill buffer3, so AVR can detect next message from RockBlock
	
	return textData_irid;
}

void Iridium_sleep(void){
	_delay_ms(500);
	PORTK &= ~(1<<PK0);		// Turn off RockBlock
	USART_putstring("Iridium unit sleeping..\r\n");
}

void Iridium_wakeup(void){
	PORTK |= (1<<PK0);		// Turn on RockBlock
	_delay_ms(1000);
	USART_putstring("Iridium unit on\r\n");
//	USART3_putstring("AT\r\n");		// Send a command to refill buffer3	
//	_delay_ms(100);
}

void Iridium_charge(void){
	startTimer1_sec(60);	// set charging time in sec here
	USART_putstring("Charging Iridium unit (1 min). Press any key to stop\r\n");
	waitForUser = true;
	while(waitForUser){
		if (StrRxFlag)
		{
			//USART_putstring("\r\nYou entered "); USART_putstring(buffer); USART_putstring("\r\n");
			USART_putstring("Stop charging\r\n");
			timer1_stop();
			memset(buffer, 0, 127);
			waitForUser = false;
			StrRxFlag = 0;
		}
		if (timer1sup)
		{
			waitForUser = timer1sup = false;
		}
		timer1_counting();
	}
}

char *parseUTCtime(char *timeString){
	char *retUTC = NULL;
	const char UTC_prefix[] = "+CCLK:";
	char * date_time_str = NULL;
	
	retUTC = strstr(timeString, UTC_prefix);		// If found "ST=" in the string from SoundTrap, start forward process to send this string to server
	if (retUTC != NULL){
		date_time_str = retUTC+6;			// move pointer behind "+CCLK:"
		USART_putstring("Parsed UTC time: ");
		USART_putstring(date_time_str); USART_putstring("\r\n");
	}
	
	return date_time_str;
}

void set_UTC_time_for_AVR(char *UTC_string){
	unsigned char yearUTC = ((UTC_string[0] - '0') *10)  +  (UTC_string[1] - '0');
	unsigned char monthUTC = ((UTC_string[3] - '0') *10)  +  (UTC_string[4] - '0');
	unsigned char dayUTC = ((UTC_string[6] - '0') *10)  +  (UTC_string[7] - '0');
	
	unsigned char hourUTC = ((UTC_string[9] - '0') *10)  +  (UTC_string[10] - '0');
	unsigned char minuteUTC = ((UTC_string[12] - '0') *10)  +  (UTC_string[13] - '0');
	unsigned char secondUTC = ((UTC_string[15] - '0') *10)  +  (UTC_string[16] - '0');
	
	if (yearUTC > 18)	// don't update time if UTC time is not up-to-date
	{
		// Set date for AVR
		setDate( dayUTC, monthUTC, yearUTC);
		printDatestring();
		
		// Set time for AVR
		setTime(hourUTC, minuteUTC, secondUTC);
		printTimestring();
	}
	
}

void FRAM_record(unsigned char type){
	switch(type)			
	{
		case 1:
		LogInfo("Explosion detected\r\n");
		break;
		case 11:
		LogInfo("Device: working\r\n");
		break;
		case 12:
		LogInfo("Device: hi Middy\r\n");
		break;
		default:
		LogInfo("Unknown message from hydrophone\r\n");
	}
}
*/