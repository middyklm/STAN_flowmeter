/*
 * iridium.h
 *
 * Created: 29/07/2019 3:38:02 PM
 *  Author: Middy Khong
 */ 


#ifndef ROCKBLOCK_H_
#define ROCKBLOCK_H_

#define ROCKBLOCK_UART_PORT 3

#include "logging.h"
#include "DS3231.h"
#include "uart.h"



char rockblock_uart_test(void);

/*
#include "uart.h"
#include "mk_library.h"
#include "wakeup.h"
#include "timer.h"

#include "hydrophones.h"

uint8_t alarmInterval;


void reSendMessage(void);


// -----Receiving variables-------
#define connectToServerForRx "AT+SBDIXA"
bool waitingReceiveStatus;
char queuedMsg[8]; uint8_t queuedMsgInt;

// Receiving functions
bool receiveStatus(char *status);
void tryToGetServerMsg(void);
void readTextData(void);
char * processServerMsg(char * serverMsg);


void Iridium_sleep(void);
void Iridium_wakeup(void);
void Iridium_charge(void);

char *parseUTCtime(char *timeString);
void set_UTC_time_for_AVR(char *UTC_string);
void FRAM_record(unsigned char type);
//void sum(void);

*/
#endif /* IRIDIUM_H_ */
