#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"

#include "stm32f1xx_hal.h"

// ����������ö��
typedef enum {
    ENCODER_DIR_NONE,    // �޷���
    ENCODER_DIR_CW,      // ˳ʱ��
    ENCODER_DIR_CCW      // ��ʱ��
} Encoder_DirTypeDef;

// ����״̬ö��
typedef enum {
    ENCODER_KEY_RELEASED,  // �����ͷ�
    ENCODER_KEY_PRESSED,   // ��������
    ENCODER_KEY_CLICKED    // ��������(���²��ͷ�)
} Encoder_KeyTypeDef;

// �������ṹ��
typedef struct {
    // ����������
    GPIO_TypeDef* A_Port;
    uint16_t A_Pin;
    GPIO_TypeDef* B_Port;
    uint16_t B_Pin;
    GPIO_TypeDef* KEY_Port;
    uint16_t KEY_Pin;
    
    // ������״̬
    int32_t count;               // ����
    Encoder_DirTypeDef dir;      // ����
    uint8_t last_state;          // ��һ��״̬
    
    // ����״̬
    Encoder_KeyTypeDef key_state;      // ��ǰ����״̬
    uint8_t key_press_flag;            // �������±�־
    uint32_t key_press_time;           // ��������ʱ��(ms)
    uint32_t key_last_change_time;     // �������һ��״̬�仯ʱ��(ms)
    uint8_t key_debounce_count;        // ��������
} Encoder_HandleTypeDef;

// ��������
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

