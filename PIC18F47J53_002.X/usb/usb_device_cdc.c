// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright 2015 Microchip Technology Inc. (www.microchip.com)

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
//DOM-IGNORE-END

/********************************************************************
 Change History:
  Rev    Description
  ----   -----------
  2.3    Deprecated the mUSBUSARTIsTxTrfReady() macro.  It is
         replaced by the USBUSARTIsTxTrfReady() function.

  2.6    Minor definition changes

  2.6a   No Changes

  2.7    Fixed error in the part support list of the variables section
         where the address of the CDC variables are defined.  The
         PIC18F2553 was incorrectly named PIC18F2453 and the PIC18F4558
         was incorrectly named PIC18F4458.

         http://www.microchip.com/forums/fb.aspx?m=487397

  2.8    Minor change to CDCInitEP() to enhance ruggedness in
         multi0-threaded usage scenarios.

  2.9b   Updated to implement optional support for DTS reporting.

********************************************************************/

/** I N C L U D E S **********************************************************/
#include "system.h"

#ifdef __USB_CDC


#include "usb.h"
#include "usb_device_cdc.h"

#ifdef USB_USE_CDC

#ifndef FIXED_ADDRESS_MEMORY
    #define IN_DATA_BUFFER_ADDRESS_TAG
    #define OUT_DATA_BUFFER_ADDRESS_TAG
    #define CONTROL_BUFFER_ADDRESS_TAG
    #define DRIVER_DATA_ADDRESS_TAG
#endif

#if !defined(IN_DATA_BUFFER_ADDRESS_TAG) || !defined(OUT_DATA_BUFFER_ADDRESS_TAG) || !defined(CONTROL_BUFFER_ADDRESS_TAG) || !defined(DRIVER_DATA_ADDRESS_TAG)
    #error "One of the fixed memory address definitions is not defined.  Please define the required address tags for the required buffers."
#endif

/** V A R I A B L E S ********************************************************/
volatile unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE] IN_DATA_BUFFER_ADDRESS_TAG;
volatile unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE] OUT_DATA_BUFFER_ADDRESS_TAG;

typedef union
{
    LINE_CODING lineCoding;
    CDC_NOTICE cdcNotice;
} CONTROL_BUFFER;

//static CONTROL_BUFFER controlBuffer CONTROL_BUFFER_ADDRESS_TAG;

LINE_CODING line_coding;    // ラインコーディング情報を格納するバッファ Buffer to store line coding information
CDC_NOTICE cdc_notice;

#if defined(USB_CDC_SUPPORT_DSR_REPORTING)
    SERIAL_STATE_NOTIFICATION SerialStatePacket DRIVER_DATA_ADDRESS_TAG;
#endif

uint8_t cdc_rx_len;            // total rx length
uint8_t cdc_trf_state;         // States are defined cdc.h
POINTER pCDCSrc;            // Dedicated source pointer
POINTER pCDCDst;            // Dedicated destination pointer
uint8_t cdc_tx_len;            // total tx length
uint8_t cdc_mem_type;          // _ROM, _RAM

USB_HANDLE CDCDataOutHandle;
USB_HANDLE CDCDataInHandle;


CONTROL_SIGNAL_BITMAP control_signal_bitmap;
uint32_t BaudRateGen;			// BRG value calculated from baud rate

#if defined(USB_CDC_SUPPORT_DSR_REPORTING)
    BM_SERIAL_STATE SerialStateBitmap;
    BM_SERIAL_STATE OldSerialStateBitmap;
    USB_HANDLE CDCNotificationInHandle;
#endif

/*
 *************************************************************************
 * SEND_ENCAPSULATED_COMMAND と GET_ENCAPSULATED_RESPONSE は、CDC 仕様に従って
 * 必須のリクエストです。 ただし、ここでは実際には使用されないため、適合性のために
 * ダミー バッファが使用されます。
 * 
 * SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required   
 * requests according to the CDC specification.   
 * However, it is not really being used here, therefore a dummy buffer is   
 * used for conformance.
 **************************************************************************/
#define dummy_length    0x08
uint8_t dummy_encapsulated_cmd_response[dummy_length];

#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
CTRL_TRF_RETURN USB_CDC_SET_LINE_CODING_HANDLER(CTRL_TRF_PARAMS);
#endif

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void USBCDCSetLineCoding(void);

/** D E C L A R A T I O N S **************************************************/
//#pragma code

