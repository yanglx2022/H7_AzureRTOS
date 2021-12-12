/**
 * @brief   : USB_OTG硬件驱动
 * @author  : yanglx2022
 * @version : V0.1
 * @date    : 2021-12-05
 */

#ifndef USB_OTG_H
#define USB_OTG_H
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

HRESULT USB_OTG_Init(void);
HRESULT USB_OTG_Start(void);


#ifdef __cplusplus
}
#endif
#endif /* USB_OTG_H */


