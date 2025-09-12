/*************************************************************************************
  * �ļ����ƣ��˵���ʾ�����ļ���5��������棩
  * �ļ�˵����Y��16����/�������3�PA8���ƹ�����ȷ�Ͻ���5������
  * ��    �ߣ�Cube
  * ע    �⣺����128x64 OLED��������ж��ļ���������ʹ��
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

// 2. �����¼����壨��PA8�ж϶��룩
typedef enum {
    KEY_EVENT_DOWN = 1,  // PA8������ѡ��������+����
    KEY_EVENT_ENTER = 2  // ȷ�ϰ���������ѡ�����������䰴���жϣ���PA9��
} KeyEventType;

// 3. ȫ�ֱ��������������ж��ļ��ж��壩
extern QueueHandle_t KeyQueue;               // �����¼�����
extern volatile uint8_t menuInterruptEnabled;// �˵��ж�ʹ�ܿ���

// 4. 5������ľ�����������񴴽������壬�˴����������л���
extern TaskHandle_t xClockTaskHandle;    // ʱ��������
extern TaskHandle_t xAlarmTaskHandle;    // ����������
extern TaskHandle_t xGameTaskHandle;     // ��Ϸ������
extern TaskHandle_t xTempTaskHandle;     // ����������
extern TaskHandle_t xGyroTaskHandle;     // ������������

// 5. �˵��������飨��MenuItemType������Ӧ��Ӣ����ʾ��
static char* g_menuNames[MENU_COUNT] = {
    "Clock",    // MENU_CLOCK
    "Alarm",    // MENU_ALARM
    "Game",     // MENU_GAME
    "Temp",     // MENU_TEMP
    "Gyro"      // MENU_GYRO
};


// 6. ��ʾ��ǰ�˵������ģ���ƫ������ʾ3�Y��̶�16���أ�
static void ShowCurrentMenu(uint8_t selectedIndex, uint8_t displayOffset) {
    OLED_Clear();  // �����������Ӱ��
    
    // ��1�У��̶���ʾ���⣨Y=0��16���ظ߶ȣ�
    OLED_ShowString(0, 0, "Menu", OLED_8X16);
    
    // ��2-4�У���ʾ��ǰƫ������Ӧ��3������Y=16��32��48�� each 16px��
    for (uint8_t i = 0; i < 3; i++) {  // ������3��
        uint8_t menuIdx = displayOffset + i;  // ��ǰҪ��ʾ����������
        if (menuIdx >= MENU_COUNT) break;     // ��������������ֹͣ�������һҳ���ܴ�����
        
        // ѡ����ǰ��">", δѡ�мӿո�Y������=16 + i*16���̶�16���ؼ����
        if (menuIdx == selectedIndex) {
            OLED_ShowString(0, 16 + i*16, ">", OLED_8X16);  // ѡ�б��
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);  // ��������
        } else {
            OLED_ShowString(0, 16 + i*16, " ", OLED_8X16);  // δѡ������
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);
        }
    }
    
    OLED_Update();  // ˢ����ʾ��������ã����򲻸�����Ļ��
}


// 7. ������ʾƫ���������ݵ�ǰѡ���ȷ��ѡ��������ʾ�����ڣ�
static uint8_t CalcDisplayOffset(uint8_t selectedIndex) {
    uint8_t offset = 0;
    // ѡ����0-2 �� ƫ��0����ʾ0-2��
    // ѡ����1-3 �� ƫ��1����ʾ1-3��
    // ѡ����2-4 �� ƫ��2����ʾ2-4��
    if (selectedIndex >= 2 && selectedIndex <= 4) {
        offset = selectedIndex - 2;  // ѡ����2��ƫ��0������ѡ����2ʱ��ƫ��0��ʾ0-2��ƫ��1��ʾ1-3��ƫ��2��ʾ2-4���˴��߼�������ѡ����>=2ʱ��ƫ��=selectedIndex-2 �� ѡ��2��0��3��1��4��2
    } else if (selectedIndex == 1) {
        offset = 1;  // ѡ��1ʱ��ƫ��1��ʾ1-3��ȷ������ʱ�ܹ�����
    }
    // ѡ��0ʱƫ��0��Ĭ�ϣ�
    return offset;
}


// 8. ����Ŀ��������ͣ�˵����񣬻ָ�ѡ�е�����
static void EnterTargetTask(uint8_t selectedIndex) {
    // 1. �Ƚ��ò˵��жϣ����������л�ʱ�󴥷�������
    menuInterruptEnabled = 0;
    
    // 2. ��ͣ�˵����񣬻ָ�Ŀ������FreeRTOS�����л���׼�÷���
    switch (selectedIndex) {
        case MENU_CLOCK:
            vTaskResume(xClockTaskHandle);
            vTaskSuspend(NULL);  // ��ͣ��ǰ�˵�����NULL=��������
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
    
    // ����1����ʼ��OLED����ִ��һ�Σ������ظ���ʼ����
    OLED_Init();
    
    // ����2�������������У���δ�������ط����������5�㹻���水���¼���
    if (KeyQueue == NULL) {
        KeyQueue = xQueueCreate(5, sizeof(KeyEventType));
        configASSERT(KeyQueue != NULL);  // ���ԣ����д���ʧ������ʾ�������ã�
    }
    
    // ����3������PA8�����жϣ��˵���������ʱ��ʹ�ܣ��������������󴥷���
    menuInterruptEnabled = 1;
    
    // ����4����ʼ��ʾ�˵���ƫ��0��ѡ��Clock��
    ShowCurrentMenu(selectedIndex, displayOffset);
    
    // ����5���˵���ѭ����FreeRTOS���������ѭ���������˳���
    for (;;) {
        // �ȴ������¼�����ʱ100ms�������Ӧ�ٶȺ�CPU��ռ�ã�
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                // -------------- PA8������ѡ��������+���� --------------
                case KEY_EVENT_DOWN:
                    // 1. ѡ�������ƣ�ѭ�������һ�����һ�
                    selectedIndex++;
                    if (selectedIndex >= MENU_COUNT) {
                        selectedIndex = 0;  // 4��Gyro����0��Clock��
                    }
                    
                    // 2. �����µ���ʾƫ������ȷ��ѡ�����ڵ�ǰ��ʾ�����ڣ�
                    displayOffset = CalcDisplayOffset(selectedIndex);
                    
                    // 3. ������ʾ�����µ�ѡ�����ƫ����ˢ��UI��
                    ShowCurrentMenu(selectedIndex, displayOffset);
                    break;
                
                // -------------- ȷ�ϰ���������ѡ������ --------------
                case KEY_EVENT_ENTER:
                    // �л���Ŀ���������񷵻غ�Ż�ִ�к������룩
                    EnterTargetTask(selectedIndex);
                    
                    // Ŀ�����񷵻غ󣨲˵����񱻻ָ��������³�ʼ����ʾ
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