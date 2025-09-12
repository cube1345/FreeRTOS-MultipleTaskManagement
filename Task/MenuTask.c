/*************************************************************************************
  * 文件名称：菜单显示任务文件
  * 文件说明：Y轴16像素/项，单次显3项，PA8下移滚动，确认进入5个任务
  * 作    者：Cube
  * 注    意：
  * 链    接：
  * 更新日期：
  * GitHub： 
*************************************************************************************/ 
/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 * 
 *            佛祖保佑     永不宕机     永无BUG
 */

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MenuTask.h"
#include "OLED.h"  

typedef enum {
    MENU_CLOCK,    
    MENU_ALARM,    
    MENU_GAME,    
    MENU_TEMP,     
    MENU_GYRO,    
    MENU_COUNT 
} MenuItemType;

typedef enum {
    KEY_EVENT_DOWN = 1,  
    KEY_EVENT_ENTER = 2 
} KeyEventType;

extern QueueHandle_t KeyQueue;               // 按键事件队列
extern volatile uint8_t menuInterruptEnabled;// 菜单中断使能开关

extern TaskHandle_t xClockTaskHandle;    // 时钟任务句柄
extern TaskHandle_t xAlarmTaskHandle;    // 闹钟任务句柄
extern TaskHandle_t xGameTaskHandle;     // 游戏任务句柄
extern TaskHandle_t xTempTaskHandle;     // 测温任务句柄
extern TaskHandle_t xGyroTaskHandle;     // 陀螺仪任务句柄

static char* g_menuNames[MENU_COUNT] = {
    "Clock",    
    "Alarm",    
    "Game",     
    "Temp",     
    "Gyro"      
};

static void ShowCurrentMenu(uint8_t selectedIndex, uint8_t displayOffset) {
    OLED_Clear(); 
    OLED_ShowString(0, 0, "Menu", OLED_8X16);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t menuIdx = displayOffset + i;  // 当前要显示的任务索引
        if (menuIdx >= MENU_COUNT) break;   
        // 选中项前加">", 未选中加空格，Y轴坐标=16 + i*16（固定16像素间隔）
        if (menuIdx == selectedIndex) {
            OLED_ShowString(0, 16 + i*16, ">", OLED_8X16);  // 选中标记
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);  // 任务名称
        } else {
            OLED_ShowString(0, 16 + i*16, " ", OLED_8X16);  // 未选中留空
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);
        }
    }
    OLED_Update();
}

static uint8_t CalcDisplayOffset(uint8_t selectedIndex) {
    uint8_t offset = 0;

    if (selectedIndex >= 2 && selectedIndex <= 4) {
        offset = selectedIndex - 2; 
    }
    return offset;
}


static void EnterTargetTask(uint8_t selectedIndex) {
    menuInterruptEnabled = 0;
    
    switch (selectedIndex) {
        case MENU_CLOCK:
            vTaskResume(xClockTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_ALARM:
            vTaskResume(xAlarmTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_GAME:
            vTaskResume(xGameTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_TEMP:
            vTaskResume(xTempTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_GYRO:
            vTaskResume(xGyroTaskHandle);
            vTaskSuspend(NULL);
            break;
        default:
            break;
    }
}


/* 
 * -----------------------------------------菜单任务入口------------------------------------------
 * 函 数 名：MenuTask
 * 说    明：1. PA8按键：选中项下移+滚动显示；2. 确认按键：进入5个任务；3. 任务返回后恢复菜单
 * 核心逻辑：selectedIndex（选中项索引）+ displayOffset（显示偏移量）控制UI滚动
 * 参    数: pvParameters - 任务创建时传入的参数（此处未使用）
 * 返 回 值：无（FreeRTOS任务必须死循环，不返回）
 * 注    意：需在中断文件中定义KeyQueue和menuInterruptEnabled
 **/
void MenuTask(void *pvParameters) 
{
    uint8_t selectedIndex = MENU_CLOCK;  // 初始选中第0项（Clock）
    uint8_t displayOffset = 0;           // 初始显示偏移0（显示0-2项）
    KeyEventType keyEvent;               // 接收按键事件（1=下移，2=确认）
    
    if (KeyQueue == NULL) {
        KeyQueue = xQueueCreate(5, sizeof(KeyEventType));
        configASSERT(KeyQueue != NULL);  
    }

    menuInterruptEnabled = 1;
    ShowCurrentMenu(selectedIndex, displayOffset);
    for (;;) {
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                case KEY_EVENT_DOWN:
                    selectedIndex++;
                    if (selectedIndex >= MENU_COUNT) {
                        selectedIndex = 0; 
                    }
                    displayOffset = CalcDisplayOffset(selectedIndex);
                    ShowCurrentMenu(selectedIndex, displayOffset);
                    break;
                case KEY_EVENT_ENTER:
                    EnterTargetTask(selectedIndex);
                    displayOffset = CalcDisplayOffset(selectedIndex);  // 重新计算偏移
                    ShowCurrentMenu(selectedIndex, displayOffset);    // 刷新菜单
                    menuInterruptEnabled = 1;                         // 重新允许中断
                    break;
                
                default:
                    break;
            }
        }
    }
}