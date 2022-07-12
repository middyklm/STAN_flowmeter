/*
 * settings.h
 *
 * Created: 29/03/2022 11:27:43 AM
 *  Author: Middy Khong
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#define SETTINGSIZE 10

typedef union {
	struct {		
		uint16_t volumeSample;		// volume in ml
		uint8_t	newVariable;		
	};
	
	uint16_t asArray[SETTINGSIZE+1];
}settings_Union;


settings_Union settings_Load();
void settings_Save(settings_Union input);


#endif /* SETTINGS_H_ */