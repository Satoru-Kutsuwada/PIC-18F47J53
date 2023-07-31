/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

/** INCLUDES *******************************************************/
#include "system.h"


#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "usb.h"

#include "app_led_usb_status.h"
#include "app_device_cdc_basic.h"
#include "usb_config.h"

/** VARIABLES ******************************************************/

#ifdef __USB_CDC

static bool buttonPressed;
static char buttonMessage[] = "Button pressed.\r\n";
static uint8_t readBuffer[CDC_DATA_OUT_EP_SIZE];
static uint8_t writeBuffer[CDC_DATA_IN_EP_SIZE];
static uint8_t putc_writeBuffer[CDC_DATA_IN_EP_SIZE];
static uint8_t putc_writeBufptr=0;


void print_usb(char *pt);
extern  uint8_t main_status;
/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceCDCBasicDemoInitialize()
{   
    line_coding.bCharFormat = 0;
    line_coding.bDataBits = 8;
    line_coding.bParityType = 0;
    line_coding.dwDTERate = 9600;

    buttonPressed = false;
}

typedef enum{
    USB_STATE_UNKWON,
    USB_STATE_NOT_CONECT,
    USB_STATE_CONECT,
            
}USB_STATE_FLG;

USB_STATE_FLG usb_state_flg = USB_STATE_UNKWON;
/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceCDCBasicDemoInitialize() and APP_DeviceCDCBasicDemoStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void Wait2Connect(void)
{
    while( USBGetDeviceState() < CONFIGURED_STATE )
    {
        
    }
}

#ifdef __PRINTF_USB
    
int getch(void)
{
    uint8_t numBytesRead;
    
    numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
    
    if( numBytesRead > 0 ){
        return (int) readBuffer[0]; // 受信データを返す
    }
    else{
        return (char)0;
        
    }
}

void putch(char c)
{    
        uint8_t i;
        
    if( main_status == 0 )
        return;
        
        
        
    putc_writeBuffer[putc_writeBufptr] = c;
    putc_writeBufptr++;
    
    if( putc_writeBufptr == (CDC_DATA_IN_EP_SIZE-1) ){
        putc_writeBuffer[putc_writeBufptr] = '\0';
        putc_writeBufptr = 0;
        
        for( i=0; i<CDC_DATA_IN_EP_SIZE ;i++ ){
           writeBuffer[i] = putc_writeBuffer[i];
        }
        
        while(!USBUSARTIsTxTrfReady()){
            CDCTxService();
        }        
        putsUSBUSART(writeBuffer);
        CDCTxService();
    }
    else{
        putc_writeBuffer[putc_writeBufptr] = '\0';
    }
}
#endif   


void APP_DeviceCDCBasicDemoTasks()
{
        uint8_t i;
    /***************************************************************************
     * USB デバイスがまだ設定されていない場合は、通信するホストがないため、
     * 他に何もすることができません。 したがって、while ループの先頭に戻ります。
     **************************************************************************/
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        if( usb_state_flg == USB_STATE_UNKWON){
            printf("USB NOT CONECT\r\n");
            usb_state_flg = USB_STATE_NOT_CONECT;
        }
        return;
    }

    if( usb_state_flg == USB_STATE_NOT_CONECT){
            printf("USB CONECT\r\n");
            usb_state_flg = USB_STATE_CONECT;
    }

    /***************************************************************************
     * 現在一時停止されている場合は、リモート ウェイクアップを発行する必要があるか
     * どうかを確認する必要があります。
     **************************************************************************/
    if( USBIsDeviceSuspended()== true )
    {
        printf("USB SUSPENDT\r\n");
        return;
    }

#ifdef ____NOP
    /* If the user has pressed the button associated with this demo, then we
     * are going to send a "Button Pressed" message to the terminal.
     */
    if(BUTTON_IsPressed(BUTTON_DEVICE_CDC_BASIC_DEMO) == true)
    {
        /* Make sure that we only send the message once per button press and
         * not continuously as the button is held.
         */
        if(buttonPressed == false)
        {
            /* Make sure that the CDC driver is ready for a transmission.
             */
            if(mUSBUSARTIsTxTrfReady() == true)
            {
                putrsUSBUSART(buttonMessage);
                buttonPressed = true;
            }
        }
    }
    else
    {
        /* If the button is released, we can then allow a new message to be
         * sent the next time the button is pressed.
         */
        buttonPressed = false;
    }
    
    /***************************************************************************
     * 進行中の送信があるかどうかを確認し、そうでない場合は、受信したデータに対する
     * エコー応答の実行について確認できます。
     **************************************************************************/
    if( USBUSARTIsTxTrfReady() == true)
    {
        uint8_t i;
        uint8_t numBytesRead;

        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));

        if( numBytesRead > 0 ){
            
            /* For every byte that was read... */
            for(i=0; i<numBytesRead; i++)
            {
                switch(readBuffer[i])
                {
                    /* If we receive new line or line feed commands, just echo
                     * them direct.
                     */
                    case 0x0A:
                    case 0x0D:
                        writeBuffer[i] = readBuffer[i];
                        break;

                    /* If we receive something else, then echo it plus one
                     * so that if we receive 'a', we echo 'b' so that the
                     * user knows that it isn't the echo enabled on their
                     * terminal program.
                     */
                    default:
                        writeBuffer[i] = readBuffer[i];
                        break;
                }
            }
        }

        if(numBytesRead > 0)
        {
            /*******************************************************************
             * 受信したデータをすべて処理した後、「エコー」データを送信する必要があります。
             ******************************************************************/
            putUSBUSART(writeBuffer,numBytesRead);
        }
    }
#endif

    /*
     * 送信バッファーへ転送
     */
    
    if(USBUSARTIsTxTrfReady()){
        if( putc_writeBufptr > 0 ){        
            for( i=0; i<CDC_DATA_IN_EP_SIZE ;i++ ){
               if( putc_writeBuffer[i] != '\0' ){
                   writeBuffer[i] = putc_writeBuffer[i];           
               }
               else{
                    writeBuffer[i] = '\0';
                    putc_writeBufptr = 0;
                    break;
               }
            }
            putUSBUSART(writeBuffer,i);
        }
    }


    CDCTxService();
}
#endif // __USB_CDC


void print_usb(char *pt)
{
#ifdef __USB_CDC
    putrsUSBUSART(pt);
    CDCTxService();
#endif // __USB_CDC
}