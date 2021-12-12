/**
 * @brief   : STM32H743 Azure RTOS全家桶示例
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#include "common.h"
#include "sdram.h"
#include "idle_task.h"
#include "usb_device.h"
#include "lx_nand_driver.h"


int main(void)
{
    // 硬件初始化
    Sys_Init();
    SDRAM_Init();
    NAND_Init();

    // 进入系统
    tx_kernel_enter();
    while(1);
}

// 启动任务
void tx_application_define(void* first_unused_memory)
{
    // 空闲任务
    idle_thread_create();

    // LevelX初始化
    lx_nand_flash_initialize();
    lx_nand_flash_open(&nand_flash, "nand flash", nand_driver_initialize);
    // USBX初始化
    usb_init();
}


