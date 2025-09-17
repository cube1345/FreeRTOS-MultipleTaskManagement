#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"

#include "stm32f1xx_hal.h"

// 编码器方向枚举
typedef enum {
    ENCODER_DIR_NONE,    // 无方向
    ENCODER_DIR_CW,      // 顺时针
    ENCODER_DIR_CCW      // 逆时针
} Encoder_DirTypeDef;

// 按键状态枚举
typedef enum {
    ENCODER_KEY_RELEASED,  // 按键释放
    ENCODER_KEY_PRESSED,   // 按键按下
    ENCODER_KEY_CLICKED    // 按键单击(按下并释放)
} Encoder_KeyTypeDef;

// 编码器结构体
typedef struct {
    // 编码器引脚
    GPIO_TypeDef* A_Port;
    uint16_t A_Pin;
    GPIO_TypeDef* B_Port;
    uint16_t B_Pin;
    GPIO_TypeDef* KEY_Port;
    uint16_t KEY_Pin;
    
    // 编码器状态
    int32_t count;               // 计数
    Encoder_DirTypeDef dir;      // 方向
    uint8_t last_state;          // 上一次状态
    
    // 按键状态
    Encoder_KeyTypeDef key_state;      // 当前按键状态
    uint8_t key_press_flag;            // 按键按下标志
    uint32_t key_press_time;           // 按键按下时间(ms)
    uint32_t key_last_change_time;     // 按键最后一次状态变化时间(ms)
    uint8_t key_debounce_count;        // 防抖计数
} Encoder_HandleTypeDef;

// 函数声明
void Encoder_Init(Encoder_HandleTypeDef* hencoder, 
                 GPIO_TypeDef* A_Port, uint16_t A_Pin,
                 GPIO_TypeDef* B_Port, uint16_t B_Pin,
                 GPIO_TypeDef* KEY_Port, uint16_t KEY_Pin);
void Encoder_Update(Encoder_HandleTypeDef* hencoder);
int32_t Encoder_GetCount(Encoder_HandleTypeDef* hencoder);
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef* hencoder);
Encoder_KeyTypeDef Encoder_GetKeyState(Encoder_HandleTypeDef* hencoder);
uint8_t Encoder_IsKeyPressed(Encoder_HandleTypeDef* hencoder);
uint8_t Encoder_IsKeyClicked(Encoder_HandleTypeDef* hencoder);
uint32_t Encoder_GetKeyPressTime(Encoder_HandleTypeDef* hencoder);
#endif /* ENCODER_H */

