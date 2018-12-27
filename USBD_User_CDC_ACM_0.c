/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2017 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    USBD_User_CDC_ACM_0.c
 * Purpose: USB Device Communication Device Class (CDC)
 *          Abstract Control Model (ACM) User module
 * Rev.:    V6.3.1
 *----------------------------------------------------------------------------*/
/**
 * \addtogroup usbd_cdcFunctions
 *
 * USBD_User_CDC_ACM_0.c implements the application specific functionality
 * of the CDC ACM class and is used to receive and send data to the USB Host.
 *
 * The implementation must match the configuration file USBD_Config_CDC_0.h.
 * The following values in USBD_Config_CDC_0.h affect the user code:
 *
 *  - 'Maximum Communication Device Send Buffer Size' specifies the maximum
 *    value for \em len in \ref USBD_CDC_ACM_WriteData
 *
 *  - 'Maximum Communication Device Receive Buffer Size' specifies the maximum
 *    value for \em len in \ref USBD_CDC_ACM_ReadData
 *
 */

//! [code_USBD_User_CDC_ACM]

#include <stdint.h>
#include <stdbool.h>
#include "rl_usb.h"

extern osThreadId T_Terminal_Thread;

uint8_t uart_tx_buf[64];
uint8_t uart_rx_buf[64];

// Local Variables
static CDC_LINE_CODING cdc_acm_line_coding = { 57600U, 1U, 2U, 8U };

// Called when new data was received from the USB Host.
// \param[in]   len           number of bytes available to read.
void USBD_CDC0_ACM_DataReceived(uint32_t len)
{
    int32_t cnt;
    (void)(len);
    cnt = USBD_CDC_ACM_ReadData(0U, uart_rx_buf, 64);
    if (cnt > 0) {
        osSignalSet(T_Terminal_Thread, 0x01);
    }
}
// Called during USBD_Initialize to initialize the USB CDC class instance (ACM).
void USBD_CDC0_ACM_Initialize(void)
{
    // Add code for initialization
}

// Called during USBD_Uninitialize to de-initialize the USB CDC class instance (ACM).
void USBD_CDC0_ACM_Uninitialize(void)
{
  // Add code for de-initialization
}
 
 
// Called upon USB Bus Reset Event.
void USBD_CDC0_ACM_Reset (void) {
  // Add code for reset
}
 
 
// Called upon USB Host request to change communication settings.
// \param[in]   line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          set line coding request processed.
// \return      false         set line coding request not supported or not processed.
bool USBD_CDC0_ACM_SetLineCoding (const CDC_LINE_CODING *line_coding) {
  // Add code for set line coding
 
  // Store requested settings to local variable
  cdc_acm_line_coding = *line_coding;
 
  return true;
}
 
 
// Called upon USB Host request to retrieve communication settings.
// \param[out]  line_coding   pointer to CDC_LINE_CODING structure.
// \return      true          get line coding request processed.
// \return      false         get line coding request not supported or not processed.
bool USBD_CDC0_ACM_GetLineCoding (CDC_LINE_CODING *line_coding) {
 
  // Load settings from ones stored on USBD_CDC0_ACM_SetLineCoding callback
  *line_coding = cdc_acm_line_coding;
 
  return true;
}
 
 
// Called upon USB Host request to set control line states.
// \param [in]  state         control line settings bitmap.
//                - bit 0: DTR state
//                - bit 1: RTS state
// \return      true          set control line state request processed.
// \return      false         set control line state request not supported or not processed.
bool USBD_CDC0_ACM_SetControlLineState (uint16_t state) {
  // Add code for set control line state
 
  (void)(state);
 
  return true;
}
 
//! [code_USBD_User_CDC_ACM]

/*
$ sudo stty -F /dev/ttyUSB0 raw
$ sudo stty -F /dev/ttyUSB0 -echo -echoe -echok
$ sudo stty -F /dev/ttyUSB0 115200
*/
