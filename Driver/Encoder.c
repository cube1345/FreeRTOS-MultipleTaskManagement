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

// 初始化编码器
void Encoder_Init(Encoder_HandleTypeDef *encoder) {
    // 配置引脚
    encoder->A_GPIO_Port = GPIOA;
    encoder->A_Pin = GPIO_PIN_11;
    encoder->B_GPIO_Port = GPIOA;
    encoder->B_Pin = GPIO_PIN_15;
    encoder->C_GPIO_Port = GPIOA;
    encoder->C_Pin = GPIO_PIN_12;
    
    // 初始化状态和计数
    encoder->count = 0;
    encoder->dir = ENCODER_DIR_NONE;
    
    // 读取初始状态
    uint8_t a_state = HAL_GPIO_ReadPin(encoder->A_GPIO_Port, encoder->A_Pin);
    uint8_t b_state = HAL_GPIO_ReadPin(encoder->B_GPIO_Port, encoder->B_Pin);
    encoder->last_state = (a_state << 1) | b_state;
}

// 更新编码器状态
void Encoder_Update(Encoder_HandleTypeDef *encoder) {
    uint8_t a_state = HAL_GPIO_ReadPin(encoder->A_GPIO_Port, encoder->A_Pin);
    uint8_t b_state = HAL_GPIO_ReadPin(encoder->B_GPIO_Port, encoder->B_Pin);
    uint8_t current_state = (a_state << 1) | b_state;
    
    // 检查状态是否有变化
    if (current_state != encoder->last_state) {
        // 根据状态变化判断方向
        switch ((encoder->last_state << 2) | current_state) {
            case 0x01:  // 00 -> 01
            case 0x13:  // 01 -> 11
            case 0x32:  // 11 -> 10
            case 0x20:  // 10 -> 00
                encoder->dir = ENCODER_DIR_CW;
                encoder->count++;
                break;
                
            case 0x02:  // 00 -> 10
            case 0x23:  // 10 -> 11
            case 0x31:  // 11 -> 01
            case 0x10:  // 01 -> 00
                encoder->dir = ENCODER_DIR_CCW;
                encoder->count--;
                break;
                
            default:
                // 无效状态变化，可能是噪声导致
                encoder->dir = ENCODER_DIR_NONE;
                break;
        }
        
        // 更新上一次状态
        encoder->last_state = current_state;
    } else {
        encoder->dir = ENCODER_DIR_NONE;
    }
}

// 获取当前计数
int32_t Encoder_GetCount(Encoder_HandleTypeDef *encoder) {
    return encoder->count;
}

// 获取当前方向
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef *encoder) {
    return encoder->dir;
}

// 重置计数
void Encoder_ResetCount(Encoder_HandleTypeDef *encoder) {
    encoder->count = 0;
}

