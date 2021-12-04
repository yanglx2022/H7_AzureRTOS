/**
 * @brief   : 空闲任务
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#include "idle_task.h"

// 任务堆栈
#define STACK_SIZE              2048
static UCHAR idle_stack[STACK_SIZE];
// 任务句柄
static TX_THREAD idle_thread;


// 任务函数
void idle_thread_entry(ULONG thread_input)
{
    while(1)
    {
        printf("idle_task\n");
        tx_thread_sleep(MS_TO_TICKS(1000));
    }
}

// 创建任务
void idle_thread_create(void)
{
    tx_thread_create(&idle_thread, "idle_thread", idle_thread_entry, 0, 
        idle_stack, STACK_SIZE, IDLE_TASK_PRI, IDLE_TASK_PRI, TX_NO_TIME_SLICE, TX_AUTO_START);
}


