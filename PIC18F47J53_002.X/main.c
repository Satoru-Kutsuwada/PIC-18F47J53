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

#ifdef __USB_CDC
#endif  // __USB_CDC


#include "main.h"

#include "app_device_cdc_basic.h"
#include "app_led_usb_status.h"

#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"
#include "rtc_main.h"


/** External ProtoType *******************************************************/

extern void USB_start_dispg(void);

        
extern void uart_init(void);
extern void uart485_init(void);
extern void uart_start_dispg(void);
extern void uart485_start_dispg(void);
extern void main_loop_disp(void);
extern void USB_start_dispg(void);

#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern void i2c_test(void);
extern void i2c_init(uint8_t sel);
#endif

extern  void Init_Timer(void);
extern  void print_usb(char *pt);    
extern   void debug_main(void);
extern  void debug_main485(void);
 

extern void VL53_init(void);
extern void vl53l0x_test(void);
extern void Clear1secTaskFlg(void);
extern bool Check1secTask(void);
extern void log_init(void);

void rs485_com_task(void);

/** Local ProtoType *******************************************************/

//void Wait(uint16_t num);
void Timer0_init(void);
void Timer1_init(void);

bool Check100msTask(void);
void Clear100msTaskFlg(void);
void rtc_data_init(void);

/** External Valiabl *******************************************************/
extern  uint16_t i2c_sens_cnt;    

uint8_t main_status =0;


