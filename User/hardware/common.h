/**
 * @brief   : MPU 时钟 延时 串口打印等
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#ifndef COMMON_H
#define COMMON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_system.h"
#include "stm32h7xx_ll_utils.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_gpio.h"
#include "stdio.h"

// 硬件驱动返回值
typedef enum
{
    HR_OK = 0,          // 成功
    HR_ERROR,           // 未知错误
    HR_NO_DEVICE,       // 无设备
    HR_TIMEOUT,         // 超时
    HR_SD_PLUG_IN,      // SD卡插入
    HR_SD_PLUG_OFF,     // SD卡拔出
    HR_INVALID_ADDRESS, // 地址无效
    HR_ECC_ERROR        // ECC校验失败
} HRESULT;

// 中断优先级分配
#define SYSTICK_PRI                 2
#define ADC_DMA_PRI                 4
#define ADC_TIM_PRI                 5
#define MDMA_PRI                    6
#define SDCARD_PRI                  7
#define USB_PRI                     8

// 是否使用RTOS(影响延时相关函数)
#define USE_RTOS                    1
#if USE_RTOS
#include "tx_api.h"
// 任务优先级分配
#define FILE_TASK_PRI               14
#define TOUCH_TASK_PRI              19
#define USB_TASK_PRI                20
#define GUI_TASK_PRI                25
#define IDLE_TASK_PRI               31
#endif

/**
 * @brief : 获取cpu tick值(400MHz时1tick=2.5ns 最大2^32*2.5ns=10.7s)
 * @param  
 * @return cpu tick
 */
__STATIC_INLINE uint32_t get_cpu_tick(void)
{
    return (DWT->CYCCNT);
}

/**
 * @brief : cpu tick转ns(主频400MHz)
 * @param  cpu_tick cpu tick值
 * @return ns
 */
__STATIC_INLINE uint32_t cpu_tick_to_ns(uint32_t cpu_tick)
{
    return ((cpu_tick) * 5 / 2);
}

/**
 * @brief : ns转cpu tick(主频400MHz)
 * @param  ns ns值
 * @return cpu tick
 */
__STATIC_INLINE uint32_t ns_to_cpu_tick(uint32_t ns)
{
    return ((ns) / 5 * 2 + 1);
}

/**
 * @brief : ns延时(精度2.5ns)
 * @param  num ns数
 * @return 
 */
__STATIC_INLINE void delay_ns(uint32_t num)
{
    uint32_t start = DWT->CYCCNT;
    num = ns_to_cpu_tick(num);
    while((DWT->CYCCNT - start) < num);
}

void Sys_Init(void);
void delay_us(uint32_t num);
void delay_ms(uint32_t num);
uint32_t get_tick_ms(void);


#ifdef __cplusplus
}
#endif
#endif /* COMMON_H */


