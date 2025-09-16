#include "OLED.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include "OLED_UI.h"

// 宏定义
#define FONT_SIZE       OLED_8X16
#define ALARM_ROW_HEIGHT 16
#define BLOCK_SIZE      4
#define BLOCK_SPACING   3

#define TITLE_STR       "Alarm List"
#define TITLE_X         0
#define TITLE_Y         0
#define TIME_START_X    5
#define BLOCK_START_X   75
#define PAGE_TIP_X      90
#define PAGE_TIP_Y      0

#define KEY_DELAY       20

// 闹钟数据
AlarmTypeDef g_AlarmList[] = {
    {7, 30, 0, {0, 1, 1, 1, 1, 1, 0}},
    {8, 0, 0, {1, 0, 0, 0, 0, 0, 1}},
    {12, 30, 0, {1, 1, 1, 1, 1, 1, 1}},
    {18, 0, 0, {0, 1, 0, 1, 0, 1, 0}},
    {22, 30, 0, {1, 1, 0, 0, 0, 1, 1}},
    {6, 0, 0, {0, 1, 1, 1, 1, 1, 0}}
};

// 计算宏
#define ALARM_TOTAL     (sizeof(g_AlarmList) / sizeof(AlarmTypeDef))
#define ALARM_PER_PAGE  3
#define MAX_PAGE        ((ALARM_TOTAL - 1) / ALARM_PER_PAGE)

// 全局变量（与头文件声明对应）
uint8_t g_CurrentPage = 0;
uint8_t g_SelectedIndex = 0;

// 绘制星期方块（支持反相）
void OLED_DrawWeekBlock(uint16_t x, uint16_t y, uint8_t isEnable, uint8_t invert) {
    if (isEnable) {
        OLED_DrawRectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, OLED_FILLED);
    } else {
        OLED_DrawRectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, OLED_UNFILLED);
    }
    if (invert) {
        OLED_ReverseArea(x, y, BLOCK_SIZE, BLOCK_SIZE);
    }
}

// 获取闹钟总数
uint8_t GetAlarmCount(void) {
    return ALARM_TOTAL;
}

// 显示单条闹钟（支持选中反相）
void OLED_ShowSingleAlarm(uint8_t row, AlarmTypeDef *pAlarm, uint8_t isSelected) {
    uint16_t rowY = TITLE_Y + 15 + row * ALARM_ROW_HEIGHT; 
    uint16_t blockX = BLOCK_START_X;                    
    uint8_t invert = isSelected ? 1 : 0;
    OLED_ShowNum(TIME_START_X, rowY, pAlarm->hour, 2, FONT_SIZE);
    OLED_ShowChar(TIME_START_X + 16, rowY, ':', FONT_SIZE);
    OLED_ShowNum(TIME_START_X + 24, rowY, pAlarm->minute, 2, FONT_SIZE);
    OLED_ShowChar(TIME_START_X + 40, rowY, ':', FONT_SIZE);
    OLED_ShowNum(TIME_START_X + 48, rowY, pAlarm->second, 2, FONT_SIZE);  
    if (invert) {
        OLED_ReverseArea(TIME_START_X, rowY, 65, 16);  // 反相时间区域
    } 
    for (uint8_t i = 0; i < 7; i++) {
        OLED_DrawWeekBlock(blockX, rowY + 8, pAlarm->repeat[i], invert);
        blockX += BLOCK_SIZE + BLOCK_SPACING;
    }
}

// 显示当前页闹钟列表
void OLED_ShowAlarmPage(void) {
    uint8_t startIdx = g_CurrentPage * ALARM_PER_PAGE;
    char pageTip[10];
    OLED_Clear();
    OLED_ShowString(TITLE_X, TITLE_Y, TITLE_STR, FONT_SIZE);
    sprintf(pageTip, "%d/%d", g_CurrentPage + 1, MAX_PAGE + 1);
    OLED_ShowString(PAGE_TIP_X, PAGE_TIP_Y, pageTip, FONT_SIZE);
    for (uint8_t i = 0; i < ALARM_PER_PAGE; i++) {
        uint8_t alarmIdx = startIdx + i;
        if (alarmIdx >= ALARM_TOTAL) break;
        uint8_t isSelected = (alarmIdx == g_SelectedIndex) ? 1 : 0;
        OLED_ShowSingleAlarm(i, &g_AlarmList[alarmIdx], isSelected);
    }
    
    OLED_Update();
}