//=============================================================================
//
//=============================================================================
void T1secMainTask(void)
{
#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if( i2c_sens_cnt != 0 )
    {
        i2c_sens_cnt --;
        i2c_test();
    }    
#endif
}
//=============================================================================
//
//=============================================================================
void T100msecMainTask(void)
{
    char string[50];
    RTC_DATA rtc;
        
    if( PORTDbits.RD2==0 ){
        printf("Button2\r\n");

        if( PORTAbits.RA0 )
            LATAbits.LATA0 = 0;
        else
            LATAbits.LATA0 = 1;


        sprintf(string,"I2C regista display\r\n");
        print_usb(string);
        sprintf(string,"  SCL=%x\r\n",I2C_SCL  );
        print_usb(string);
        sprintf(string,"  SDA=%x\r\n",I2C_SDA  );
        print_usb(string);
        sprintf(string,"  SSP2STAT=%x\r\n",I2C_SSPSTAT);
        print_usb(string);
        sprintf(string,"  SSP2CON1=%x\r\n",I2C_SSPCON1);
        print_usb(string);
        sprintf(string,"  SSP2CON2=%x\r\n",I2C_SSPCON2);
        print_usb(string);
        sprintf(string,"  SSP2ADD=%x\r\n",I2C_SSPADD );
        print_usb(string);


    }

    if( PORTDbits.RD3==0 ){
        if( PORTAbits.RA1 )
            LATAbits.LATA1 = 0;
        else
            LATAbits.LATA1 = 1;

        printf("Button1\r\n");
        Rtc_Read(&rtc);
        printf("\r\n20%d/%d/%d %d:%d:%d\r\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.min,rtc.sec);
    }
    
}
            

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
MAIN_RETURN main(void)
{
    
    
#ifdef __USB_CDC
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);
#endif  // __USB_CDC

    PLLEN  = 1 ;             // 内部クロックをPLLに通し48MHzとする
    FSEN   = 1 ;             // USBクロック使用時は48MHzとする
    RBPU   = 0 ;             // RBポートのプルアップを有効(全PORTBがプルアップ)
    OSCCON = 0b01110000 ;    // 内部OscでPLLからのクロックを使う(48MHz)
    ANCON0 = 0b00011111 ;    // AN0-4アナログは使用しない、デジタルI/Oに割当
    ANCON1 = 0b00011111 ;    // AN8-12アナログは使用しない、デジタルI/Oに割当
    TRISA  = 0b00000000 ;    // RA0-RA7全て出力に設定(RA4は無)
    TRISB  = 0b00110000 ;    // RB4/5(I2C)で入力、他は全て出力に設定
    TRISC  = 0b00000000 ;    // RC0-RVC7全て出力に設定(RC3は無)(RC4/5はINのみ)
    TRISD  = 0b00001111 ; 
    PORTA  = 0b00000000 ;    // 出力ピンの初期化(全てLOWにする)
    PORTB  = 0b00000000 ;    // 出力ピンの初期化(全てLOWにする)
    PORTC  = 0b00000000 ;    // 出力ピンの初期化(全てLOWにする)
    PORTD  = 0b00000000 ;    // 出力ピンの初期化(全てLOWにする)

    TRISAbits.TRISA0 = 0;   // LED1
    TRISAbits.TRISA1 = 0;   // LED2
    TRISAbits.TRISA2 = 0;   // Timer0 確認用
    TRISBbits.TRISB2 = 0;   // Timer1 確認用
    LATAbits.LATA0 = 1;
    LATAbits.LATA1 = 1;
    
    TRISDbits.TRISD3 = 1;   // Button 1
    TRISDbits.TRISD2 = 1;   // Button 2

    Wait(10);
    
    uart_init();
    uart_start_dispg();

    uart485_init();
    //uart485_start_dispg();

    log_init();
    
#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    printf("i2c_init()\r\n");
    i2c_init(1);
#endif

    
 #ifdef ____NOP    
    rtc.year = 23;
    rtc.month = 6;
    rtc.day = 7;
    rtc.wday = 3;
    rtc.hour = 9;
    rtc.min =35;
    rtc.sec =18;    
    printf("\r\nRTC Init 20%d/%d/%d %d:%d:%d\r\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.min,rtc.sec);
    RTC_init(&rtc);
#endif

    
#ifdef __USB_CDC
    //printf("USBDeviceInit()\r\n");
    USBDeviceInit();
        
    //printf("USBDeviceAttach()\r\n");
    USBDeviceAttach();

    //printf("USB_start_dispg()\r\n");
    main_status =1;

    USB_start_dispg();
    
    //printf("USBDeviceTasks()\r\n");
    USBDeviceTasks();
#endif  // __USB_CDC

    
    rtc_data_init();
    printf("Timer_init()\r\n");
    Timer0_init();  // 100usec
    Timer1_init();  // 10mmsec 
   
    Init_Timer();
    

    
    
#ifdef __VL53L_MASTER  //++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //VL53_init();

#endif  //__VL53L_MASTER +++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //Interrupt_init();
    
    //Interrupt_Start();
        //PIR2bits.USBIF = 0;
    //PIE2bits.USBIE = 1;     // USB Int
    printf("  SSPADD=%x\r\n",I2C_SSPADD);
    printf("  true=%x\r\n",true);
    printf("  false=%x\r\n",false);

    printf("main_loop_disp()\r\n");
    
    main_loop_disp();
    
    while(1)
    {



        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  100msec
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if( Check100msTask() == true )
        {
            Clear100msTaskFlg();
            T100msecMainTask();
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  1sec
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if( Check1secTask() == true )
        {
            Clear1secTaskFlg();
            T1secMainTask();
        }

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  allways
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         debug_main();
         
         //debug_main485();

         rs485_com_task();
         
       //vl53l0x_test();


        //SYSTEM_Tasks();

#ifdef __USB_CDC

        #if defined(USB_POLLING)
            // Interrupt or polling method.  If using polling, must call
            // this function periodically.  This function will take care
            // of processing and responding to SETUP transactions
            // (such as during the enumeration process when you first
            // plug in).  USB hosts require that USB devices should accept
            // and process SETUP packets in a timely fashion.  Therefore,
            // when using polling, this function should be called
            // regularly (such as once every 1.8ms or faster** [see
            // inline code comments in usb_device.c for explanation when
            // "or faster" applies])  In most cases, the USBDeviceTasks()
            // function does not take very long to execute (ex: <100
            // instruction cycles) before it returns.
            USBDeviceTasks();
        #endif

        //Application specific tasks
        APP_DeviceCDCBasicDemoTasks();

#endif  // __USB_CDC

        
       
       

    
    //Wait(100);

    }//end while
}//end main
//==============================================================================
//
//==============================================================================

#ifdef ___NOP
void Interrupt_Start(void)
{

    //==============================
    // TIMER0
    //==============================
    INTCONbits.TMR0IF = 0;    
    INTCONbits.TMR0IE = 1;

    //==============================
    // UART Rx
    //==============================
    PIR1bits.RC1IF = 0;
    PIE1bits.RC1IE = 1;     //UART RX

    //==============================
    // UART Tx
    //==============================
    PIR1bits.TX1IF = 0;
    PIE1bits.TX1IE = 1;     //UART TX

    //==============================
    // TIMER1
    //==============================
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;    //Timer1 Int


    //==============================
    // USB
    //==============================
    PIR2bits.USBIF = 0;
    PIE2bits.USBIE = 1;     // USB Int


    //==============================
    // RTC
    //==============================
    PIR3bits.RTCCIF = 0;
    PIE3bits.RTCCIE = 1;    // RTC
    
}
//==============================================================================
//#define USBEnableInterrupts() {RCONbits.IPEN = 1;IPR2bits.USBIP = 1;PIE2bits.USBIE = 1;INTCONbits.GIEH = 1;}
//==============================================================================

void Interrupt_init(void)
{
    //==============================
    // RCON
    //==============================
    RCON = 0;
    RCONbits.IPEN = 1;
    
    //==============================
    // INTCON
    //==============================
    INTCON = 0;
    INTCONbits.GIE_GIEH = 1;
    INTCONbits.PEIE_GIEL = 1;
    //INTCONbits.TMR0IE = 1;

    //==============================
    // INTCON2
    //==============================
    INTCON2 = 0;
    INTCON2bits.TMR0IP = 1;     // Timer1 Priority High   

    //==============================
    // INTCON3
    //==============================
    INTCON3 = 0;

    //==============================
    // PIR1
    //==============================
    PIR1 = 0;

    //==============================
    // PIR2
    //==============================
    PIR2 = 0;
    PIR2bits.USBIF = 1;

    //==============================
    // PIR3
    //==============================    
    PIR3 = 0;

    //==============================
    // PIR4
    //==============================
    PIR4 = 0;

    //==============================
    // PIR5
    //==============================
    PIR5 = 0;

    //==============================
    // PIE1
    //==============================
    PIE1 = 0;
    //PIE1bits.RC1IE = 1;     //UART RX
    //PIE1bits.TX1IE = 1;     //UART TX
    //PIE1bits.TMR1IE = 1;    //Timer1 Int



    PIE2 = 0;
    //PIE2bits.USBIE = 1;     // USB Int


    PIE3 = 0;
    //PIE3bits.RTCCIE = 1;    // RTC


    PIE4 = 0;
    PIE5 = 0;

    IPR1 = 0;
    IPR1bits.RC1IP = 0;     // UART Rx Priority Low
    IPR1bits.TX1IP = 0;     // UART Tx Priority Low
    IPR1bits.TMR1IP = 1;    // Timer1 Priority High


    IPR2 = 0;

    IPR3 = 0;
    IPR3bits.RTCCIP = 1;    // RTC Priority High 


    IPR4 = 0;
    IPR5 = 0;

            
            
}
#endif // ___NOP

//==============================================================================
// Waite time = num x 10ms
//==============================================================================
void Wait(uint16_t num)
{
     int i ;

     for (i=0 ; i < num ; i++) {
          __delay_ms(10) ;
     }
}
//==============================================================================
// TIMER0 100usec
// 
//==============================================================================


void Timer0_init(void)
{
    T0CON = 0;
    T0CONbits.TMR0ON = 1;       // タイマー0を有効にする
    T0CONbits.T08BIT = 0;       // 16 ビット タイマー
    T0CONbits.PSA    = 1;       // Timer0 プリスケーラーは割り当てない
    
    TMR0H = TIMER0_100usec >> 8;
    TMR0L = TIMER0_100usec & 0x00ff;
            
    
     //==============================
    // TIMER0
    //==============================
    INTCONbits.TMR0IF = 0;    
    INTCONbits.TMR0IE = 1;
    
}

//==============================================================================
// TIMER0 10msec
// 
//==============================================================================


void Timer1_init(void)
{
    T1CON = 0;
    
    /***************************************************************************
     * 10 = Timer1 クロック ソースは T1OSC または T1CKI ピンです
     * 01 = Timer1 のクロック ソースはシステム クロック (FOSC)(1)
     * 00 = Timer1 のクロック ソースは命令クロック (FOSC/4)
     **************************************************************************/ 
    T1CONbits.TMR1CS0 = 0;
    T1CONbits.TMR1CS1 = 0;
    
    /**************************************************************************
     * 11 = 1:8 プリスケール値
     * 10 = 1:4 プリスケール値
     * 01 = 1:2 プリスケール値
     * 00 = 1:1 プリスケール値
     **************************************************************************/ 
    T1CONbits.T1CKPS0 = 1;
    T1CONbits.T1CKPS1 = 0;

    /***************************************************************************
     * 1 = 1 回の 16 ビット操作で Timer1 のレジスタ読み取り/書き込みを有効にします。
     * 0 = 2 つの 8 ビット操作で Timer1 のレジスタ読み取り/書き込みを有効にし
     **************************************************************************/ 
    T1CONbits.RD16 = 1;

    /***************************************************************************
     * 1 = タイマー 1 を有効にする
     * 0 = タイマー 1 を停止します
     **************************************************************************/ 
    T1CONbits.TMR1ON = 1;
        
    TMR1H = TIMER1_10msec >> 8;
    TMR1L = TIMER1_10msec & 0x00ff;
            
    
     //==============================
    // TIMER1
    //==============================
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;    //Timer1 Int
}


/*******************************************************************************
 End of File
*/
