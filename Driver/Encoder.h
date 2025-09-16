#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f1xx_hal.h"

// ������������
typedef enum {
    ENCODER_DIR_NONE = 0,
    ENCODER_DIR_CW   = 1,  // ˳ʱ��
    ENCODER_DIR_CCW  = 2   // ��ʱ��
} Encoder_DirTypeDef;

// �������ṹ��
typedef struct {
    GPIO_TypeDef* A_GPIO_Port;
    uint16_t A_Pin;
    GPIO_TypeDef* B_GPIO_Port;
    uint16_t B_Pin;
    GPIO_TypeDef* C_GPIO_Port;
    uint16_t C_Pin;
    
    uint8_t last_state;     // ��һ��״̬
    int32_t count;          // ����
    Encoder_DirTypeDef dir; // ����
} Encoder_HandleTypeDef;

// ��������
void Encoder_Init(Encoder_HandleTypeDef *encoder);
void Encoder_Update(Encoder_HandleTypeDef *encoder);
int32_t Encoder_GetCount(Encoder_HandleTypeDef *encoder);
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef *encoder);
void Encoder_ResetCount(Encoder_HandleTypeDef *encoder);

#endif /* ENCODER_H */

