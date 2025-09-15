/**********************************************************************************
 * 文件名称：
 * 文件说明：
 * 操作外设：
 * 移植须知：
 * 接线须知：
-------------
-------------
-------------
-------------
-------------
 * 其   他：
 * 作   者：Cube
 * 更新日期：
 * 更新事项：
 * GitHub：https://github.com/cube1345
 ***********************************************************************************/
/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (  -_-  )
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\     |// '.
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
 * 
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *            佛祖保佑     永不宕机     永无BUG
 */
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"

#include "MenuTask.h"
#include "Assests.h"  // 包含枚举（MenuItemType/KeyEventType）
#include "OLED.h"  

typedef enum {
    MENU_CLOCK,        // 时钟
    MENU_ALARM,        // 闹钟
    MENU_GAME,         // 游戏
    MENU_TEMP,         // 测温
    MENU_GYRO,         // 陀螺仪
		MENU_SETTING,      // 设置界面
    MENU_COUNT         // 菜单总数
} MenuItemType;
QueueHandle_t KeyQueue = NULL;                  // 按键事件队列
extern volatile uint8_t menuInterruptEnabled;   // 菜单下移中断使能开关
extern volatile uint8_t enterInterruptEnabled;  // 确认中断使能
extern volatile uint8_t exitInterruptEnabled;   // 退出中断使能
uint8_t selectedIndex = MENU_CLOCK;             // 全局选中项
uint8_t g_isInMenuTask = 0;

extern TaskHandle_t xMenuTaskHandle;
extern TaskHandle_t xClockTaskHandle;
extern TaskHandle_t xAlarmTaskHandle;
extern TaskHandle_t xGameTaskHandle;
extern TaskHandle_t xTempTaskHandle;
extern TaskHandle_t xGyroTaskHandle;
extern TaskHandle_t xSettingTaskHandle;

static char* g_menuNames[MENU_COUNT] = {
    "Clock", "Alarm", "Game", "Temp", "Gyro", "Setting"
};


static void ShowCurrentMenu(uint8_t selectedIndex, uint8_t displayOffset) {
    OLED_Clear(); 
    OLED_ShowString(0, 0, "Menu", OLED_8X16);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t menuIdx = displayOffset + i;
        if (menuIdx >= MENU_COUNT) break;   
        if (menuIdx == selectedIndex) {
            OLED_ShowString(0, 16 + i*16, ">", OLED_8X16);
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);
        } else {
            OLED_ShowString(0, 16 + i*16, " ", OLED_8X16);
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);
        }
    }
    OLED_Update();
}

static uint8_t CalcDisplayOffset(uint8_t selectedIndex) {
    uint8_t offset = 0;
    if (selectedIndex >= 2 && selectedIndex <= 5) {
        offset = selectedIndex - 2; 
    }
    return offset;
}

static void EnterTargetTask(uint8_t selectedIndex) {
    menuInterruptEnabled = 0;    // 禁用菜单下移中断
    enterInterruptEnabled = 0;   // 禁用确认中断
    exitInterruptEnabled = 1;    // 启用退出中断
    g_isInMenuTask = 0;         

    switch (selectedIndex) {
        case MENU_CLOCK: vTaskResume(xClockTaskHandle); break;
        case MENU_ALARM: 
				{
					vTaskResume(xAlarmTaskHandle); 
					vTaskResume(xClockTaskHandle);
					break;
				}
        case MENU_GAME: vTaskResume(xGameTaskHandle); break;
        case MENU_TEMP: vTaskResume(xTempTaskHandle); break;
        case MENU_GYRO: vTaskResume(xGyroTaskHandle); break;
        case MENU_SETTING: vTaskResume(xSettingTaskHandle); break;
        default: break;
    }
    vTaskSuspend(NULL);

    g_isInMenuTask = 1;        
    menuInterruptEnabled = 1;   
    enterInterruptEnabled = 1;  
    exitInterruptEnabled = 0;   
    ShowCurrentMenu(selectedIndex, CalcDisplayOffset(selectedIndex));
}


void MenuTask(void *pvParameters) 
{
    uint8_t displayOffset = 0;
    KeyEventType keyEvent;
    if (KeyQueue == NULL) {
        KeyQueue = xQueueCreate(5, sizeof(KeyEventType));
        configASSERT(KeyQueue != NULL);
    }

    menuInterruptEnabled = 1;
    enterInterruptEnabled = 1;
    exitInterruptEnabled = 0;
    g_isInMenuTask = 1;
    ShowCurrentMenu(selectedIndex, displayOffset);
    for (;;) {
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                case KEY_EVENT_DOWN:
                    selectedIndex = (selectedIndex + 1) % MENU_COUNT;
                    displayOffset = CalcDisplayOffset(selectedIndex);
                    ShowCurrentMenu(selectedIndex, displayOffset);
                    break;
                case KEY_EVENT_ENTER:
                    EnterTargetTask(selectedIndex);
                    break;
                default: break;
            }
        }
    }
}