/*************************************************************************************
  * 文件名称：菜单显示任务文件（5任务滚动版）
  * 文件说明：Y轴16像素/项，单次显3项，PA8下移滚动，确认进入5个任务
  * 作    者：Cube
  * 注    意：适配128x64 OLED，需配合中断文件、任务句柄使用
  * 链    接：
  * 更新日期：
  * GitHub： 
*************************************************************************************/ 
/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
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
 *            佛祖保佑     永不宕机     永无BUG
 */

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MenuTask.h"
#include "OLED.h"  

typedef enum {
    MENU_CLOCK,    
    MENU_ALARM,    
    MENU_GAME,    
    MENU_TEMP,     
    MENU_GYRO,    
    MENU_COUNT 
} MenuItemType;

// 2. 按键事件定义（与PA8中断对齐）
typedef enum {
    KEY_EVENT_DOWN = 1,  // PA8按键：选中项下移+滚动
    KEY_EVENT_ENTER = 2  // 确认按键：进入选中任务（需另配按键中断，如PA9）
} KeyEventType;

// 3. 全局变量声明（需在中断文件中定义）
extern QueueHandle_t KeyQueue;               // 按键事件队列
extern volatile uint8_t menuInterruptEnabled;// 菜单中断使能开关

// 4. 5个任务的句柄（需在任务创建处定义，此处声明用于切换）
extern TaskHandle_t xClockTaskHandle;    // 时钟任务句柄
extern TaskHandle_t xAlarmTaskHandle;    // 闹钟任务句柄
extern TaskHandle_t xGameTaskHandle;     // 游戏任务句柄
extern TaskHandle_t xTempTaskHandle;     // 测温任务句柄
extern TaskHandle_t xGyroTaskHandle;     // 陀螺仪任务句柄

// 5. 菜单名称数组（与MenuItemType索引对应，英文显示）
static char* g_menuNames[MENU_COUNT] = {
    "Clock",    // MENU_CLOCK
    "Alarm",    // MENU_ALARM
    "Game",     // MENU_GAME
    "Temp",     // MENU_TEMP
    "Gyro"      // MENU_GYRO
};


// 6. 显示当前菜单（核心：按偏移量显示3项，Y轴固定16像素）
static void ShowCurrentMenu(uint8_t selectedIndex, uint8_t displayOffset) {
    OLED_Clear();  // 清屏（避免残影）
    
    // 第1行：固定显示标题（Y=0，16像素高度）
    OLED_ShowString(0, 0, "Menu", OLED_8X16);
    
    // 第2-4行：显示当前偏移量对应的3个任务（Y=16、32、48， each 16px）
    for (uint8_t i = 0; i < 3; i++) {  // 单次显3项
        uint8_t menuIdx = displayOffset + i;  // 当前要显示的任务索引
        if (menuIdx >= MENU_COUNT) break;     // 超出任务总数则停止（仅最后一页可能触发）
        
        // 选中项前加">", 未选中加空格，Y轴坐标=16 + i*16（固定16像素间隔）
        if (menuIdx == selectedIndex) {
            OLED_ShowString(0, 16 + i*16, ">", OLED_8X16);  // 选中标记
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);  // 任务名称
        } else {
            OLED_ShowString(0, 16 + i*16, " ", OLED_8X16);  // 未选中留空
            OLED_ShowString(16, 16 + i*16, g_menuNames[menuIdx], OLED_8X16);
        }
    }
    
    OLED_Update();  // 刷新显示（必须调用，否则不更新屏幕）
}


// 7. 计算显示偏移量（根据当前选中项，确保选中项在显示区域内）
static uint8_t CalcDisplayOffset(uint8_t selectedIndex) {
    uint8_t offset = 0;
    // 选中项0-2 → 偏移0（显示0-2）
    // 选中项1-3 → 偏移1（显示1-3）
    // 选中项2-4 → 偏移2（显示2-4）
    if (selectedIndex >= 2 && selectedIndex <= 4) {
        offset = selectedIndex - 2;  // 选中项2→偏移0？不：选中项2时，偏移0显示0-2，偏移1显示1-3，偏移2显示2-4；此处逻辑修正：选中项>=2时，偏移=selectedIndex-2 → 选中2→0，3→1，4→2
    } else if (selectedIndex == 1) {
        offset = 1;  // 选中1时，偏移1显示1-3（确保下移时能滚动）
    }
    // 选中0时偏移0（默认）
    return offset;
}


