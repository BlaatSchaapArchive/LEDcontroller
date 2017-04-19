/**
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_LED_H
#define __USB_LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_led
  * @brief This file is the Header file for usbd_led.c
  * @{
  */ 


/** @defgroup usbd_led_Exported_Defines
  * @{
  */ 
#define LED_IN_EP                                   0x81  /* EP1 for data IN */
#define LED_OUT_EP                                  0x01  /* EP1 for data OUT */


/* LED Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define LED_DATA_MAX_PACKET_SIZE                 64  /* Endpoint IN & OUT Packet size */

#define USB_LED_CONFIG_DESC_SIZ                     32

#define LED_DATA_IN_PACKET_SIZE                  LED_DATA_MAX_PACKET_SIZE
#define LED_DATA_OUT_PACKET_SIZE                 LED_DATA_MAX_PACKET_SIZE




typedef struct _USBD_LED_Itf
{
  int8_t (* Init)          (void);
  int8_t (* DeInit)        (void);
  int8_t (* Receive)       (uint8_t *, uint32_t *);  

}USBD_LED_ItfTypeDef;




typedef struct
{
  uint32_t data[LED_DATA_MAX_PACKET_SIZE/4];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;    
  uint8_t  *RxBuffer;  
  uint8_t  *TxBuffer;   
  uint32_t RxLength;
  uint32_t TxLength;    
  
  __IO uint32_t TxState;     
  __IO uint32_t RxState;    
}
USBD_LED_HandleTypeDef; 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 
  
/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_ClassTypeDef  USBD_LED;
#define USBD_LED_CLASS    &USBD_LED
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_LED_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_LED_ItfTypeDef *fops);

uint8_t  USBD_LED_SetTxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff,
                                      uint16_t length);

uint8_t  USBD_LED_SetRxBuffer        (USBD_HandleTypeDef   *pdev,
                                      uint8_t  *pbuff);
  
uint8_t  USBD_LED_ReceivePacket      (USBD_HandleTypeDef *pdev);

uint8_t  USBD_LED_TransmitPacket     (USBD_HandleTypeDef *pdev);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_LED_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
