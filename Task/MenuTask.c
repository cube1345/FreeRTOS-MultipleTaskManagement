/**********************************************************************************
 * �ļ����ƣ�
 * �ļ�˵����
 * �������裺
 * ��ֲ��֪��
 * ������֪��
-------------
-------------
-------------
-------------
-------------
 * ��   ����
 * ��   �ߣ�Cube
 * �������ڣ�
 * �������
 * GitHub��https://github.com/cube1345
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
 *            ���汣��     ����崻�     ����BUG
 */
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"

#include "MenuTask.h"
#include "Assests.h"  // ����ö�٣�MenuItemType/KeyEventType��
#include "OLED.h"  

typedef enum {
    MENU_CLOCK,        // ʱ��
    MENU_ALARM,        // ����
    MENU_GAME,         // ��Ϸ
    MENU_TEMP,         // ����
    MENU_GYRO,         // ������
		MENU_SETTING,
    MENU_COUNT         // �˵�����
} MenuItemType;
// -------------------------- ȫ�ֱ������壨�޸���ȥ��extern��ʵ�ʷ����ڴ棩 --------------------------
QueueHandle_t KeyQueue = NULL;               // �����¼����У�ȫ��Ψһ�����˴�������
extern volatile uint8_t menuInterruptEnabled;   // �˵������ж�ʹ�ܿ���
extern volatile uint8_t enterInterruptEnabled;  // ȷ���ж�ʹ�ܿ���
extern volatile uint8_t exitInterruptEnabled;   // �˳��ж�ʹ�ܿ��أ���ʼ������
uint8_t selectedIndex = MENU_CLOCK;          // ȫ��ѡ���ȥ��static����TimeTask���ã�
uint8_t g_isInMenuTask = 0;

// �������ⲿ�����������񴴽������壬��main.c��
extern TaskHandle_t xMenuTaskHandle;
extern TaskHandle_t xClockTaskHandle;
extern TaskHandle_t xAlarmTaskHandle;
extern TaskHandle_t xGameTaskHandle;
extern TaskHandle_t xTempTaskHandle;
extern TaskHandle_t xGyroTaskHandle;
extern TaskHandle_t xSettingTaskHandle;

// �˵��������飨��MenuTask�ڲ�ʹ�ã�static����������
static char* g_menuNames[MENU_COUNT] = {
    "Clock", "Alarm", "Game", "Temp", "Gyro", "Setting"
};


// ��ʾ��ǰ�˵�����̬���������ڲ�ʹ�ã�
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


// ������ʾƫ��������̬���������ڲ�ʹ�ã�
static uint8_t CalcDisplayOffset(uint8_t selectedIndex) {
    uint8_t offset = 0;
    if (selectedIndex >= 2 && selectedIndex <= 5) {
        offset = selectedIndex - 2; 
    }
    return offset;
}


// ����Ŀ�����񣨾�̬���������ڲ�ʹ�ã�
static void EnterTargetTask(uint8_t selectedIndex) {
    menuInterruptEnabled = 0;    // ���ò˵������ж�
    enterInterruptEnabled = 0;   // ����ȷ���ж�
    exitInterruptEnabled = 1;    // �����˳��жϣ���������ʹ�ã�
    g_isInMenuTask = 0;          // ��ǣ����ڲ˵�����
    
    // ���Ѷ�Ӧ�����񣬹���˵�����
    switch (selectedIndex) {
        case MENU_CLOCK: vTaskResume(xClockTaskHandle); break;
        case MENU_ALARM: vTaskResume(xAlarmTaskHandle); break;
        case MENU_GAME: vTaskResume(xGameTaskHandle); break;
        case MENU_TEMP: vTaskResume(xTempTaskHandle); break;
        case MENU_GYRO: vTaskResume(xGyroTaskHandle); break;
        case MENU_SETTING: vTaskResume(xSettingTaskHandle); break;
        default: break;
    }
    vTaskSuspend(NULL);  // ����˵����񣬵ȴ������񷵻�

    // �����񷵻غ󣺻ָ��˵�״̬
    g_isInMenuTask = 1;        
    menuInterruptEnabled = 1;   
    enterInterruptEnabled = 1;  
    exitInterruptEnabled = 0;   
    ShowCurrentMenu(selectedIndex, CalcDisplayOffset(selectedIndex));
}


// �˵�������ڣ����Ⱪ¶��������MenuTask.h�У�
void MenuTask(void *pvParameters) 
{
    uint8_t displayOffset = 0;
    KeyEventType keyEvent;  // ����Assests.h�е�ö��
    
    // ����MenuTask�д���һ��KeyQueue��ȫ��Ψһ��
    if (KeyQueue == NULL) {
        KeyQueue = xQueueCreate(5, sizeof(KeyEventType));
        configASSERT(KeyQueue != NULL);  // ���ԣ����д���ʧ���򱨴�
    }

    // ��ʼ��״̬
    menuInterruptEnabled = 1;
    enterInterruptEnabled = 1;
    exitInterruptEnabled = 0;
    g_isInMenuTask = 1;
    ShowCurrentMenu(selectedIndex, displayOffset);
    
    // ��ѭ�����������¼�
    for (;;) {
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                case KEY_EVENT_DOWN:
                    // ѡ�������ƣ�ѭ����
                    selectedIndex = (selectedIndex + 1) % MENU_COUNT;
                    displayOffset = CalcDisplayOffset(selectedIndex);
                    ShowCurrentMenu(selectedIndex, displayOffset);
                    break;
                case KEY_EVENT_ENTER:
                    EnterTargetTask(selectedIndex);  // ����������
                    break;
                default: break;  // �����˳��¼����˵��в�����
            }
        }
    }
}