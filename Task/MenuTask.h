/* MenuTask.h - 菜单任务头文件（仅声明对外暴露的内容） */
#ifndef MENU_TASK_H
#define MENU_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "Assests.h"  // 声明枚举类型

// 对外暴露的函数声明
void MenuTask(void *pvParameters);

// 全局变量声明（供其他任务引用，如TimeTask）
extern QueueHandle_t KeyQueue;
extern volatile uint8_t menuInterruptEnabled;
extern volatile uint8_t enterInterruptEnabled;
extern volatile uint8_t exitInterruptEnabled;
extern uint8_t selectedIndex;            // 全局选中项索引
extern TaskHandle_t xMenuTaskHandle;     // 菜单任务句柄

#endif // MENU_TASK_H