/** C L A S S  S P E C I F I C  R E Q ****************************************/
/******************************************************************************
 * Function:
 *      void USBCheckCDCRequest(void)
 * 
 * 説明：
 * このルーチンは、最後に受信した SETUP データ パケットをチェックして、要求が 
 * CDC クラスに固有のものであるかどうかを確認します。 
 * リクエストが CDC 固有のリクエストである場合、この関数はリクエストの処理と適切な
 * 応答を処理します。
 * 
 * 前提条件:
 * この関数は、ホストから制御転送 SETUP パケットが到着した後にのみ呼び出す必要があります。
 * 
 * 備考：
 * SETUP パケットに CDC クラス固有の要求が含まれていない場合、この関数はステータスを
 * 変更したり、何も実行したりしません。
 *****************************************************************************/
void USBCheckCDCRequest(void)
{
    /*
     * If request recipient is not an interface then return
     */
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;

    /*
     * If request type is not class-specific then return
     */
    if(SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) return;

    /*
     * Interface ID must match interface numbers associated with
     * CDC class, else return
     */
    if((SetupPkt.bIntfID != CDC_COMM_INTF_ID)&&
       (SetupPkt.bIntfID != CDC_DATA_INTF_ID)) return;

    switch(SetupPkt.bRequest)
    {
        //****** These commands are required ******//
        case SEND_ENCAPSULATED_COMMAND:
         //send the packet
            inPipes[0].pSrc.bRam = (uint8_t*)&dummy_encapsulated_cmd_response;
            inPipes[0].wCount.Val = dummy_length;
            inPipes[0].info.bits.ctrl_trf_mem = USB_EP0_RAM;
            inPipes[0].info.bits.busy = 1;
            break;
        case GET_ENCAPSULATED_RESPONSE:
            // Populate dummy_encapsulated_cmd_response first.
            inPipes[0].pSrc.bRam = (uint8_t*)&dummy_encapsulated_cmd_response;
            inPipes[0].info.bits.busy = 1;
            break;
        //****** End of required commands ******//

        #if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1)
        case SET_LINE_CODING:
            outPipes[0].wCount.Val = SetupPkt.wLength;
            outPipes[0].pDst.bRam = (uint8_t*)LINE_CODING_TARGET;
            outPipes[0].pFunc = LINE_CODING_PFUNC;
            outPipes[0].info.bits.busy = 1;
            break;

        case GET_LINE_CODING:
            USBEP0SendRAMPtr(
                (uint8_t*)&line_coding,
                LINE_CODING_LENGTH,
                USB_EP0_INCLUDE_ZERO);
            break;

        case SET_CONTROL_LINE_STATE:
            control_signal_bitmap._byte = (uint8_t)SetupPkt.wValue;
            
            /******************************************************************
             * 
             * RTS ピンを制御する 1 つの方法は、USB ホストが RTS ピンに出力する値を
             * 決定できるようにすることです。 RTS および CTS ピンの機能は技術的には 
             * UART ハードウェア ベースのフロー制御を目的としていますが、一部のレガシー 
             * UART デバイスは RTS ピンを PC ホストからの「汎用」出力ピンのように
             * 使用します。 この使用モデルでは、RTS ピンは RX/TX のフロー制御には
             * 関係しません。
             * このシナリオでは、USB ホストは RTS ピンを制御できるようにする必要が
             * あるため、以下のコード行のコメントを解除する必要があります。
             * ただし、RX/TX ペアに真の RTS/CTS フロー制御を実装することが目的の場合、
             * このアプリケーション ファームウェアは USB ホストの RTS 設定をオーバー
             * ライドし、代わりに残りのバッファ スペースの量に基づいて実際の RTS信号を
             * 生成する必要があります。 このマイクロコントローラーの実際のハードウェア
             *  UART で使用できます。 この場合、以下のコードはコメントアウトしたままに
             * しておく必要がありますが、代わりに、このマイクロコントローラーのハード
             * ウェア UART の操作を担当するアプリケーション ファームウェアで RTS 
             * を制御する必要があります。
             * 
             * CONFIGURE_RTS(control_signal_bitmap.CARRIER_CONTROL);
             ******************************************************************/


            #if defined(USB_CDC_SUPPORT_DTR_SIGNALING)
                if(control_signal_bitmap.DTE_PRESENT == 1)
                {
                    UART_DTR = USB_CDC_DTR_ACTIVE_LEVEL;
                }
                else
                {
                    UART_DTR = (USB_CDC_DTR_ACTIVE_LEVEL ^ 1);
                }
            #endif
            inPipes[0].info.bits.busy = 1;
            break;
        #endif

        #if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2)
        case SEND_BREAK:                        // Optional
            inPipes[0].info.bits.busy = 1;
			if (SetupPkt.wValue == 0xFFFF)  //0xFFFF means send break indefinitely until a new SEND_BREAK command is received
			{
				UART_Tx = 0;       // Prepare to drive TX low (for break signaling)
				UART_TRISTx = 0;   // Make sure TX pin configured as an output
				UART_ENABLE = 0;   // Turn off USART (to relinquish TX pin control)
			}
			else if (SetupPkt.wValue == 0x0000) //0x0000 means stop sending indefinite break
			{
    			UART_ENABLE = 1;   // turn on USART
				UART_TRISTx = 1;   // Make TX pin an input
			}
			else
			{
                //Send break signaling on the pin for (SetupPkt.wValue) milliseconds
                UART_SEND_BREAK();
			}
            break;
        #endif
        default:
            break;
    }//end switch(SetupPkt.bRequest)

}//end USBCheckCDCRequest

