/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "queue.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

extern QueueHandle_t KeyQueue;
extern uint8_t g_isInMenuTask;
volatile uint8_t menuInterruptEnabled = 0;    // 锟剿碉拷锟斤拷锟斤拷锟叫讹拷使锟斤拷
volatile uint8_t enterInterruptEnabled = 0;   // 确锟斤拷/锟剿筹拷锟叫讹拷使锟杰ｏ拷统一锟斤拷锟斤拷PB12/PB13锟斤拷
volatile uint8_t exitInterruptEnabled = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles RTC global interrupt.
  */
void RTC_IRQHandler(void)
{
  /* USER CODE BEGIN RTC_IRQn 0 */

  /* USER CODE END RTC_IRQn 0 */
  HAL_RTCEx_RTCIRQHandler(&hrtc);
  /* USER CODE BEGIN RTC_IRQn 1 */

  /* USER CODE END RTC_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
  static uint32_t LastPressTime = 0; 
  uint32_t CurrentTime = HAL_GetTick();
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  
  KeyEventType Key_Event = KEY_EVENT_DOWN;                
  if (!menuInterruptEnabled || (CurrentTime - LastPressTime) < 200) {
    goto EXIT_IRQ_HANDLER;
  }
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_RESET) {
    if (KeyQueue != NULL) {
      xQueueSendFromISR(KeyQueue, &Key_Event, &xHigherPriorityTaskWoken);
    }
    LastPressTime = CurrentTime;
  }

EXIT_IRQ_HANDLER:
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
  
 
  if (xHigherPriorityTaskWoken == pdTRUE) { 
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
  }
  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  static uint32_t LastPressTime = 0;  // 上次按键时间（用于消抖）
  uint32_t CurrentTime = HAL_GetTick();  // 当前时间
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;  // 任务唤醒标志
  KeyEventType Key_Event;  // 按键事件类型（使用枚举定义）

  // 处理PB12（ENTER键）按下事件
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) {
    // 检查：ENTER中断使能 且 按键间隔大于50ms（消抖）
    if (enterInterruptEnabled && (CurrentTime - LastPressTime) >= 50) {
      Key_Event = KEY_EVENT_ENTER;  // 定义为ENTER事件
      // 若队列有效，则发送事件到按键队列
      if (KeyQueue != NULL) {
        xQueueSendFromISR(KeyQueue, &Key_Event, &xHigherPriorityTaskWoken);
      }
      LastPressTime = CurrentTime;  // 更新上次按键时间
    }
  }
  // 处理PB13（EXIT键）按下事件
  else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) {
    // 检查：EXIT中断使能 且 按键间隔大于50ms（消抖）
    if (exitInterruptEnabled && (CurrentTime - LastPressTime) >= 50) {
      Key_Event = KEY_EVENT_EXIT;  // 定义为EXIT事件
      // 若队列有效，则发送事件到按键队列
      if (KeyQueue != NULL) {
        xQueueSendFromISR(KeyQueue, &Key_Event, &xHigherPriorityTaskWoken);
      }
      LastPressTime = CurrentTime;  // 更新上次按键时间
    }
  }

EXIT_IRQ_HANDLER:
  /* USER CODE END EXTI15_10_IRQn 0 */
  // 清除EXTI中断标志（必须调用，否则会重复触发中断）
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  // 若有高优先级任务被唤醒，触发任务切换
  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
  /* USER CODE END EXTI15_10_IRQn 1 */
}


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
