/*************************************************************************************
  * �ļ����ƣ��˵���ʾ�����ļ�
  * �ļ�˵����Y��16����/�������3�PA8���ƹ�����ȷ�Ͻ���5������
  * ��    �ߣ�Cube
  * ע    �⣺
  * ��    �ӣ�
  * �������ڣ�
  * GitHub�� 
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
 *            ���汣��     ����崻�     ����BUG
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

extern QueueHandle_t KeyQueue;               // �����¼�����
extern volatile uint8_t menuInterruptEnabled;// �˵��ж�ʹ�ܿ���

extern TaskHandle_t xClockTaskHandle;    // ʱ��������
extern TaskHandle_t xAlarmTaskHandle;    // ����������
extern TaskHandle_t xGameTaskHandle;     // ��Ϸ������
extern TaskHandle_t xTempTaskHandle;     // ����������
extern TaskHandle_t xGyroTaskHandle;     // ������������

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
        uint8_t menuIdx = displayOffset + i;  // ��ǰҪ��ʾ����������
        if (menuIdx >= MENU_COUNT) break;   
        // ѡ����ǰ��">", δѡ�мӿո�Y������=16 + i*16���̶�16���ؼ����
        if (menuIdx == selectedIndex) {
            OLED_ShowString(0, 16 + i*16, ">", OLED_8X16);  // ѡ�б��
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);  // ��������
        } else {
            OLED_ShowString(0, 16 + i*16, " ", OLED_8X16);  // δѡ������
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
 * -----------------------------------------�˵��������------------------------------------------
 * �� �� ����MenuTask
 * ˵    ����1. PA8������ѡ��������+������ʾ��2. ȷ�ϰ���������5������3. ���񷵻غ�ָ��˵�
 * �����߼���selectedIndex��ѡ����������+ displayOffset����ʾƫ����������UI����
 * ��    ��: pvParameters - ���񴴽�ʱ����Ĳ������˴�δʹ�ã�
 * �� �� ֵ���ޣ�FreeRTOS���������ѭ���������أ�
 * ע    �⣺�����ж��ļ��ж���KeyQueue��menuInterruptEnabled
 **/
void MenuTask(void *pvParameters) 
{
    uint8_t selectedIndex = MENU_CLOCK;  // ��ʼѡ�е�0�Clock��
    uint8_t displayOffset = 0;           // ��ʼ��ʾƫ��0����ʾ0-2�
    KeyEventType keyEvent;               // ���հ����¼���1=���ƣ�2=ȷ�ϣ�
    
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
                    displayOffset = CalcDisplayOffset(selectedIndex);  // ���¼���ƫ��
                    ShowCurrentMenu(selectedIndex, displayOffset);    // ˢ�²˵�
                    menuInterruptEnabled = 1;                         // ���������ж�
                    break;
                
                default:
                    break;
            }
        }
    }
}