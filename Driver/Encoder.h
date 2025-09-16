#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f1xx_hal.h"

// 编码器方向定义
typedef enum {
    ENCODER_DIR_NONE = 0,
    ENCODER_DIR_CW   = 1,  // 顺时针
    ENCODER_DIR_CCW  = 2   // 逆时针
} Encoder_DirTypeDef;

// 编码器结构体
typedef struct {
    GPIO_TypeDef* A_GPIO_Port;
    uint16_t A_Pin;
    GPIO_TypeDef* B_GPIO_Port;
    uint16_t B_Pin;
    GPIO_TypeDef* C_GPIO_Port;
    uint16_t C_Pin;
    
    uint8_t last_state;     // 上一次状态
    int32_t count;          // 计数
    Encoder_DirTypeDef dir; // 方向
} Encoder_HandleTypeDef;

// 函数声明
void Encoder_Init(Encoder_HandleTypeDef *encoder);
void Encoder_Update(Encoder_HandleTypeDef *encoder);
int32_t Encoder_GetCount(Encoder_HandleTypeDef *encoder);
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef *encoder);
void Encoder_ResetCount(Encoder_HandleTypeDef *encoder);

#endif /* ENCODER_H */

