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

// ��ʼ��������
void Encoder_Init(Encoder_HandleTypeDef *encoder) {
    // ��������
    encoder->A_GPIO_Port = GPIOA;
    encoder->A_Pin = GPIO_PIN_11;
    encoder->B_GPIO_Port = GPIOA;
    encoder->B_Pin = GPIO_PIN_15;
    encoder->C_GPIO_Port = GPIOA;
    encoder->C_Pin = GPIO_PIN_12;
    
    // ��ʼ��״̬�ͼ���
    encoder->count = 0;
    encoder->dir = ENCODER_DIR_NONE;
    
    // ��ȡ��ʼ״̬
    uint8_t a_state = HAL_GPIO_ReadPin(encoder->A_GPIO_Port, encoder->A_Pin);
    uint8_t b_state = HAL_GPIO_ReadPin(encoder->B_GPIO_Port, encoder->B_Pin);
    encoder->last_state = (a_state << 1) | b_state;
}

// ���±�����״̬
void Encoder_Update(Encoder_HandleTypeDef *encoder) {
    uint8_t a_state = HAL_GPIO_ReadPin(encoder->A_GPIO_Port, encoder->A_Pin);
    uint8_t b_state = HAL_GPIO_ReadPin(encoder->B_GPIO_Port, encoder->B_Pin);
    uint8_t current_state = (a_state << 1) | b_state;
    
    // ���״̬�Ƿ��б仯
    if (current_state != encoder->last_state) {
        // ����״̬�仯�жϷ���
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
                // ��Ч״̬�仯����������������
                encoder->dir = ENCODER_DIR_NONE;
                break;
        }
        
        // ������һ��״̬
        encoder->last_state = current_state;
    } else {
        encoder->dir = ENCODER_DIR_NONE;
    }
}

// ��ȡ��ǰ����
int32_t Encoder_GetCount(Encoder_HandleTypeDef *encoder) {
    return encoder->count;
}

// ��ȡ��ǰ����
Encoder_DirTypeDef Encoder_GetDir(Encoder_HandleTypeDef *encoder) {
    return encoder->dir;
}

// ���ü���
void Encoder_ResetCount(Encoder_HandleTypeDef *encoder) {
    encoder->count = 0;
}

