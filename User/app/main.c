/**
 * @brief   : STM32H743 Azure RTOS全家桶示例
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#include "common.h"
#include "sdram.h"
#include "idle_task.h"

int main(void)
{
    Sys_Init();
    SDRAM_Init();

    // printf测试(中文需要串口调试助手支持UTF-8)
    float pi = 3.14159f;
    printf("hello world 你好 %.2f\n", pi);

    // SDRAM测试
    uint8_t* data = (uint8_t*)SDRAM_BASE;
    for(int i = 0; i < 100; i++)
    {
        data[i] = i;
    }
    for(int i = 0; i < 100; i++)
    {
        if (data[i] != i)
        {
            printf("SDRAM Test Failed!\n");
            break;
        }
    }
    printf("SDRAM Test Finished\n");

    // 延时测试
    for(int i = 0; i < 10; i++)
    {
        printf("%d\n", i);
        delay_ms(1000);
    }

    // ThreadX
    tx_kernel_enter();
    while(1);
}

void tx_application_define(void* first_unused_memory)
{
    // 空闲任务
    idle_thread_create();
}