/** U S E R  A P I ***********************************************************/

/**************************************************************************
 *  Function:
 *         void CDCInitEP(void)
 * まとめ：
 * この関数は CDC 関数ドライバーを初期化します。 この関数は、SET_CONFIGURATION 
 * コマンドの後に呼び出す必要があります (例: USBCBInitEP() 関数のコンテキスト内)。
 * 
 * 説明：
 * この関数は CDC 関数ドライバーを初期化します。 この関数は、デフォルトのライン 
 * コーディング (ボー レート、ビット パリティ、データ ビット数、およびフォーマット) 
 * を設定します。 この機能はエンドポイントも有効にし、ホストからの最初の転送の準備をします。
 * この関数は、SET_CONFIGURATION コマンドの後に呼び出す必要があります。 
 * これは、USBCBInitEP() 関数からこの関数を呼び出すことで最も簡単に実行できます。
 * 

    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            CDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None
  **************************************************************************/
void CDCInitEP(void)
{
    //Abstract line coding information
    line_coding.dwDTERate   = 19200;      // baud rate
    line_coding.bCharFormat = 0x00;             // 1 stop bit
    line_coding.bParityType = 0x00;             // None
    line_coding.bDataBits = 0x08;               // 5,6,7,8, or 16

    cdc_rx_len = 0;

    /*
     * ここで IN パイプの Cnt を初期化する必要はありません。
     * 理由：
     * ホストに送信するバイト数はトランザクションごとに異なります。 Cnt は、特定の 
     * IN トランザクションで送信するバイトの正確な数と等しくなければなりません。 
     * このバイト数は、データが送信される直前にのみわかります。
     * 
     */
    USBEnableEndpoint(CDC_COMM_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBEnableEndpoint(CDC_DATA_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));
    CDCDataInHandle = NULL;

    #if defined(USB_CDC_SUPPORT_DSR_REPORTING)
      	CDCNotificationInHandle = NULL;
        mInitDTSPin();  //Configure DTS as a digital input
      	SerialStateBitmap.byte = 0x00;
      	OldSerialStateBitmap.byte = !SerialStateBitmap.byte;    //To force firmware to send an initial serial state packet to the host.
        //Prepare a SerialState notification element packet (contains info like DSR state)
        SerialStatePacket.bmRequestType = 0xA1; //Always 0xA1 for this type of packet.
        SerialStatePacket.bNotification = SERIAL_STATE;
        SerialStatePacket.wValue = 0x0000;  //Always 0x0000 for this type of packet
        SerialStatePacket.wIndex = CDC_COMM_INTF_ID;  //Interface number
        SerialStatePacket.SerialState.byte = 0x00;
        SerialStatePacket.Reserved = 0x00;
        SerialStatePacket.wLength = 0x02;   //Always 2 bytes for this type of packet
        CDCNotificationHandler();
  	#endif

  	#if defined(USB_CDC_SUPPORT_DTR_SIGNALING)
  	    mInitDTRPin();
  	#endif

  	#if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
  	    mInitRTSPin();
  	    mInitCTSPin();
  	#endif

    cdc_trf_state = CDC_TX_READY;
}//end CDCInitEP


/**************************************************************************
  Function: void CDCNotificationHandler(void)
 * まとめ：
 * DSR ステータスの変化をチェックし、USB ホストに報告します。 
 * 
 * 説明：
 * DSR ピンの状態の変化をチェックし、変化があれば USB ホストに報告します。 
 * 条件: CDCInitEP() は、初めて CDCNotificationHandler() を呼び出す前に、
 * 事前に呼び出されている必要があります。
 * 
 * 備考：
 * この関数は、USB_CDC_SUPPORT_DSR_REPORTING オプションが有効になっている場合にのみ
 * 実装され、必要になります。 この機能が有効な場合は、DSR ピンをサンプリングして情報を
 *  USB ホストに供給するために定期的に呼び出す必要があります。 
 * これは、CDCNotificationHandler() を単独で呼び出すか、必要に応じて 
 * CDCNotificationHandler() を内部的に呼び出す CDCTxService() を呼び出すことに
 * よって実行できます。
 * 
  **************************************************************************/
