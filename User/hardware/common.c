/**
 * @brief   : MPU 时钟 延时 串口打印等
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-04
 */

#include "common.h"

#if USE_RTOS
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_initialize.h"
#else
// SysTick ms计数值
static __IO uint32_t tick_ms;
#endif

static void MPU_Cache_Config(void);
static void SystemClock_Config(void);
static void Delay_Init(void);
static void USART1_Init(void);
static void LED_Init(void);

/**
 * @brief : MCU初始化
 * @param  
 * @return 
 */
void Sys_Init(void)
{
    // 中断分组
    NVIC_SetPriorityGrouping(3);	// 全抢占
    // MPU
    MPU_Cache_Config();
    // 时钟
    SystemClock_Config();
    SystemCoreClockUpdate();
    // 延时
    Delay_Init();
    // 串口1
    USART1_Init();
    // LED
    LED_Init();
}

// 配置MPU 开启Cache
static void MPU_Cache_Config(void)
{
    // MPU配置说明(只要启用cache即开启read-allocate)
    // TEX=000 C=0 B=0 ----- 内存类型Strongly ordered
    // TEX=000 C=0 B=1 ----- 内存类型Device
    // TEX=000 C=1 B=0 ----- 内存类型Normal, write-through, no write-allocate
    // TEX=000 C=1 B=1 ----- 内存类型Normal, write-back, no write-allocate
    // TEX=001 C=0 B=0 ----- 内存类型Normal, 禁用cache
    // TEX=001 C=1 B=1 ----- 内存类型Normal, write-back, write-allocate

    // MPU配置前禁能
    LL_MPU_Disable();
    // AXI SRAM最高性能(Normal, Write back, Write allocate)
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER0, 
                        0x00,                               // 子区域全使能
                        AXI_SRAM_BASE,                      // AXI SRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL1                |  // TEX=001
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_CACHEABLE          |  // C=1使能Cache
                        LL_MPU_ACCESS_BUFFERABLE         |  // B=1禁止缓冲
                        LL_MPU_REGION_SIZE_512KB);          // 512KB
    // SDRAM(Normal, Write through)
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER1, 
                        0x00,                               // 子区域全使能
                        SDRAM_BASE,                         // SDRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL0                |  // TEX=000
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_CACHEABLE          |  // C=1使能Cache
                        LL_MPU_ACCESS_NOT_BUFFERABLE     |  // B=0禁止缓冲
                        LL_MPU_REGION_SIZE_32MB);           // 32MB
    // NAND Flash内存类型Device
    LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER2, 
                        0x00,                               // 子区域全使能
                        NAND_DEVICE,                        // SDRAM地址
                        LL_MPU_INSTRUCTION_ACCESS_ENABLE |  // 允许执行代码
                        LL_MPU_REGION_FULL_ACCESS        |  // 完全访问权限(特权与非特权模式均读写)
                        LL_MPU_TEX_LEVEL0                |  // TEX=000
                        LL_MPU_ACCESS_NOT_SHAREABLE      |  // S=0关闭共享
                        LL_MPU_ACCESS_NOT_CACHEABLE      |  // C=0
                        LL_MPU_ACCESS_BUFFERABLE         |  // B=1
                        LL_MPU_REGION_SIZE_512MB);          // 512MB
    // 使能MPU
    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);	// 使能背景区, NMI与HardFault中断中关闭MPU
    // 开启Cache
    SCB_EnableICache();
    SCB_EnableDCache();
}

