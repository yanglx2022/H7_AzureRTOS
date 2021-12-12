/**
 * @brief   : USBX设备配置
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#include "usb_device.h"
#include "usb_desc.h"
#include "ux_device_class_storage.h"
#include "ux_dcd_stm32.h"
#include "usb_otg.h"
#include "usb_storage_driver.h"

// USBX内存分配
#define USBX_MEMORY_SIZE                    (32 * 1024)
static uint8_t usbx_memory[USBX_MEMORY_SIZE];

// Slave Storage Class参数(使用时引用的是指针,注意要定义为全局)
static UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;

/**
 * @brief : USB初始化
 * @param  
 * @return 
 */
UINT usb_init(void)
{
    // USBX初始化
    UINT ret = ux_system_initialize(usbx_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (ret != UX_SUCCESS)
    {
        return ret;
    }
    
    // USB设备协议栈初始化
    ULONG device_framework_fs_length;
    ULONG string_framework_length;
    ULONG languge_id_framework_length;
    UCHAR* device_framework_fs      = device_config_descriptor_get((uint32_t*)&device_framework_fs_length);
    UCHAR* string_framework         = string_descriptor_get((uint32_t*)&string_framework_length);
    UCHAR* language_id_framework    = language_id_get((uint32_t*)&languge_id_framework_length);
    ret = ux_device_stack_initialize(NULL, 0, 
                                     device_framework_fs, device_framework_fs_length, 
                                     string_framework, string_framework_length, 
                                     language_id_framework, languge_id_framework_length, 
                                     UX_NULL);
    if (ret != UX_SUCCESS)
    {
        return ret;
    }
    
    // USB Storage Class配置
    storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;
    // lun0 NandFlash
    ULONG block_num = (nand_flash.lx_nand_flash_total_blocks - nand_flash.lx_nand_flash_bad_blocks) * (nand_flash.lx_nand_flash_pages_per_block - 1);
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba        = block_num - 1;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length    = nand_flash.lx_nand_flash_bytes_per_page;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type            = 0;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag  = 0x80;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read            = usb_storage_media_read;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write           = usb_storage_media_write; 
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status          = usb_storage_media_status;
    // 注册到配置1 接口0
    ret = ux_device_stack_class_register(_ux_system_slave_class_storage_name, 
                                         _ux_device_class_storage_entry, 
                                         1, 0, (VOID*)&storage_parameter);
    if (ret != UX_SUCCESS)
    {
        return ret;
    }
    
    // USB硬件配置
    ret = USB_OTG_Init();
    if (ret != HR_OK)
    {
        return ret;
    }
    
    // 链接到USBX协议栈
    ret = ux_dcd_stm32_initialize((ULONG)USB_OTG_FS, (ULONG)&hpcd_USB_OTG_FS);
    if (ret != UX_SUCCESS)
    {
        return ret;
    }
    
    // USB启动
    ret = USB_OTG_Start();
    if (ret != HR_OK)
    {
        return ret;
    }
    
    return 0;
}