#if defined(USB_CDC_SUPPORT_DSR_REPORTING)
void CDCNotificationHandler(void)
{
    //Check the DTS I/O pin and if a state change is detected, notify the
    //USB host by sending a serial state notification element packet.
    if(UART_DTS == USB_CDC_DSR_ACTIVE_LEVEL) //UART_DTS must be defined to be an I/O pin in the hardware profile to use the DTS feature (ex: "PORTXbits.RXY")
    {
        SerialStateBitmap.bits.DSR = 1;
    }
    else
    {
        SerialStateBitmap.bits.DSR = 0;
    }

    //If the state has changed, and the endpoint is available, send a packet to
    //notify the hUSB host of the change.
    if((SerialStateBitmap.byte != OldSerialStateBitmap.byte) && (!USBHandleBusy(CDCNotificationInHandle)))
    {
        //Copy the updated value into the USB packet buffer to send.
        SerialStatePacket.SerialState.byte = SerialStateBitmap.byte;
        //We don't need to write to the other bytes in the SerialStatePacket USB
        //buffer, since they don't change and will always be the same as our
        //initialized value.

        //Send the packet over USB to the host.
        CDCNotificationInHandle = USBTransferOnePacket(CDC_COMM_EP, IN_TO_HOST, (uint8_t*)&SerialStatePacket, sizeof(SERIAL_STATE_NOTIFICATION));

        //Save the old value, so we can detect changes later.
        OldSerialStateBitmap.byte = SerialStateBitmap.byte;
    }
}//void CDCNotificationHandler(void)
#else
    #define CDCNotificationHandler() {}
#endif


/**********************************************************************************
 * 説明：
 * USB スタックからのイベントを処理します。 この関数は、CDC ドライバーによって処理
 * する必要がある USB イベントがあるときに呼び出す必要があります。
 * 
 * 条件：
 * 入力引数「len」の値は、CDC クラスの USB ホストからのバルク データの受信を担当する
 * 最大エンドポイント サイズより小さくする必要があります。 
 * 入力引数 'buffer' は、'len' で指定されたサイズ以上のバッファ領域を指す必要があります。
 * 
 * 入力：
 * イベント - 発生したイベントのタイプ
 * pdata - イベントを引き起こしたデータへのポインタ
 * size - pdata が指すデータのサイズ
 * 
 *********************************************************************************/
bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size)
{
    switch( (uint16_t)event )
    {
        case EVENT_TRANSFER_TERMINATED:
            if(pdata == CDCDataOutHandle)
            {
                CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));
            }
            if(pdata == CDCDataInHandle)
            {
                //flush all of the data in the CDC buffer
                cdc_trf_state = CDC_TX_READY;
                cdc_tx_len = 0;
            }
            break;
        default:
            return false;
    }
    return true;
}

/*******************************************************************************
 * Function:
 *         uint8_t getsUSBUSART(char *buffer, uint8_t len)
 * 
 * まとめ：
 * getUSBUSART は、USB CDC Bulk OUT エンドポイント経由で受信した BYTE の文字列を
 * ユーザーの指定した場所にコピーします。 ノンブロッキング関数です。 利用可能なデータが
 * ない場合、データを待ちません。 
 * 代わりに「0」を返し、利用可能なデータがないことを呼び出し元に通知します。
 * 
 * 説明：
 * getUSBUSART は、USB CDC Bulk OUT エンドポイント経由で受信した BYTE の文字列を
 * ユーザーの指定した場所にコピーします。 ノンブロッキング関数です。 
 * 利用可能なデータがない場合、データを待ちません。 
 * 代わりに「0」を返し、利用可能なデータがないことを呼び出し元に通知します。
 * 

    Typical Usage:
    <code>
        uint8_t numBytes;
        uint8_t buffer[64]

        numBytes = getsUSBUSART(buffer,sizeof(buffer)); //until the buffer is free.
        if(numBytes \> 0)
        {
            //we received numBytes bytes of data and they are copied into
            //  the "buffer" variable.  We can do something with the data
            //  here.
        }
    </code>
 * 
 * 条件：
 * 入力引数「len」の値は、CDC クラスの USB ホストからのバルク データの受信を担当する
 * 最大エンドポイント サイズより小さくする必要があります。 
 * 入力引数 'buffer' は、'len' で指定されたサイズ以上のバッファ領域を指す必要があります。
 * 
 * 入力：
 * buffer - 受信したBYTEが格納される場所へのポインタ
 * len - 予期されるバイト数。
 * 
 *********************************************************************************/