// 8. 进入目标任务（暂停菜单任务，恢复选中的任务）
static void EnterTargetTask(uint8_t selectedIndex) {
    // 1. 先禁用菜单中断（避免任务切换时误触发按键）
    menuInterruptEnabled = 0;
    
    // 2. 暂停菜单任务，恢复目标任务（FreeRTOS任务切换标准用法）
    switch (selectedIndex) {
        case MENU_CLOCK:
            vTaskResume(xClockTaskHandle);
            vTaskSuspend(NULL);  // 暂停当前菜单任务（NULL=自身句柄）
            break;
        case MENU_ALARM:
            vTaskResume(xAlarmTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_GAME:
            vTaskResume(xGameTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_TEMP:
            vTaskResume(xTempTaskHandle);
            vTaskSuspend(NULL);
            break;
        case MENU_GYRO:
            vTaskResume(xGyroTaskHandle);
            vTaskSuspend(NULL);
            break;
        default:
            break;
    }
}


/* 
 * -----------------------------------------菜单任务入口------------------------------------------
 * 函 数 名：MenuTask
 * 说    明：1. PA8按键：选中项下移+滚动显示；2. 确认按键：进入5个任务；3. 任务返回后恢复菜单
 * 核心逻辑：selectedIndex（选中项索引）+ displayOffset（显示偏移量）控制UI滚动
 * 参    数: pvParameters - 任务创建时传入的参数（此处未使用）
 * 返 回 值：无（FreeRTOS任务必须死循环，不返回）
 * 注    意：需在中断文件中定义KeyQueue和menuInterruptEnabled
 **/
void MenuTask(void *pvParameters) 
{
    uint8_t selectedIndex = MENU_CLOCK;  // 初始选中第0项（Clock）
    uint8_t displayOffset = 0;           // 初始显示偏移0（显示0-2项）
    KeyEventType keyEvent;               // 接收按键事件（1=下移，2=确认）
    
    // 步骤1：初始化OLED（仅执行一次，避免重复初始化）
    OLED_Init();
    
    // 步骤2：创建按键队列（若未在其他地方创建，深度5足够缓存按键事件）
    if (KeyQueue == NULL) {
        KeyQueue = xQueueCreate(5, sizeof(KeyEventType));
        configASSERT(KeyQueue != NULL);  // 断言：队列创建失败则提示（调试用）
    }
    
    // 步骤3：允许PA8按键中断（菜单任务运行时才使能，避免其他任务误触发）
    menuInterruptEnabled = 1;
    
    // 步骤4：初始显示菜单（偏移0，选中Clock）
    ShowCurrentMenu(selectedIndex, displayOffset);
    
    // 步骤5：菜单主循环（FreeRTOS任务必须死循环，不能退出）
    for (;;) {
        // 等待按键事件（超时100ms：兼顾响应速度和CPU低占用）
        if (xQueueReceive(KeyQueue, &keyEvent, pdMS_TO_TICKS(100)) == pdPASS) {
            switch (keyEvent) {
                // -------------- PA8按键：选中项下移+滚动 --------------
                case KEY_EVENT_DOWN:
                    // 1. 选中项下移（循环：最后一项→第一项）
                    selectedIndex++;
                    if (selectedIndex >= MENU_COUNT) {
                        selectedIndex = 0;  // 4（Gyro）→0（Clock）
                    }
                    
                    // 2. 计算新的显示偏移量（确保选中项在当前显示区域内）
                    displayOffset = CalcDisplayOffset(selectedIndex);
                    
                    // 3. 更新显示（按新的选中项和偏移量刷新UI）
                    ShowCurrentMenu(selectedIndex, displayOffset);
                    break;
                
                // -------------- 确认按键：进入选中任务 --------------
                case KEY_EVENT_ENTER:
                    // 切换到目标任务（任务返回后才会执行后续代码）
                    EnterTargetTask(selectedIndex);
                    
                    // 目标任务返回后（菜单任务被恢复），重新初始化显示
                    displayOffset = CalcDisplayOffset(selectedIndex);  // 重新计算偏移
                    ShowCurrentMenu(selectedIndex, displayOffset);    // 刷新菜单
                    menuInterruptEnabled = 1;                         // 重新允许中断
                    break;
                
                default:
                    break;
            }
        }
    }
}