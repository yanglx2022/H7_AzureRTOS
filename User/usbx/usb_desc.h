/**
 * @brief   : USB描述符
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#ifndef USB_DESC_H
#define USB_DESC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// 描述符类型定义
#define  USB_DESC_TYPE_DEVICE                           0x01U
#define  USB_DESC_TYPE_CONFIGURATION                    0x02U
#define  USB_DESC_TYPE_STRING                           0x03U
#define  USB_DESC_TYPE_INTERFACE                        0x04U
#define  USB_DESC_TYPE_ENDPOINT                         0x05U

// 电源配置
#define USB_CONFIG_BUS_POWERED                          0x80
#define USB_CONFIG_SELF_POWERED                         0xC0
#define USB_CONFIG_POWER_MA(mA)                         ((mA)/2)

#define WBVAL(x)                                        ((x) & 0xFF),(((x) >> 8) & 0xFF)

#define USB_MAX_EP0_SIZE                                64U
#define USB_FS_MAX_PACKET_SIZE                          64U


uint8_t* device_config_descriptor_get(uint32_t* length);
uint8_t* string_descriptor_get(uint32_t* length);
uint8_t* language_id_get(uint32_t* length);


#ifdef __cplusplus
}
#endif
#endif /* USB_DESC_H */