uint8_t getsUSBUSART(uint8_t *buffer, uint8_t len)
{
    cdc_rx_len = 0;

    if(!USBHandleBusy(CDCDataOutHandle))
    {
        /*
         * Adjust the expected number of BYTEs to equal
         * the actual number of BYTEs received.
         */
        if(len > USBHandleGetLength(CDCDataOutHandle))
            len = USBHandleGetLength(CDCDataOutHandle);

        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for(cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
            buffer[cdc_rx_len] = cdc_data_rx[cdc_rx_len];

        /*
         * Prepare dual-ram buffer for next OUT transaction
         */

        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_rx,sizeof(cdc_data_rx));

    }//end if

    return cdc_rx_len;

}//end getsUSBUSART

/******************************************************************************
 * Function:
 * 	void putUSBUSART(char *data, uint8_t length)
 * 
 * まとめ：
 * putUSBUSART は、データの配列を USB に書き込みます。 このバージョンを使用すると、
 * 0x00 (通常、文字列転送関数の NULL 文字) を転送できます。
 * 
 * 説明：
 * putUSBUSART は、データの配列を USB に書き込みます。 このバージョンを使用すると、
 * 0x00 (通常、文字列転送関数の NULL 文字) を転送できます。
 * 

    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
            putUSBUSART(data,5);
        }
    </code>

 * 
 * デバイスからホスト(put)への転送メカニズムは、ホストからデバイス(get)よりも柔軟です。
 *  バルク IN エンドポイントの最大サイズを超えるデータ文字列を処理できます。 
 * ステート マシンは、\長いデータ文字列を複数の USB トランザクションで転送するために
 * 使用されます。 データのブロックをホストに送信し続けるには、CDCTxService() を
 * 定期的に呼び出す必要があります。
 * 
 * 条件：
 * USBUSARTIsTxTrfReady() は true を返す必要があります。 これは、最後の転送が完了し、
 * 新しいデータ ブロックを受信する準備ができていることを示します。 
 * 「data」が指す文字列は 255 バイト以下である必要があります。
 * 
 * 入力：
 *  char *data - ホストに転送されるデータの RAM 配列へのポインタ
 *   uint8_t length - 転送されるバイト数 (255 未満である必要があります)。
 * 
***************************************************************************/
void putUSBUSART(uint8_t *data, uint8_t  length)
{
    /*
     * この関数を呼び出す前に、cdc_trf_state が CDC_TX_READY 状態であることを確認する
     * 必要があります。 安全対策として、この関数は状態をもう一度チェックして、保留中の
     * トランザクションをオーバーライドしないことを確認します。
     * 現時点では、ユーザーにエラーを報告せずにルーチンを終了するだけです。
     * 
     * 例：
     * if(USBUSARTIsTxTrfReady())
     *      putUSBUSART(pData, 長さ);
     * 
     * 重要: 待機するために次のブロッキング while ループを決して使用しないでください。
     * while(!USBUSARTIsTxTrfReady())
     *    putUSBUSART(pData, 長さ);
     * 
     * ファームウェア フレームワーク全体は協調的なマルチタスクに基づいて記述されており、
     * ブロッキング コードは受け入れられません。
     * 代わりにステート マシンを使用してください。
     */
    USBMaskInterrupts();
    if(cdc_trf_state == CDC_TX_READY)
    {
        mUSBUSARTTxRam((uint8_t*)data, length);     // See cdc.h
    }
    USBUnmaskInterrupts();
}//end putUSBUSART

/*****************************************************************************
 * Function:
 * 		void putsUSBUSART(char *data)
 * 
 * まとめ：
 * putUSBUSART は、ヌル文字を含むデータ文字列を USB に書き込みます。 
 * RAM バッファからデータを転送するには、このバージョン「puts」を使用します。
 * 
 * 説明：
 * putUSBUSART は、ヌル文字を含むデータ文字列を USB に書き込みます。 RAM バッファから
 * データを転送するには、このバージョン「puts」を使用します。
 * 
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = "Hello World";
            putsUSBUSART(data);
        }
    </code>

 * 
 * デバイスからホスト(put)への転送メカニズムは、ホストからデバイス(get)よりも柔軟です。 
 * バルク IN エンドポイントの最大サイズを超えるデータ文字列を処理できます。 
 * ステート マシンは、\長いデータ文字列を複数の USB トランザクションで転送するために
 * 使用されます。 データのブロックをホストに送信し続けるには、CDCTxService() を
 * 定期的に呼び出す必要があります。
 * 
 * 条件：
 * USBUSARTIsTxTrfReady() は true を返す必要があります。 これは、最後の転送が完了し、
 * 新しいデータ ブロックを受信する準備ができていることを示します。 
 * 「data」が指す文字列は 255 バイト以下である必要があります。
 * 
 * 入力：null で終了する定数データの文字列。 NULL 文字が見つからない場合、255 バイトの
 *       データがホストに転送されます。
 *  char *data - 
 * 
 *****************************************************************************/