// 配置时钟
static void SystemClock_Config(void)
{
    // 晶振为25MHz
    // 注意主频改变时要同步修改延时相关参数,SDRAM刷新率以及tx_initialize_low_level.S中SYSTEM_CLOCK

    // 设置FLASH读延时(2个等待状态)
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2);
    // 使能稳压器
    LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
    // 电压调节1(最佳FLASH访问性能)
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while(LL_PWR_GetRegulVoltageScaling() != LL_PWR_REGU_VOLTAGE_SCALE1);

    // 使能HSE
    LL_RCC_HSE_Enable();
    while(LL_RCC_HSE_IsReady() == 0);

    // 使能HSI48(USB使用)
    LL_RCC_HSI48_Enable();
    while(LL_RCC_HSI48_IsReady() == 0);

    // // 使能后备区
    // LL_PWR_EnableBkUpAccess();
    // // 使能LSE
    // LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    // LL_RCC_LSE_Enable();
    // while(LL_RCC_LSE_IsReady() == 0);
    // // 使能RTC
    // LL_RCC_EnableRTC();

    // 设置PLL时钟源为HSE
    LL_RCC_PLL_SetSource(LL_RCC_PLLSOURCE_HSE);

    // 设置PLL1输入频率范围4MHz~8MHz
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
    // 设置PLL1输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL1预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL1输出频率范围内
    LL_RCC_PLL1_SetM(5);
    LL_RCC_PLL1_SetN(160);      // 25MHz/5*160=800MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL1_SetP(2);        // 800MHz/2=400MHz
    LL_RCC_PLL1_SetQ(4);        // 800MHz/4=200MHz
    LL_RCC_PLL1_SetR(2);        // 800MHz/2=400MHz
    // 使能PLL1
    LL_RCC_PLL1P_Enable();
    LL_RCC_PLL1_Enable();
    while(LL_RCC_PLL1_IsReady() == 0);

    // 设置PLL2输入频率范围1MHz~2MHz
    LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_1_2);
    // 设置PLL2输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL2预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL2输出频率范围内
    LL_RCC_PLL2_SetM(25);
    LL_RCC_PLL2_SetN(504);      // 25MHz/25*504=504MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL2_SetP(7);        // 504MHz/7=72MHz
    LL_RCC_PLL2_SetQ(2);        // 504MHz/2=252MHz
    LL_RCC_PLL2_SetR(21);       // 504MHz/21=24MHz
    LL_RCC_PLL2P_Enable();
    // 使能PLL2
    LL_RCC_PLL2P_Enable();
    LL_RCC_PLL2R_Enable();
    LL_RCC_PLL2_Enable();
    while(LL_RCC_PLL2_IsReady() == 0);

    // 设置PLL3输入频率范围8MHz~16MHz
    LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    // 设置PLL3输出频率范围WIDE(192~836MHz)
    LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
    // 设置PLL3预分频系数M(取值1~63) 倍频系数N(取值4~512) 注意:N倍频后频率要在PLL3输出频率范围内
    LL_RCC_PLL3_SetM(32);
    LL_RCC_PLL3_SetN(128);      // 25MHz/32*128=100MHz
    // 设置PLL1P PLL1Q PLL1R分频系数(取值2~128)
    LL_RCC_PLL3_SetP(2);        // 100MHz/2=50MHz
    LL_RCC_PLL3_SetQ(2);        // 100MHz/2=50MHz
    LL_RCC_PLL3_SetR(3);        // LTDC 2--50MHz时屏幕闪烁严重(刷新时闪现一些像素粒)  3--33.3MHz无像素粒
    // 使能PLL3
    LL_RCC_PLL3R_Enable();      // LTDC时钟固定为PLL3R
    LL_RCC_PLL3_Enable();
    while(LL_RCC_PLL3_IsReady() == 0);

    // AHB预分频设为2
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);
    // 选择PLL1作为系统时钟源 则SYSCLK=PLL1P=400MHz
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    // 系统预分频为1 则CPU主频=SYSCLK=400MHz
    LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_SetSystemCoreClock(400000000);

    // 设置AHB APB预分频
    LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_2);       // AXI AHB HCLK = CPU主频/2=200MHz
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);     // APB = AHB/2=100MHz APB_TIM = AHB/2*2=200MHz
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_2);
    LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_2);

    // 设置外设时钟源
    LL_RCC_SetUSARTClockSource(LL_RCC_USART16_CLKSOURCE_PCLK2);
    LL_RCC_SetFMCClockSource(LL_RCC_FMC_CLKSOURCE_HCLK);
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_HSI48);
    // LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    // LL_RCC_SetQSPIClockSource(LL_RCC_QSPI_CLKSOURCE_HCLK);
    // LL_RCC_SetSDMMCClockSource(LL_RCC_SDMMC_CLKSOURCE_PLL2R);
    // LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLL2P);
}

// 延时初始化
static void Delay_Init(void)
{
    // DWT ns级延时
    CoreDebug->DEMCR |= 1 << 24;    // 使能DWT
    DWT->CYCCNT       = 0;          // CYCCNT计数清0
    DWT->CTRL        |= 1;          // 使能CYCCNT

    #if !USE_RTOS
    // SysTick get_tick_ms
    SysTick_Config(SystemCoreClock / 1000UL);
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRI);
    #endif
}

