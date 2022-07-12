/*
 * wakeupINT2.h
 *
 * Created: 6/02/2019 4:02:50 PM
 *  Author: Middy Khong
 */ 


#ifndef WAKEUPINT2_H_
#define WAKEUPINT2_H_

#include <stdbool.h>

bool PCINT8flag;

void INT2_init(void);
void USART1_disable(void);
void USART1_re_enable(void);

void pin_change_1_init(void);

#endif /* WAKEUP_H_ */