void Wait2TxRedy(void)
{
    while(cdc_trf_state != CDC_TX_READY)
    {
        
    }
}
void putsUSBUSART(char *data)
{
    uint8_t len;
    char *pData;

    /*
     * この関数を呼び出す前に、cdc_trf_state が CDC_TX_READY 状態であることを確認
     * する必要があります。 安全対策として、この関数は状態をもう一度チェックして、
     * 保留中のトランザクションをオーバーライドしないことを確認します。
     * 
     * 現時点では、ユーザーにエラーを報告せずにルーチンを終了するだけです。
     * 
     * 結論:
     * ユーザーは、この関数を呼び出す前に USBUSARTIsTxTrfReady()==1 であることを
     * 確認する必要があります。 例：
     * 
     * 例：
     * if(USBUSARTIsTxTrfReady())
     *      putUSBUSART(pData, 長さ);
     * 
     * 重要: 
     * 待機するために次のブロッキング while ループを決して使用しないでください。
     * while(!USBUSARTIsTxTrfReady())
     *      putUSBUSART(pData);
     * 
     * ファームウェア フレームワーク全体は協調的なマルチタスクに基づいて記述されており、
     * ブロッキング コードは受け入れられません。 代わりにステート マシンを使用してください。
     */
    
    USBMaskInterrupts();
    if(cdc_trf_state != CDC_TX_READY)
    {
        USBUnmaskInterrupts();
        return;
    }

    /*
     * While ループは、NULL 文字を含む送信する BYTE の数をカウントします。
     */
    len = 0;
    pData = data;
    do
    {
        len++;
        if(len == 255) break;       // Break loop once max len is reached.
    }while(*pData++);

    /*
     * 2 番目の情報 (送信するデータの長さ) が準備できました。
     * mUSBUSARTTxRam を呼び出して転送をセットアップします。
     * 実際の転送プロセスは CDCTxService() によって処理され、メイン プログラム ループごとに 1 回呼び出す必要があります。
     */
    mUSBUSARTTxRam((uint8_t*)data, len);     // See cdc.h
    USBUnmaskInterrupts();
}//end putsUSBUSART

/**************************************************************************
 *  Function:
 *         void putrsUSBUSART(const const char *data)
 * 
 * まとめ：
 * putrsUSBUSART は、ヌル文字を含むデータ文字列を USB に書き込みます。 
 * このバージョンの「putrs」を使用して、データ リテラルとプログラム メモリにあるデータを
 * 転送します。
 * 
 * 説明：
 * putrsUSBUSART は、ヌル文字を含むデータ文字列を USB に書き込みます。 
 * このバージョンの「putrs」を使用して、データ リテラルとプログラム メモリにあるデータ
 * を転送します。

    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("Hello World");
        }
    </code>

 * デバイスからホスト(put)への転送メカニズムは、ホストからデバイス(get)よりも柔軟です。 
 * バルク IN エンドポイントの最大サイズを超えるデータ文字列を処理できます。 
 * ステート マシンは、\長いデータ文字列を複数の USB トランザクションで転送するために
 * 使用されます。 データのブロックをホストに送信し続けるには、CDCTxService() を
 * 定期的に呼び出す必要があります。
 * 
 * 条件：
 * USBUSARTIsTxTrfReady() は true を返す必要があります。 
 * これは、最後の転送が完了し、新しいデータ ブロックを受信する準備ができていることを
 * 示します。 「data」が指す文字列は 255 バイト以下である必要があります。
 * 
 * Input:
 *  const const char *data
 * 
 * null で終わる定数データの文字列。 NULL 文字が見つからない場合、255 uint8_ts の
 * データがホストに転送されます。
 * 
 **************************************************************************/
void putrsUSBUSART(const char *data)
{
    uint8_t len;
    const char *pData;

    /*
     * この関数を呼び出す前に、cdc_trf_state が CDC_TX_READY 状態であることを確認
     * する必要があります。 安全対策として、この関数は状態をもう一度チェックして、
     * 保留中のトランザクションをオーバーライドしないことを確認します。
     * 
     * 現時点では、ユーザーにエラーを報告せずにルーチンを終了するだけです。
     * 
     * 結論: ユーザーは、この関数を呼び出す前に USBUSARTIsTxTrfReady() を確認する必要があります.
     *
     * 例：
     * if(USBUSARTIsTxTrfReady())
     *      putUSBUSART(pData);
     * 
     * 重要: 
     * 待機するために次のブロッキング while ループを決して使用しないでください。
     * while(cdc_trf_state != CDC_TX_READY)
     *      putUSBUSART(pData);
     * 
     * ファームウェア フレームワーク全体は協調的なマルチタスクに基づいて記述されており、
     * ブロッキング コードは受け入れられません。
     * 代わりにステート マシンを使用してください。
     */
    USBMaskInterrupts();
    if(cdc_trf_state != CDC_TX_READY)
    {
        USBUnmaskInterrupts();
        return;
    }

    /*
     * While loop counts the number of BYTEs to send including the
     * null character.
     */
    len = 0;
    pData = data;
    do
    {
        len++;
        if(len == 255) break;       // Break loop once max len is reached.
    }while(*pData++);

    /*
     * 2 番目の情報 (送信するデータの長さ) が準備できました
     * mUSBUSARTTxRom を呼び出して転送をセットアップします。
     * 実際の転送プロセスは CDCTxService() によって処理され、メイン プログラム 
     * ループごとに 1 回呼び出す必要があります。
     */

    mUSBUSARTTxRom((const uint8_t*)data,len); // See cdc.h
    USBUnmaskInterrupts();

}//end putrsUSBUSART

