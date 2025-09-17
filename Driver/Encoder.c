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
 
#include "Encoder.h"
#include "stm32f1xx_hal.h"

// 宏定义
#define ENCODER_KEY_DEBOUNCE_THRESHOLD 5    // 防抖阈值(连续检测次数)
#define ENCODER_KEY_CLICK_MAX_TIME 500      // 单击最大时长(ms)

// 初始化编码器
void Encoder_Init(Encoder_HandleTypeDef* hencoder, 
                 GPIO_TypeDef* A_Port, uint16_t A_Pin,
                 GPIO_TypeDef* B_Port, uint16_t B_Pin,
                 GPIO_TypeDef* KEY_Port, uint16_t KEY_Pin) {
    // 保存引脚信息
    hencoder->A_Port = A_Port;
    hencoder->A_Pin = A_Pin;
    hencoder->B_Port = B_Port;
    hencoder->B_Pin = B_Pin;
    hencoder->KEY_Port = KEY_Port;
    hencoder->KEY_Pin = KEY_Pin;
    
    // 初始化状态
    hencoder->count = 0;
    hencoder->dir = ENCODER_DIR_NONE;
    hencoder->last_state = 0;
    
    // 读取初始状态
    uint8_t a_phase = HAL_GPIO_ReadPin(hencoder->A_Port, hencoder->A_Pin);
    uint8_t b_phase = HAL_GPIO_ReadPin(hencoder->B_Port, hencoder->B_Pin);
    hencoder->last_state = (a_phase << 1) | b_phase;
    
    // 初始化按键状态
    hencoder->key_state = ENCODER_KEY_RELEASED;
    hencoder->key_press_flag = 0;
    hencoder->key_press_time = 0;
    hencoder->key_last_change_time = HAL_GetTick();
    hencoder->key_debounce_count = 0;
}

// 更新编码器状态(包括按键检测)
void Encoder_Update(Encoder_HandleTypeDef* hencoder) {
    uint8_t a_phase, b_phase;
    uint8_t current_state;
    uint32_t current_time = HAL_GetTick();
    
    // 1. 更新编码器相位状态
    a_phase = HAL_GPIO_ReadPin(hencoder->A_Port, hencoder->A_Pin);
    b_phase = HAL_GPIO_ReadPin(hencoder->B_Port, hencoder->B_Pin);
    current_state = (a_phase << 1) | b_phase;
    
    // 检测状态变化
    if (current_state != hencoder->last_state) {
        // 根据状态变化判断方向
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
        
        // 更新计数
        if (hencoder->dir == ENCODER_DIR_CW) {
            hencoder->count++;
        } else if (hencoder->dir == ENCODER_DIR_CCW) {
            hencoder->count--;
        }
        
        // 保存当前状态
        hencoder->last_state = current_state;
    } else {
        hencoder->dir = ENCODER_DIR_NONE;
    }
    
    // 2. 更新按键状态(带防抖处理)
    uint8_t key_level = HAL_GPIO_ReadPin(hencoder->KEY_Port, hencoder->KEY_Pin);
    
    // 按键电平检测(上拉配置: 按下为低电平)
    if (key_level == GPIO_PIN_RESET) {
        // 检测到按键按下
        hencoder->key_debounce_count++;
        if (hencoder->key_debounce_count >= ENCODER_KEY_DEBOUNCE_THRESHOLD) {
            // 防抖后确认按下
            if (hencoder->key_state != ENCODER_KEY_PRESSED) {
                hencoder->key_state = ENCODER_KEY_PRESSED;
                hencoder->key_press_flag = 1;
                hencoder->key_last_change_time = current_time;
                hencoder->key_press_time = 0; // 重置按下时间计数
            } else {
                // 计算按下时长
                hencoder->key_press_time = current_time - hencoder->key_last_change_time;
            }
        }
    } else {
        // 检测到按键释放
        hencoder->key_debounce_count--;
        if (hencoder->key_debounce_count > ENCODER_KEY_DEBOUNCE_THRESHOLD) {
            hencoder->key_debounce_count = ENCODER_KEY_DEBOUNCE_THRESHOLD;
        }
        
        if (hencoder->key_debounce_count <= 0) {
            // 防抖后确认释放
            if (hencoder->key_state == ENCODER_KEY_PRESSED) {
                // 判断是单击还是长按
                if (hencoder->key_press_time < ENCODER_KEY_CLICK_MAX_TIME) {
                    hencoder->key_state = ENCODER_KEY_CLICKED;
                } else {
                    hencoder->key_state = ENCODER_KEY_RELEASED;
                }
                hencoder->key_last_change_time = current_time;
                hencoder->key_debounce_count = 0;
            } else if (hencoder->key_state == ENCODER_KEY_CLICKED) {
                // 单击状态只维持一次更新
                hencoder->key_state = ENCODER_KEY_RELEASED;
            }
        }
    }
}

// 获取编码器计数
int32_t Encoder_GetCount(Encoder_HandleTypeDef* hencoder) {
    return hencoder->count;
}

// 获取编码器方向
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef* hencoder) {
    return hencoder->dir;
}

// 获取按键当前状态
Encoder_KeyTypeDef Encoder_GetKeyState(Encoder_HandleTypeDef* hencoder) {
    return hencoder->key_state;
}

// 检查按键是否按下
uint8_t Encoder_IsKeyPressed(Encoder_HandleTypeDef* hencoder) {
    return (hencoder->key_state == ENCODER_KEY_PRESSED) ? 1 : 0;
}

// 检查按键是否单击(按下并释放)
uint8_t Encoder_IsKeyClicked(Encoder_HandleTypeDef* hencoder) {
    if (hencoder->key_state == ENCODER_KEY_CLICKED) 
		{
        hencoder->key_state = ENCODER_KEY_RELEASED;
        return 1;
    }
    return 0;
}

// 获取按键按下时长(ms)
uint32_t Encoder_GetKeyPressTime(Encoder_HandleTypeDef* hencoder) {
    return hencoder->key_press_time;
}