/**
 * @brief : us延时(阻塞)
 * @param  num us数
 * @return 
 */
void delay_us(uint32_t num)
{
    uint32_t cnt = num / 4000000;
    uint32_t remain = num - cnt * 4000000;
    while(cnt--)
    {
        delay_ns(4000000000);
    }
    if (remain)
    {
        delay_ns(remain * 1000);
    }
}

/**
 * @brief : ms延时(阻塞)
 * @param  num ms数
 * @return 
 */
void delay_ms(uint32_t num)
{
    while(num--)
    {
        delay_us(1000);
    }
}

/**
 * @brief : 重写ms延时(基于DWT)(阻塞)
 * @param  Delay 延时ms数
 * @return 
 */
void HAL_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}

/**
 * @brief : 获取tick_ms值(使用RTOS但RTOS尚未运行时tick基于DWT,注意此时循环只有10.7s(400MHz时))
 * @param  
 * @return tick_ms
 */
uint32_t get_tick_ms(void)
{
    #if USE_RTOS
    if (_tx_thread_system_state == TX_INITIALIZE_IS_FINISHED)
    {
        return (1000 / TX_TIMER_TICKS_PER_SECOND * tx_time_get());
    }
    else
    {
        // 使用RTOS但RTOS尚未运行的话利用DWT的tick,注意此时一个循环只有10.7s(400MHz时)
        return (cpu_tick_to_ns(get_cpu_tick() / 1000000));
    }
    #else
    return tick_ms;
    #endif
}

/**
 * @brief : 重写获取ms tick值
 * @param  
 * @return tick_ms
 */
uint32_t HAL_GetTick(void)
{
  return get_tick_ms();
}

#if !USE_RTOS
// SysTick中断
void SysTick_Handler(void)
{
   tick_ms++;
}
#endif

// 串口1初始化
static void USART1_Init(void)
{
    // 开启时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    // PA9 ------> USART1_TX
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART1
    LL_USART_InitTypeDef USART_InitStruct = {0};
    USART_InitStruct.PrescalerValue         = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate               = 115200;
    USART_InitStruct.DataWidth              = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits               = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity                 = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection      = LL_USART_DIRECTION_TX;
    USART_InitStruct.HardwareFlowControl    = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling           = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1, &USART_InitStruct);
    LL_USART_DisableFIFO(USART1);
    LL_USART_ConfigAsyncMode(USART1);
    LL_USART_Enable(USART1);
    while(LL_USART_IsActiveFlag_TEACK(USART1) == 0);
}

// printf支持
#ifdef __ARMCC_VERSION
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
    LL_USART_TransmitData8(USART1, ch);
    while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    return ch;
}
#else
// 重定义_write函数
int _write(int fd, char *ptr, int len)  
{
    for(int i = 0; i < len; i++)
    {
        LL_USART_TransmitData8(USART1, ptr[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    return len;
}
#endif

// LED初始化
static void LED_Init(void)
{
    // 开启时钟
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);

    // PB1 LED 低电平亮
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Pin         = LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);     // 默认高电平LED不亮
}

// 错误指示(LED闪烁 串口打印错误信息)
static void Error_Handler(const char* text)
{
    // 打印错误信息
    const char* tip = "[Unhandled Error] ";
    for(int i = 0; i < strlen(tip); i++)
    {
        LL_USART_TransmitData8(USART1, tip[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    for(int i = 0; i < strlen(text); i++)
    {
        LL_USART_TransmitData8(USART1, text[i]);
        while(LL_USART_IsActiveFlag_TC(USART1) == 0);
    }
    // LED闪烁
    while(1)
    {
        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_1);
        delay_ms(250);
    }
}

/* Cortex Processor Interruption and Exception Handlers */
// NMI
void NMI_Handler(void)
{
    Error_Handler("NMI: Non maskable interrupt\n");
}

// HardFault
void HardFault_Handler(void)
{
    Error_Handler("HardFault: Hard fault interrupt\n");
}

// MemManage
void MemManage_Handler(void)
{
    Error_Handler("MemManage: Memory management fault\n");
}

// BusFault
void BusFault_Handler(void)
{
    Error_Handler("BusFault: Pre-fetch fault, memory access fault\n");
}

// UsageFault
void UsageFault_Handler(void)
{
    Error_Handler("UsageFault: Undefined instruction or illegal state\n");
}
/* */


