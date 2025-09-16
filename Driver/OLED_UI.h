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

// ��ʾ�궨��
#define FONT_SIZE       OLED_8X16
#define ALARM_ROW_HEIGHT 18
#define BLOCK_SIZE      4
#define BLOCK_SPACING   3

#define TITLE_STR       "Alarm List"
#define TITLE_X         0
#define TITLE_Y         0
#define TIME_START_X    5
#define BLOCK_START_X   95
#define PAGE_TIP_X      90
#define PAGE_TIP_Y      5

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
