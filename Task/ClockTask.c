#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"
#include "OLED.h"  
#include "TimeTask.h"
#include "OLED_UI.h"

// 外部任务句柄
extern TaskHandle_t xMenuTaskHandle;
extern TaskHandle_t xClockTaskHandle;
extern TaskHandle_t xAlarmTaskHandle;
extern TaskHandle_t xGameTaskHandle;
extern TaskHandle_t xTempTaskHandle;
extern TaskHandle_t xGyroTaskHandle;
extern TaskHandle_t xSettingTaskHandle;

// 外部队列和全局变量
extern QueueHandle_t KeyQueue;
extern volatile uint8_t menuInterruptEnabled;
extern volatile uint8_t enterInterruptEnabled;
extern volatile uint8_t exitInterruptEnabled;
extern volatile uint8_t g_isInMenuTask;
extern TaskHandle_t xMenuTaskHandle;

/**
 * 退出到菜单任务
 */
static void ExitToMenuTask(void) {
    g_isInMenuTask = 1;
    menuInterruptEnabled = 1;
    enterInterruptEnabled = 1;
    exitInterruptEnabled = 0;
    vTaskResume(xMenuTaskHandle);
    vTaskSuspend(xClockTaskHandle);
}

/**
 * 计算当前页码，确保选中项在可见页
 */
static void UpdateCurrentPage(void) {
    g_CurrentPage = g_SelectedIndex / ALARM_PER_PAGE;
}

/**
 * 闹钟列表任务（FreeRTOS任务）
 */
void ClockTask(void *pvParameters) {
    KeyEventType keyEvent; 
    menuInterruptEnabled = 1;
    enterInterruptEnabled = 0;
    exitInterruptEnabled = 1;
    g_isInMenuTask = 0;
    g_SelectedIndex = 0;
    g_CurrentPage = 0;
    OLED_ShowAlarmPage();
    
    for(;;) {
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                case KEY_EVENT_EXIT:
                    ExitToMenuTask();
                    break;
                case KEY_EVENT_DOWN:
                    if (g_SelectedIndex < GetAlarmCount() - 1) {
                        g_SelectedIndex++;
                    } else {
                        g_SelectedIndex = 0;
                    }
                    UpdateCurrentPage();  // 更新页码
                    OLED_ShowAlarmPage(); // 刷新显示
                    break;
              
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
    