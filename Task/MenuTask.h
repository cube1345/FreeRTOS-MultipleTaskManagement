/* MenuTask.h - �˵�����ͷ�ļ������������Ⱪ¶�����ݣ� */
#ifndef MENU_TASK_H
#define MENU_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "Assests.h"  // ����ö������

// ���Ⱪ¶�ĺ�������
void MenuTask(void *pvParameters);

// ȫ�ֱ����������������������ã���TimeTask��
extern QueueHandle_t KeyQueue;
extern volatile uint8_t menuInterruptEnabled;
extern volatile uint8_t enterInterruptEnabled;
extern volatile uint8_t exitInterruptEnabled;
extern uint8_t selectedIndex;            // ȫ��ѡ��������
extern TaskHandle_t xMenuTaskHandle;     // �˵�������

#endif // MENU_TASK_H