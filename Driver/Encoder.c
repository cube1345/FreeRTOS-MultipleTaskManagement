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
 
#include "Encoder.h"
#include "stm32f1xx_hal.h"

// �궨��
#define ENCODER_KEY_DEBOUNCE_THRESHOLD 5    // ������ֵ(����������)
#define ENCODER_KEY_CLICK_MAX_TIME 500      // �������ʱ��(ms)

// ��ʼ��������
void Encoder_Init(Encoder_HandleTypeDef* hencoder, 
                 GPIO_TypeDef* A_Port, uint16_t A_Pin,
                 GPIO_TypeDef* B_Port, uint16_t B_Pin,
                 GPIO_TypeDef* KEY_Port, uint16_t KEY_Pin) {
    // ����������Ϣ
    hencoder->A_Port = A_Port;
    hencoder->A_Pin = A_Pin;
    hencoder->B_Port = B_Port;
    hencoder->B_Pin = B_Pin;
    hencoder->KEY_Port = KEY_Port;
    hencoder->KEY_Pin = KEY_Pin;
    
    // ��ʼ��״̬
    hencoder->count = 0;
    hencoder->dir = ENCODER_DIR_NONE;
    hencoder->last_state = 0;
    
    // ��ȡ��ʼ״̬
    uint8_t a_phase = HAL_GPIO_ReadPin(hencoder->A_Port, hencoder->A_Pin);
    uint8_t b_phase = HAL_GPIO_ReadPin(hencoder->B_Port, hencoder->B_Pin);
    hencoder->last_state = (a_phase << 1) | b_phase;
    
    // ��ʼ������״̬
    hencoder->key_state = ENCODER_KEY_RELEASED;
    hencoder->key_press_flag = 0;
    hencoder->key_press_time = 0;
    hencoder->key_last_change_time = HAL_GetTick();
    hencoder->key_debounce_count = 0;
}

// ���±�����״̬(�����������)
void Encoder_Update(Encoder_HandleTypeDef* hencoder) {
    uint8_t a_phase, b_phase;
    uint8_t current_state;
    uint32_t current_time = HAL_GetTick();
    
    // 1. ���±�������λ״̬
    a_phase = HAL_GPIO_ReadPin(hencoder->A_Port, hencoder->A_Pin);
    b_phase = HAL_GPIO_ReadPin(hencoder->B_Port, hencoder->B_Pin);
    current_state = (a_phase << 1) | b_phase;
    
    // ���״̬�仯
    if (current_state != hencoder->last_state) {
        // ����״̬�仯�жϷ���
        if (hencoder->last_state == 0x03) {
            if (current_state == 0x01) hencoder->dir = ENCODER_DIR_CW;
            if (current_state == 0x02) hencoder->dir = ENCODER_DIR_CCW;
        }
        if (hencoder->last_state == 0x01) {
            if (current_state == 0x00) hencoder->dir = ENCODER_DIR_CW;
            if (current_state == 0x03) hencoder->dir = ENCODER_DIR_CCW;
        }
        if (hencoder->last_state == 0x00) {
            if (current_state == 0x02) hencoder->dir = ENCODER_DIR_CW;
            if (current_state == 0x01) hencoder->dir = ENCODER_DIR_CCW;
        }
        if (hencoder->last_state == 0x02) {
            if (current_state == 0x03) hencoder->dir = ENCODER_DIR_CW;
            if (current_state == 0x00) hencoder->dir = ENCODER_DIR_CCW;
        }
        
        // ���¼���
        if (hencoder->dir == ENCODER_DIR_CW) {
            hencoder->count++;
        } else if (hencoder->dir == ENCODER_DIR_CCW) {
            hencoder->count--;
        }
        
        // ���浱ǰ״̬
        hencoder->last_state = current_state;
    } else {
        hencoder->dir = ENCODER_DIR_NONE;
    }
    
    // 2. ���°���״̬(����������)
    uint8_t key_level = HAL_GPIO_ReadPin(hencoder->KEY_Port, hencoder->KEY_Pin);
    
    // ������ƽ���(��������: ����Ϊ�͵�ƽ)
    if (key_level == GPIO_PIN_RESET) {
        // ��⵽��������
        hencoder->key_debounce_count++;
        if (hencoder->key_debounce_count >= ENCODER_KEY_DEBOUNCE_THRESHOLD) {
            // ������ȷ�ϰ���
            if (hencoder->key_state != ENCODER_KEY_PRESSED) {
                hencoder->key_state = ENCODER_KEY_PRESSED;
                hencoder->key_press_flag = 1;
                hencoder->key_last_change_time = current_time;
                hencoder->key_press_time = 0; // ���ð���ʱ�����
            } else {
                // ���㰴��ʱ��
                hencoder->key_press_time = current_time - hencoder->key_last_change_time;
            }
        }
    } else {
        // ��⵽�����ͷ�
        hencoder->key_debounce_count--;
        if (hencoder->key_debounce_count > ENCODER_KEY_DEBOUNCE_THRESHOLD) {
            hencoder->key_debounce_count = ENCODER_KEY_DEBOUNCE_THRESHOLD;
        }
        
        if (hencoder->key_debounce_count <= 0) {
            // ������ȷ���ͷ�
            if (hencoder->key_state == ENCODER_KEY_PRESSED) {
                // �ж��ǵ������ǳ���
                if (hencoder->key_press_time < ENCODER_KEY_CLICK_MAX_TIME) {
                    hencoder->key_state = ENCODER_KEY_CLICKED;
                } else {
                    hencoder->key_state = ENCODER_KEY_RELEASED;
                }
                hencoder->key_last_change_time = current_time;
                hencoder->key_debounce_count = 0;
            } else if (hencoder->key_state == ENCODER_KEY_CLICKED) {
                // ����״ֻ̬ά��һ�θ���
                hencoder->key_state = ENCODER_KEY_RELEASED;
            }
        }
    }
}

// ��ȡ����������
int32_t Encoder_GetCount(Encoder_HandleTypeDef* hencoder) {
    return hencoder->count;
}

// ��ȡ����������
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef* hencoder) {
    return hencoder->dir;
}

// ��ȡ������ǰ״̬
Encoder_KeyTypeDef Encoder_GetKeyState(Encoder_HandleTypeDef* hencoder) {
    return hencoder->key_state;
}

// ��鰴���Ƿ���
uint8_t Encoder_IsKeyPressed(Encoder_HandleTypeDef* hencoder) {
    return (hencoder->key_state == ENCODER_KEY_PRESSED) ? 1 : 0;
}

// ��鰴���Ƿ񵥻�(���²��ͷ�)
uint8_t Encoder_IsKeyClicked(Encoder_HandleTypeDef* hencoder) {
    if (hencoder->key_state == ENCODER_KEY_CLICKED) 
		{
        hencoder->key_state = ENCODER_KEY_RELEASED;
        return 1;
    }
    return 0;
}

// ��ȡ��������ʱ��(ms)
uint32_t Encoder_GetKeyPressTime(Encoder_HandleTypeDef* hencoder) {
    return hencoder->key_press_time;
}