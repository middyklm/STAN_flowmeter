/*
 * settings.c
 *
 * Created: 29/03/2022 11:29:08 AM
 *  Author: Middy Khong 
 */ 

#include <avr/eeprom.h>
#include "settings.h"

settings_Union settings_Load()
{
	settings_Union returnMe;
	for (unsigned int i = 0; i < SETTINGSIZE; i++)
	{
		returnMe.asArray[i] = eeprom_read_word((const uint16_t *)(i*2));
	}
	
	return returnMe;
}

void settings_Save(settings_Union input)
{
	for (unsigned char i = 0; i < SETTINGSIZE; i++)
	{
		eeprom_update_word((uint16_t *)(i*2), input.asArray[i]);
	}
}
