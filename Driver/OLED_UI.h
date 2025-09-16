#ifndef __OLED_UI_H
#define __OLED_UI_H

#include "OLED.h"
#include "gpio.h"

// ���ӽṹ��
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t repeat[7];
} AlarmTypeDef;


#define KEY_DELAY       20
#define ALARM_PER_PAGE  3

// ȫ�ֱ�������
extern AlarmTypeDef g_AlarmList[];
extern uint8_t g_CurrentPage;
extern uint8_t g_SelectedIndex; 

// ��������
void OLED_DrawWeekBlock(uint16_t x, uint16_t y, uint8_t isEnable, uint8_t invert);
uint8_t GetAlarmCount(void);
void OLED_ShowSingleAlarm(uint8_t row, AlarmTypeDef *pAlarm, uint8_t isSelected);  // ��ѡ�в���
void OLED_ShowAlarmPage(void);

#endif /* __OLED_UI_H */
