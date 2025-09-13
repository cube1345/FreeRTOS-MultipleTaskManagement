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

#include "OLED.h"  
#include "rtc.h"
#include "Assests.h"
#include "MenuTask.h" 

extern QueueHandle_t KeyQueue;
extern volatile uint8_t menuInterruptEnabled;
extern volatile uint8_t enterInterruptEnabled;
extern volatile uint8_t exitInterruptEnabled;
extern volatile uint8_t g_isInMenuTask;
extern TaskHandle_t xMenuTaskHandle;
extern uint8_t selectedIndex;            

static void ExitToMenuTask(void) {
    menuInterruptEnabled = 1;    // �˵������ж�ʹ��
    enterInterruptEnabled = 1;   // ȷ���ж�ʹ��
    exitInterruptEnabled = 0;    // �˳��жϽ���
    g_isInMenuTask = 1;          // ��ǣ��ص��˵�����
    
    vTaskResume(xMenuTaskHandle);  // ���Ѳ˵�����
    vTaskSuspend(NULL);            // ����ǰʱ����������
}

void TimeTask(void *pvParameters) {
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;
    char timeStr[9], dateStr[11];
    KeyEventType keyEvent; 
    menuInterruptEnabled = 0;
    enterInterruptEnabled = 0;
    exitInterruptEnabled = 1;
    g_isInMenuTask = 0;
    for (;;) {
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); 
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  
        
        OLED_Clear();
        sprintf(dateStr, "20%02d-%02d-%02d", sDate.Year, sDate.Month, sDate.Date);
        sprintf(timeStr, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
        OLED_ShowString(0, 16, dateStr, OLED_8X16); 
        OLED_ShowString(0, 32, timeStr, OLED_8X16); 
        OLED_Update();
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            if (keyEvent == KEY_EVENT_EXIT) {
                ExitToMenuTask();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}