/************************************************************************
 * Function:   void CDCTxService(void)
 * 
 * まとめ：
 * CDCTxService は、デバイスからホストへのトランザクションを処理します。 
 * この関数は、デバイスが構成された状態に達した後、メイン プログラム ループごとに 
 * 1 回呼び出される必要があります。
 * 
 * 説明：
 * CDCTxService は、デバイスからホストへのトランザクションを処理します。 
 * この関数は、デバイスが設定された状態に達した後 (CDCIniEP() 関数が既に実行された後)、
 * メイン プログラム ループごとに 1 回呼び出される必要があります。 
 * この機能は、CDC シリアル データに関連付けられた複数のトランザクション相当の 
 * IN USB データをホストに送信する処理を行うマシンの内部ソフトウェア状態を進める
 * ために必要です。 CDCTxService() を定期的に呼び出さないと、CDC シリアル データ 
 * インターフェイス経由でデータが USB ホストに送信されなくなります。
 * 

    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == true))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep trying to send data to the PC as required
                CDCTxService();

                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    CDCIniEP() function should have already executed/the device should be
    in the CONFIGURED_STATE.
  Remarks:
    None
  ************************************************************************/

void CDCTxService(void)
{
    uint8_t byte_to_send;
    uint8_t i;

    USBMaskInterrupts();

    CDCNotificationHandler();

    /*
     * #define USBHandleBusy(handle) ((handle != 0x0000) && ((*(volatile uint8_t*)handle & _USIE) != 0x00))
     */
    
    //CDCDataInHandle != 0x0000 && (CDCDataInHandle & _USIE) != 0x00
    
    if(USBHandleBusy(CDCDataInHandle))
    {
        //printf("CDCTxService()001\r\n");
        USBUnmaskInterrupts();
        return;
    }

    /*
     * [ mCDCUSartTxIsBusy()==1 ] の間にステージを完了する必要があります。 
     * この段階があることにより、ユーザーはいつでも cdc_trf_state をチェックでき、
     * mCDCUsartTxIsBusy() を直接呼び出す必要がなくなります。
     * 
     * Completing stage is necessary while [ mCDCUSartTxIsBusy()==1 ].
     * By having this stage, user can always check cdc_trf_state,
     * and not having to call mCDCUsartTxIsBusy() directly.
     */
    if(cdc_trf_state == CDC_TX_COMPLETING)
        cdc_trf_state = CDC_TX_READY;

    /*
     * CDC_TX_READY 状態の場合は、何もせずに戻ります。
     * If CDC_TX_READY state, nothing to do, just return.
     */
    if(cdc_trf_state == CDC_TX_READY)
    {
        //printf("CDCTxService()002\r\n");
        USBUnmaskInterrupts();
        return;
    }

    /*
     * CDC_TX_BUSY_ZLP 状態の場合、長さゼロのパケットを送信
     * If CDC_TX_BUSY_ZLP state, send zero length packet
     */
    if(cdc_trf_state == CDC_TX_BUSY_ZLP)
    {
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,NULL,0);
        //CDC_DATA_BD_IN.CNT = 0;
        cdc_trf_state = CDC_TX_COMPLETING;
    }
    else if(cdc_trf_state == CDC_TX_BUSY)
    {
        /*
         * まず、送信するデータのバイト数を把握する必要があります。
         * First, have to figure out how many byte of data to send.
         */
    	if(cdc_tx_len > sizeof(cdc_data_tx))
    	    byte_to_send = sizeof(cdc_data_tx);
    	else
    	    byte_to_send = cdc_tx_len;

        /*
         * 合計から送信しようとしているバイト数を引きます。
         * Subtract the number of bytes just about to be sent from the total.
         */
    	cdc_tx_len = cdc_tx_len - byte_to_send;

        // 宛先ポインタを設定するSet destination pointer
        pCDCDst.bRam = (uint8_t*)&cdc_data_tx; 
        
        i = byte_to_send;
        if(cdc_mem_type == USB_EP0_ROM)            // メモリソースのタイプを決定する Determine type of memory source
        {
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRom;
                pCDCDst.bRam++;
                pCDCSrc.bRom++;
                i--;
            }//end while(byte_to_send)
        }
        else
        {
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRam;
                pCDCDst.bRam++;
                pCDCSrc.bRam++;
                i--;
            }
        }

        /*
         * 最後に、長さゼロのパケット状態が必要かどうかを判断します。 
         * USB 仕様 2.0: セクション 5.8.3 の説明を参照してください。
         * Lastly, determine if a zero length packet state is necessary.
         * See explanation in USB Specification 2.0: Section 5.8.3
         */
        if(cdc_tx_len == 0)
        {
            if(byte_to_send == CDC_DATA_IN_EP_SIZE)
                cdc_trf_state = CDC_TX_BUSY_ZLP;
            else
                cdc_trf_state = CDC_TX_COMPLETING;
        }//end if(cdc_tx_len...)
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_tx,byte_to_send);

    }//end if(cdc_tx_sate == CDC_TX_BUSY)

    USBUnmaskInterrupts();
}//end CDCTxService


