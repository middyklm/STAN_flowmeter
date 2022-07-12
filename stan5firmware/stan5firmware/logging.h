/*
 * logging.h
 *
 * Created: 3/06/2019 2:28:11 PM
 *  Author: cra24x
 */ 






#ifndef LOGGING_H_
#define LOGGING_H_


#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

void InitLogging();
void LoggingLevelSet(unsigned char logging_level);
void LogError(char *event_data_string);
void LogInfo(char *event_data_string);
void LogDebug(char *event_data_string);
void LogVerbose(char *event_data_string);
void LogData(char *event_data_string);



#endif /* LOGGING_H_ */