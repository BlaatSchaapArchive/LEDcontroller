#include "usbd_led.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


static uint8_t  USBD_LED_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_LED_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_LED_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_LED_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_LED_DataOut (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);


static uint8_t  *USBD_LED_GetFSCfgDesc (uint16_t *length);




uint8_t  *USBD_LED_GetDeviceQualifierDescriptor (uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_LED_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};


/* LED interface class callbacks structure */
USBD_ClassTypeDef  USBD_LED = 
{
  USBD_LED_Init,
  USBD_LED_DeInit,
  USBD_LED_Setup,
  NULL,                 /* EP0_TxSent, */
  NULL, /* EP0_RxReady, */
  USBD_LED_DataIn,
  USBD_LED_DataOut,
  NULL,
  NULL,
  NULL,     
  NULL,  // High Speed
  USBD_LED_GetFSCfgDesc,    
  NULL, // Other Speed
  USBD_LED_GetDeviceQualifierDescriptor,
};


// TODO: BOS / MSOS20 (WinUSB) Descriptor, for M$ Windows support
// See  http://janaxelson.com/files/ms_os_20_descriptors.c


// TODO: Power Configurations / Switch between self/bus powered

/* USB LED device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_LED_CfgFSDesc[USB_LED_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_LED_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x01,   /* bNumInterfaces: 1 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xA0, //0xC0,   /* bmAttributes: self powered */
  0x64, //0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: One endpoints used */
  0xFF,   /* bInterfaceClass: Vendor Defined */
  0x00,   /* bInterfaceSubClass: N/A */
  0x00,   /* bInterfaceProtocol: N/A */
  0x00,   /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  LED_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(LED_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(LED_DATA_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  LED_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(LED_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(LED_DATA_MAX_PACKET_SIZE),
  0x00,                               /* bInterval: ignore for Bulk transfer */




} ;


/**
  * @}
  */ 

/** @defgroup USBD_LED_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_LED_Init
  *         Initialize the LED interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_LED_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t ret = 0;
  USBD_LED_HandleTypeDef   *hled;
  

    /* Open EP IN */
    USBD_LL_OpenEP(pdev,
                   LED_IN_EP,
                   USBD_EP_TYPE_BULK,
				   LED_DATA_MAX_PACKET_SIZE);
    
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev,
                   LED_OUT_EP,
                   USBD_EP_TYPE_BULK,
				   LED_DATA_MAX_PACKET_SIZE);
    
    
  pdev->pClassData = USBD_malloc(sizeof (USBD_LED_HandleTypeDef));
  
  if(pdev->pClassData == NULL)
  {
    ret = 1; 
  }
  else
  {
    hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
    
    /* Init  physical Interface components */
    ((USBD_LED_ItfTypeDef *)pdev->pUserData)->Init();
    
    //
    static uint8_t TxBuffer[64];
    static uint8_t RxBuffer[64];
    hled->TxBuffer = TxBuffer;
    hled->RxBuffer = RxBuffer;
    hled->RxLength = 64;
    hled->TxLength = 64;
    //


    /* Init Xfer states */
    hled->TxState =0;
    hled->RxState =0;
       
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
                           LED_OUT_EP,
                           hled->RxBuffer,
						 LED_DATA_MAX_PACKET_SIZE);
    
  }
  return ret;
}

/**
  * @brief  USBD_LED_Init
  *         DeInitialize the LED layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_LED_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Open EP IN */
  USBD_LL_CloseEP(pdev,
              LED_IN_EP);
  
  /* Open EP OUT */
  USBD_LL_CloseEP(pdev,
              LED_OUT_EP);
  
  
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
    ((USBD_LED_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return ret;
}

/**
  * @brief  USBD_LED_Setup
  *         Handle the LED specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_LED_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  static uint8_t ifalt = 0;
    
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :

	  /*  
    if (req->wLength)
    {
      if (req->bmRequest & 0x80)
      {
        ((USBD_LED_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)hled->data,
                                                          req->wLength);
          USBD_CtlSendData (pdev, 
                            (uint8_t *)hled->data,
                            req->wLength);
      }
      else
      {
        hled->CmdOpCode = req->bRequest;
        hled->CmdLength = req->wLength;
        
        USBD_CtlPrepareRx (pdev, 
                           (uint8_t *)hled->data,
                           req->wLength);
      }
      
    }
    else
    {
      ((USBD_LED_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                        (uint8_t*)req,
                                                        0);
    }


    */
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        &ifalt,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      break;
    }
 
  default: 
    break;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_LED_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_LED_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    
    hled->TxState = 0;

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_LED_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_LED_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  /* Get the received data length */
  hled->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  

  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the application Xfer */
  if(pdev->pClassData != NULL)
  {
    ((USBD_LED_ItfTypeDef *)pdev->pUserData)->Receive(hled->RxBuffer, &hled->RxLength);


    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev,
                           LED_OUT_EP,
                           hled->RxBuffer,
                           LED_DATA_OUT_PACKET_SIZE);

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}




/**
  * @brief  USBD_LED_GetFSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_LED_GetFSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_LED_CfgFSDesc);
  return USBD_LED_CfgFSDesc;
}



/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_LED_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_LED_DeviceQualifierDesc);
  return USBD_LED_DeviceQualifierDesc;
}

/**
* @brief  USBD_LED_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_LED_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_LED_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  
  if(fops != NULL)
  {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}

/**
  * @brief  USBD_LED_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_LED_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t length)
{
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  hled->TxBuffer = pbuff;
  hled->TxLength = length;  
  
  return USBD_OK;  
}


/**
  * @brief  USBD_LED_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_LED_SetRxBuffer  (USBD_HandleTypeDef   *pdev,
                                   uint8_t  *pbuff)
{
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  hled->RxBuffer = pbuff;
  
  return USBD_OK;
}

/**
  * @brief  USBD_LED_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  USBD_LED_TransmitPacket(USBD_HandleTypeDef *pdev)
{      
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    if(hled->TxState == 0)
    {
      /* Tx Transfer in progress */
      hled->TxState = 1;
      
      /* Transmit next packet */
      USBD_LL_Transmit(pdev,
                       LED_IN_EP,
                       hled->TxBuffer,
                       hled->TxLength);
      
      return USBD_OK;
    }
    else
    {
      return USBD_BUSY;
    }
  }
  else
  {
    return USBD_FAIL;
  }
}


/**
  * @brief  USBD_LED_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_LED_ReceivePacket(USBD_HandleTypeDef *pdev)
{      
  USBD_LED_HandleTypeDef   *hled = (USBD_LED_HandleTypeDef*) pdev->pClassData;
  
  /* Suspend or Resume USB Out process */
  if(pdev->pClassData != NULL)
  {

      /* Prepare Out endpoint to receive next packet */
      USBD_LL_PrepareReceive(pdev,
                             LED_OUT_EP,
                             hled->RxBuffer,
                             LED_DATA_OUT_PACKET_SIZE);

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