#ifdef ___NOP
void CDCTxService22(void)
{
    uint8_t byte_to_send;
    uint8_t i;

    USBMaskInterrupts();

    CDCNotificationHandler();

printf("CDCTxService()001\r\n");

    if(USBHandleBusy(CDCDataInHandle))
    {
printf("CDCTxService()002\r\n");
        USBUnmaskInterrupts();
        return;
    }

    /*
     * Completing stage is necessary while [ mCDCUSartTxIsBusy()==1 ].
     * By having this stage, user can always check cdc_trf_state,
     * and not having to call mCDCUsartTxIsBusy() directly.
     */
    if(cdc_trf_state == CDC_TX_COMPLETING){
        cdc_trf_state = CDC_TX_READY;
printf("CDCTxService()003\r\n");
    }

    /*
     * If CDC_TX_READY state, nothing to do, just return.
     */
    if(cdc_trf_state == CDC_TX_READY)
    {
printf("CDCTxService()004\r\n");
        //printf("CDCTxService()002\r\n");
        USBUnmaskInterrupts();
        return;
    }

    /*
     * If CDC_TX_BUSY_ZLP state, send zero length packet
     */
    if(cdc_trf_state == CDC_TX_BUSY_ZLP)
    {
printf("CDCTxService()005\r\n");
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,NULL,0);
        //CDC_DATA_BD_IN.CNT = 0;
        cdc_trf_state = CDC_TX_COMPLETING;
    }
    else if(cdc_trf_state == CDC_TX_BUSY)
    {
printf("CDCTxService()006\r\n");
        /*
         * First, have to figure out how many byte of data to send.
         */
    	if(cdc_tx_len > sizeof(cdc_data_tx))
    	    byte_to_send = sizeof(cdc_data_tx);
    	else
    	    byte_to_send = cdc_tx_len;

        /*
         * Subtract the number of bytes just about to be sent from the total.
         */
    	cdc_tx_len = cdc_tx_len - byte_to_send;

        pCDCDst.bRam = (uint8_t*)&cdc_data_tx; // Set destination pointer

        i = byte_to_send;
        if(cdc_mem_type == USB_EP0_ROM)            // Determine type of memory source
        {
printf("CDCTxService()007\r\n");
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRom;
                pCDCDst.bRam++;
                pCDCSrc.bRom++;
                i--;
            }//end while(byte_to_send)
        }
        else
        {
printf("CDCTxService()008\r\n");
            while(i)
            {
                *pCDCDst.bRam = *pCDCSrc.bRam;
                pCDCDst.bRam++;
                pCDCSrc.bRam++;
                i--;
            }
        }

        /*
         * Lastly, determine if a zero length packet state is necessary.
         * See explanation in USB Specification 2.0: Section 5.8.3
         */
        if(cdc_tx_len == 0)
        {
printf("CDCTxService()009\r\n");
            if(byte_to_send == CDC_DATA_IN_EP_SIZE)
                cdc_trf_state = CDC_TX_BUSY_ZLP;
            else
                cdc_trf_state = CDC_TX_COMPLETING;
        }//end if(cdc_tx_len...)
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,(uint8_t*)&cdc_data_tx,byte_to_send);

    }//end if(cdc_tx_sate == CDC_TX_BUSY)
printf("CDCTxService()010\r\n");

    USBUnmaskInterrupts();
}//end CDCTxService
#endif  // ___NOP


#endif //USB_USE_CDC

#endif // __USB_CDC

#ifdef __USB_CDC
#endif // __USB_CDC


/** EOF cdc.c ****************************************************************/
