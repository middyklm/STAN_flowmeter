/*
 * logging.c
 *
 * Created: 3/06/2019 2:28:35 PM
 *  Author: cra24x
 */ 






#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>				
#include <stdlib.h>  
#include "DS3231.h"
#include "fram.h"


#define NO_LOGGING 0
#define ERROR_ONLY_LOGGING 1
#define INFO_LOGGING 2
#define DEBUG_LOGGING 3
#define VERBOSE_LOGGING 4



unsigned char logging_level_global = DEBUG_LOGGING;




/********************************************************
	* FUNCTION NAME: LogError
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: 
*********************************************************/
void LogError(char *event_data_string){
	// print the error log event if the log event priority is equal or lower than the log set point
	if (logging_level_global > 0){
		char str[120];
		char *datetimestr;
		datetimestr = malloc (sizeof (char) * 14);
		datetimestr = TimeDateString();
		sprintf(str, "%s %s %s","[ERROR] ", datetimestr, event_data_string);
		printf("%s",str);
		#ifdef FRAM_ENABLE // if the FRAM_ENABLE is defines in config.h or elsewhere then data will be writted to the FRAM also
			sprintf(str, "%s %s %s","[ERROR]* ", datetimestr, event_data_string);
			fram_writeString(str);	
		#endif
		free(datetimestr);
		
		
	}
}

/********************************************************
	* FUNCTION NAME: LogInfo
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: 
*********************************************************/
void LogInfo(char *event_data_string){
	// print the error log event if the log event priority is equal or lower than the log set point
	if (logging_level_global > 1){
		char str[120];
		char *datetimestr;
		datetimestr = malloc (sizeof (char) * 14);
		datetimestr = TimeDateString();
		sprintf(str, "%s %s %s","[INFO] ", datetimestr, event_data_string);
		printf("%s",str);
		#ifdef FRAM_ENABLE // if the FRAM_ENABLE is defines in config.h or elsewhere then data will be writted to the FRAM also
			sprintf(str, "%s %s %s","[INFO]* ", datetimestr, event_data_string);
			fram_writeString(str);
		#endif	
		free(datetimestr);
	}
}

/********************************************************
	* FUNCTION NAME: LogDebug
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: 
*********************************************************/
void LogDebug(char *event_data_string){
	// print the error log event if the log event priority is equal or lower than the log set point
	if (logging_level_global > 2){
		char str[120];
		char *datetimestr;
		datetimestr = malloc (sizeof (char) * 14);
		datetimestr = TimeDateString();
		sprintf(str, "%s %s %s","[DEBUG] ", datetimestr, event_data_string);
		printf("%s",str);
		#ifdef FRAM_ENABLE // if the FRAM_ENABLE is defines in config.h or elsewhere then data will be writted to the FRAM also
			sprintf(str, "%s %s %s","[DEBUG]* ", datetimestr, event_data_string);
			fram_writeString(str);
		#endif
		free(datetimestr);
	}
}

/********************************************************
	* FUNCTION NAME: LogVerbose
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: 
*********************************************************/
void LogVerbose(char *event_data_string){
	// print the error log event if the log event priority is equal or lower than the log set point
	if (logging_level_global > 3){
		char str[120];
		char *datetimestr;
		datetimestr = malloc (sizeof (char) * 14);
		datetimestr = TimeDateString();
		sprintf(str, "%s %s %s","[VERBOSE] ", datetimestr, event_data_string);
		printf("%s",str);
		#ifdef FRAM_ENABLE // if the FRAM_ENABLE is defines in config.h or elsewhere then data will be writted to the FRAM also
			sprintf(str, "%s %s %s","[VERBOSE]* ", datetimestr, event_data_string);
			fram_writeString(str);
		#endif
		free(datetimestr);
	}
}





	/********************************************************
	* FUNCTION NAME: LogData
	* ARGUMENTS: takes the data string to be printed and appends a [DATA] at the front end, prints to serial if it is not in no logging or error only mode
	* RETURNS: void, but should be changed to 1/0 for errror checking eventually
	* DESCRIPTION: logs data
	*********************************************************/
void LogData(char *event_data_string){
	// print the data event if the set point is greater than 1, i.e if it is not in error only mode
	if (logging_level_global > 1){  
		char str[120];
		char *datetimestr;
		datetimestr = malloc (sizeof (char) * 14);
		datetimestr = TimeDateString();
		sprintf(str, "[DATA] %s %s",datetimestr, event_data_string);
		printf("%s",str);
		#ifdef FRAM_ENABLE // if the FRAM_ENABLE is defines in config.h or elsewhere then data will be written to the FRAM also
			sprintf(str, "[DATA]* %s %s",datetimestr, event_data_string);
			fram_writeString(str);
		#endif
		free(datetimestr);
	}
}

/********************************************************
	* FUNCTION NAME: 
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: this function changes the logging_level_global variable to the input argument
	*********************************************************/
void LoggingLevelSet(unsigned char logging_level){   
	_delay_ms(50);
	logging_level_global = logging_level;
	if (logging_level_global == 0){  
		LogInfo("Logging turned OFF\r\n");
	}
	else if (logging_level_global == 1){
		LogInfo("Logging level set to ERROR\r\n");
	}
	else if (logging_level_global == 2){
		LogInfo("Logging level set to INFO\r\n");
	}
	else if (logging_level_global == 3){
		LogInfo("Logging level set to DEBUG\r\n");
	}
	else if (logging_level_global == 4){
		LogInfo("Logging level set to VERBOSE\r\n");
	}
	else{
		printf("[ERROR] LogEvent(): No valid logging level passed");
	} 
}

/********************************************************
	* FUNCTION NAME: 
	* ARGUMENTS: 
	* RETURNS: 
	* DESCRIPTION: this function changes the logging_level_global variable to the input argument
	*********************************************************/
void InitLogging(void){   
	LoggingLevelSet(logging_level_global);
}