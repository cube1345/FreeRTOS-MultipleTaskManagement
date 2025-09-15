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
#include "OLED.h"  
#include "TimeTask.h"

extern TaskHandle_t xMenuTaskHandle;
extern TaskHandle_t xClockTaskHandle;
extern TaskHandle_t xAlarmTaskHandle;
extern TaskHandle_t xGameTaskHandle;
extern TaskHandle_t xTempTaskHandle;
extern TaskHandle_t xGyroTaskHandle;
extern TaskHandle_t xSettingTaskHandle;

extern QueueHandle_t KeyQueue;
extern volatile uint8_t menuInterruptEnabled;
extern volatile uint8_t enterInterruptEnabled;
extern volatile uint8_t exitInterruptEnabled;
extern volatile uint8_t g_isInMenuTask;
extern TaskHandle_t xMenuTaskHandle;
extern uint8_t selectedIndex;

void ClockTask(void *pvParameters)
{
	KeyEventType keyEvent; 
	menuInterruptEnabled = 0;
	enterInterruptEnabled = 0;
	exitInterruptEnabled = 1;
	g_isInMenuTask = 0;
	
	for(;;)
	{
		OLED_Clear();
		OLED_ShowString(0,0,"Alarm List",OLED_8X16);
		OLED_Update();
		if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS)  if (keyEvent == KEY_EVENT_EXIT) ExitToMenuTask();
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	
}

