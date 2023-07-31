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

#include "system.h"

#ifdef __USB_CDC
#endif  // __USB_CDC

#include "usb.h"
#include "rtc_main.h"


#ifdef __USB_CDC
//==============================================================================
// PIC 18F4550のコンフィグレーション
//==============================================================================

/*******************************************************************************
 * < CONFIG1L >
 * 
 * WDTEN (ウォッチドッグタイマイネーブル):
 * 0: マイクロコントローラのリセットを監視するウォッチドッグタイマ機能は使用されません。
 * 
 * PLLDIV (PLLディバイダ):
 * 4: この設定では、PLL入力クロックは4で分周されます。
 * 
 * CFGPLLEN (設定時PLL有効):
 * 0: コンフィグレーションメモリの設定中にはPLLが動作しないようになっています。
 * 
 *  * STVREN (スタックリセット有効):
 * 0: スタックポインタがスタックオーバーフローまたはアンダーフローした場合、スタックリセットは実行されません。
 * 
 * XINST (拡張命令セット有効):
 * 1: 拡張命令セットは、通常の命令セットに比べて高度な演算やデバッグ機能を提供します
 * 
 * DEBUG (デバッグモード有効):
 * 1: デバッグモードが有効になっています。デバッグモードでは、デバッグ機能が有効になり、
 * 　 デバッガとの通信やデバッグ操作が可能になります。
 ******************************************************************************/
#pragma config CFGPLLEN = ON
#pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
#pragma config PLLDIV = 4           //Divide by 4 (16 MHz oscillator input)
#pragma config STVREN = OFF          //stack overflow/underflow reset enabled
#pragma config XINST = OFF          //Extended instruction set disabled
//#pragma config DEBUG   = ON


/*******************************************************************************
 * < CONFIG1H >
 * 
 * CPDIVV (システムクロック分周設定):
 * OSC1_PLL1: システムクロックは分周されたPLLクロックの1/1の周波数になります。
 * 
 * CP0 (コードプロテクト0):
 * 1 (有効): コードメモリのセクション0（通常は0x0000〜0x7FFFの範囲）を読み取り保護します。
 * この領域のコードがプログラマブルデバイスの読み込み操作から保護され、改ざんを防ぎます。
 * ただし、書き込み操作には影響しません。
 * 
 ******************************************************************************/
#pragma config CPUDIV = OSC1        //No CPU system clock divide
#pragma config CP0 = OFF            //Program memory is not code-protected

/*******************************************************************************
 * < CONFIG2L >
 * 
 * FOSC (オシレータ設定):
 * HSPLL_HS: 外部クロック(クリスタル)を使用し、PLL (位相ロックループ) を有効にします。
 * PLLは、クロック信号を増幅および周波数変換するために使用され、高速な動作を実現します。
 * 
 * SOSCSEL (セカンダリオシレータ選択):
 * 0: セカンダリオシレータは選択されていません。マイクロコントローラは、主に外部クロック(クリスタル)を使用して動作します。
 * 
 * CLKOEC (クロック出力イネーブル):
 * 0: マイクロコントローラから外部にクロック信号を出力する機能は使用されません。
 * 
 * FCMEN (クロック監視と切り替えイネーブル):
 * 0: クロック監視と自動切り替え機能は無効になっています。
 *    この機能を有効にすると、外部クロックの異常な動作を監視し、自動的に内部オシレータに切り替えることができます。
 * 
 * IESO (内部/外部スイッチオーバー):
 * 0: 内部/外部クロックのスイッチオーバー機能は無効になっています。
 *    この機能を有効にすると、内部オシレータと外部クロックの切り替えを制御できます。
 * 
 ******************************************************************************/
//#pragma config FOSC    = HSPLL_HS  // 外部クロック(クリスタル)を使用し、PLLを有効にする設定
//#pragma config SOSCSEL = 0
//#pragma config CLKOEC  = 0	

#pragma config OSC = HSPLL          //HS oscillator, PLL enabled, HSPLL used by USB
#pragma config FCMEN = OFF          //Fail-Safe Clock Monitor disabled
#pragma config IESO = OFF           //Two-Speed Start-up disabled

/*******************************************************************************
 * < CONFIG2H >
 * 
 * WDTPS (ウォッチドッグタイマポストスケール選択):
 * 0x0F: ウォッチドッグタイマのポストスケール値が0x0Fに設定されています。
 * この設定では、ウォッチドッグタイマが1:32768のポストスケールで動作します。
 * 
 ******************************************************************************/
//#pragma config WDTPS = 0x0f    // Watchdog Timer Postscale Select (1:32768)
#pragma config WDTPS = 32768        //1:32768
/*******************************************************************************
 * < CONFIG3L >
 * 
 * DSWDTPS<3:0> (ディープスリープウォッチドッグタイマポストスケールセレクト):
 * このビットフィールドは、ディープスリープモードでのウォッチドッグタイマのポストスケール値を設定します。
 * 推奨設定は、一般的な設定は、長いポストスケール値を選択することです。
 * 
 * DSWDTEN (ディープスリープウォッチドッグタイマイネーブル):
 * このビットは、ディープスリープモードでのウォッチドッグタイマの動作を有効または無効にします。
 * 推奨設定は、ウォッチドッグタイマの使用が必要な場合は有効にし、必要ない場合は無効にします。
 * 
 * DSBOREN (ディープスリープブラウンアウトリセットイネーブル):
 * このビットは、ディープスリープモードでのブラウンアウトリセット（電源降下時のリセット）の動作を有効または無効にします。
 * 推奨設定は、ブラウンアウトリセットの使用が必要な場合は有効にし、必要ない場合は無効にします。
 * 
 * RTCOSC (リアルタイムクロックソースセレクト):
 * このビットは、リアルタイムクロック（RTC）のソースを選択します。
 * 推奨設定は、内部オシレータを使用する場合は、内部オシレータを選択します。外部クリスタルを使用する場合は、外部クリスタルを選択します。
 * 
 * DSWDTOSC (ディープスリープウォッチドッグタイマクロスオシレータセレクト):
 * このビットは、ディープスリープモードでのウォッチドッグタイマのクロスオシレータのソースを選択します。
 * 推奨設定は、内部オシレータを使用する場合は、内部オシレータを選択します。外部クリスタルを使用する場合は、外部クリスタルを選択します。
 * 
 ******************************************************************************/

#pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep
#pragma config DSWDTEN = OFF        //Disabled
#pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
#pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as clock
//#pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as clock
#pragma config RTCOSC = INTOSCREF


/*******************************************************************************
 * < CONFIG3H >
 * 
 * MSSPMSK:
 * このビットフィールドは、MSSP（Master Synchronous Serial Port）モジュールのマスク機能を制御します。
 * マスク機能により、特定のビット位置の送信や受信を無効化することができます。
 * 
 * ADCSEL:
 * A/Dコンバータの変換モードを選択します。10ビット変換モードと12ビット変換モードの
 * 2つのオプションがあります。選択したモードに応じて、A/Dコンバータのビット数と変換の精度が異なります。
 * 
 * IOL1WAY:
 * このビットは、ピンのI/Oレジスタの一度だけの書き込み有効化を制御します
 * 有効な場合、I/Oレジスタは一度だけ書き込みが許可され、その後は変更できなくなります。
 * 
 ******************************************************************************/

//#pragma config MSSPMSK = 0
//#pragma config ADCSEL  = 0

#pragma config IOL1WAY = OFF        //IOLOCK bit can be set and cleared

/*******************************************************************************
 * < CONFIG4L >
 * 
 * WPCFG (Write/Erase Protect Configuration Region Select bit):
 * 1 = 設定ワードページは、WPENDとWPFP<5:0>の設定に関係なく、消去/書き込み保護されません。
 * 0 = 設定ワードページは、WPENDとWPFP<5:0>の設定に関係なく消去/書き込み保護されます。
 * 
 * WPFP<6:0> (Write/Erase Protect Page Start/End Location bits):
 * これらのビットは、WPENDビットと組み合わせて、フラッシュメモリのどのページが書き込み/消去保護されるかを定義します。
 * 7ビット（WPFP<6:0>）は、フラッシュメモリのページの開始位置と終了位置を指定します。
 * 
 ******************************************************************************/
#pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
#pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0

/*******************************************************************************
 * < CONFIG4H>
 * 
 * LS48MHZ (Low-Speed USB Clock Selection):
 * このビットは、低速USBクロックの選択に関連しています。
 * プロジェクトが48 MHzのシステムクロックを使用し、低速USBクロックを生成する場合は1を選択します。
 * 1 = 48 MHzのシステムクロックが使用され、低速USBクロックは8分周で生成されます。
 * 0 = 24 MHzのシステムクロックが使用され、低速USBクロックは4分周で生成されます。
 * 
 * WPEND (Write-Protect Disable bit):
 * このビットは、フラッシュメモリの特定のページを書き込み/消去保護するかどうかを制御します。
 * 1 = フラッシュページ、WPFP<6:0>から（設定ワードページまで）は書き込み/消去保護されます。
 * 0 = フラッシュページ0からWPFP<6:0>までが書き込み/消去保護されます。
 * 
 * WPDIS (Write-Protect Disable bit):
 * このビットは、フラッシュメモリの書き込み/消去保護を無効にするかどうかを制御します。
 * 1 = WPFP<5:0>、WPEND、WPCFGビットが無視され、すべてのフラッシュメモリが消去または書き込み可能になります。
 * 0 = WPFP<5:0>、WPEND、WPCFGビットが有効であり、選択された範囲に対して消去/書き込み保護が有効です。
 * 
 ******************************************************************************/
//#pragma config LS48MHZ = 1
#pragma config WPEND = PAGE_0       //Start protection at page 0
#pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored



/** CONFIGURATION Bits **********************************************/
#pragma config MSSP7B_EN = MSK7     //7 Bit address masking

#endif  // __USB_CDC

#ifdef __I2C_MASTER
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 2       // PLL Prescaler Selection (Divide by 2 (8 MHz oscillator input))
#pragma config CFGPLLEN = ON    // PLL Enable Configuration Bit (PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = INTOSCPLL  // Oscillator (INTOSCPLL)
#pragma config SOSCSEL = HIGH   // T1OSC/SOSC Power Selection Bits (High Power T1OSC/SOSC circuit selected)
#pragma config CLKOEC = ON      // EC Clock Out Enable Bit  (CLKO output enabled on the RA6 pin)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler (1:32768)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = INTOSCREF// RTCC Clock Select (RTCC uses INTRC)
#pragma config DSBOREN = ON     // Deep Sleep BOR (Enabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config ADCSEL = BIT10   // ADC 10 or 12 Bit Select (10 - Bit ADC Enabled)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_63   // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 63)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region  (Configuration Words page not erase/write-protected)

// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<6:0>/WPEND region ignored)
#pragma config WPEND = PAGE_WPFP// Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages WPFP<6:0> through Configuration Words erase/write protected)
#pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock bit (System clock at 48 MHz USB CLKEN divide-by is set to 8)

#endif //__I2C_MASTER



void Interrupt_Timer0(void);
void Interrupt_Timer1(void);
void Interrupt_RTC(void);
void Interrupt_Rx1(void);
void Interrupt_Tx1(void);
void Interrupt_USB(void);

#ifdef ___I2C_MAIN
void interrupt_i2c_bcl(void);
void interrupt_i2c_ssp(void);
#endif

void print_string(char *string);
extern void SetLogData(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4);

#define TIMER100MS 10
#define TIMER1SEC 100

uint16_t  timer0_cnt;
uint16_t  timer1_cnt;

uint8_t timer1sec = TIMER1SEC;
uint8_t timer100msec = TIMER100MS;
uint8_t task100ms_flg = 0;
uint8_t task1sec_flg = 0;


extern RTC_DATA RTCdt;

#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int AckCheck ;           // 相手からのACK待ち用フラグ変数
int CollisionCheck ;     // 他のマスターとのバス衝突検出用フラグ変数

#endif //__I2C_MASTER ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
#ifdef __USB_CDC

void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:

            /*******************************************************************
             * In this devices family of USB microcontrollers, the PLL will not 
             * power up and be enabled by default, even if a PLL enabled oscillator 
             * configuration is selected (such as HS+PLL).
             * This allows the device to power up at a lower initial operating frequency, 
             * which can be advantageous when powered from a source which is not 
             * gauranteed to be adequate for 48MHz operation.  On these devices, 
             * user firmware needs to manually set the OSCTUNE<PLLEN> bit to power 
             * up the PLL.
             * 
             * USB マイクロコントローラーのこのデバイス ファミリでは、PLL が有効な
             * 発振器構成 (HS+PLL など) が選択されている場合でも、PLL は起動せず、
             * デフォルトで有効になります。
             * これにより、デバイスはより低い初期動作周波数で電源を投入できるようになり、
             * 48MHz 動作に適切であることが保証されていない電源から電力を供給する場合に
             * 有利になります。 
             * これらのデバイスでは、ユーザー ファームウェアが OSCTUNE<PLLEN> ビットを
             * 手動で設定して PLL に電源を投入する必要があります。
             ******************************************************************/
            
             {
                 /***************************************************************
                  * PLL を有効にし、PLL がロックするまで 2 ミリ秒以上待ってから 
                  * USB モジュールを有効にします
                  *************************************************************/
                 unsigned int pll_startup_counter = 600;
                 OSCTUNEbits.PLLEN = 1;  
                 while(pll_startup_counter--);
             }
        
             /******************************************************************
              * PLL がロックされ準備が整うと、デバイスは自動的に PLL 出力に切り替わります。
              * Device switches over automatically to PLL output after PLL is locked and ready.
              *****************************************************************/
			 
            LED_Enable(LED_USB_DEVICE_STATE);
            BUTTON_Enable(BUTTON_DEVICE_CDC_BASIC_DEMO);
            break;
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            OSCCON = 0x70;		//Primary clock source selected.
            
            //Adding a software start up delay will ensure
            //that the primary oscillator and PLL are running before executing any other
            //code.  If the PLL isn't being used, (ex: primary osc = 48MHz externally applied EC)
            //then this code adds a small unnecessary delay, but it is harmless to execute anyway.
            {
                unsigned int pll_startup_counter = 800;	//Long delay at 31kHz, but ~0.8ms at 48MHz
                while(pll_startup_counter--);			//Clock will switch over while executing this delay loop
            }
            break;
    }
}
#endif  // __USB_CDC

#if(__XC8_VERSION < 2000)
    #define INTERRUPT interrupt
#else
    #define INTERRUPT __interrupt()
#endif

#ifdef ___NOP

void INTERRUPT SYS_InterruptHigh(void)
{
    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif
}
#endif


//==============================================================================
//
// high_priority
//  TIMER0
//  TIMER1
//  RTC
//
// low_priority
//  UART Rx
//  UART Tx
//  USB
//    
//==============================================================================
//id INTERRUPT SYS_InterruptHigh(void)
void __interrupt(high_priority) high_isr(void) 
{     
    //print_string("high_isr \r\n");
    //==============================
    // TIMER0
    //==============================
    if( INTCONbits.TMR0IF ){
        INTCONbits.TMR0IF = 0;      // 割込みフラグをクリア
        INTCONbits.TMR0IE = 0;      // 割込みを無効化
        
        
        // コールバック関数
        Interrupt_Timer0();
        
        // タイマー値再設定
        TMR0H = TIMER0_100usec >> 8;
        TMR0L = TIMER0_100usec & 0x00ff;
        
        INTCONbits.TMR0IE = 1;      // 割込みを有効化        
    }
    //==============================
    // TIMER1
    //==============================
    if( PIR1bits.TMR1IF ){
        PIR1bits.TMR1IF = 0;        // 割込みフラグをクリア
        PIE1bits.TMR1IE = 0;        // 割込みを無効化

        // コールバック関数
        Interrupt_Timer1();
        ssCount_Timer();
        
        // タイマー値再設定
        //==============================
        // TIMER1
        //==============================
        TMR1H = TIMER1_10msec >> 8;
        TMR1L = TIMER1_10msec & 0x00ff;
        
        PIE1bits.TMR1IE = 1;        // 割込みを有効化
    }
 
    //==============================
    // RTC
    //==============================
    if( PIR3bits.RTCCIF ){
        PIR3bits.RTCCIF = 0;        // 割込みフラグをクリア
        PIE3bits.RTCCIE = 0;        // 割込みを無効化

        // コールバック関数
        Interrupt_RTC();
        
        PIE3bits.RTCCIE = 1;        // 割込みを有効化
    }
 
    //==============================
    // USB
    //==============================
#ifdef __USB_CDC
    if( PIR2bits.USBIF ){
        PIR2bits.USBIF = 0;         // 割込みフラグをクリア
        PIE2bits.USBIE = 0;         // 割込みを無効化t
        // コールバック関数
        //printf("USB Interrupt\r\n");
        Interrupt_USB();

        PIE2bits.USBIE = 1;         // 割込みを有効化
    }
#endif  // __USB_CDC    
  

#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef __SSP_ISR_HIGH


    
     if (I2C_BCLIE == 1 && I2C_BCLIF == 1) {  
     //if ( I2C_BCLIF == 1) {       // MSSP(I2C)バス衝突割り込み発生か？
          I2C_BCLIF = 0 ;        // フラグクリア
          interrupt_i2c_bcl();
     }
    
     if (I2C_SSPIE == 1 && I2C_SSPIF == 1) { 
    //if ( I2C_SSPIF == 1) {       // SSP(I2C)割り込み発生か？
        I2C_SSPIF = 0;
        interrupt_i2c_ssp();
     }

#endif  // __SSP_ISR_HIGH

#endif //__I2C_MASTER ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    
}

//==============================================================================
//
//==============================================================================
void __interrupt(low_priority) low_isr(void) 
{
#ifdef ___NOP   
    print_string("low_isr \r\n");

    //==============================
    // UART Rx
    //==============================
    if( PIR1bits.RC1IF ){
        PIR1bits.RC1IF = 0;         // 割込みフラグをクリア
        PIE1bits.RC1IE = 0;         // 割込みを無効化
        // コールバック関数
        Interrupt_Rx1();

        PIE1bits.RC1IE = 1;         // 割込みを有効化
    }
    //==============================
    // UART Tx
    //==============================
    if( PIR1bits.TX1IF ){
    PIR1bits.TX1IF = 0;             // 割込みフラグをクリア
    PIE1bits.TX1IE = 0;             // 割込みを無効化
        // コールバック関数
        Interrupt_Tx1();

        PIE1bits.TX1IE = 1;         // 割込みを有効化

    }
#endif

#ifndef __SSP_ISR_HIGH
     if (I2C_SSPIE == 1 && I2C_SSPIF == 1) {       // SSP(I2C)割り込み発生か？
        I2C_SSPIF = 0;

        interrupt_i2c_ssp();
     }
     if (I2C_BCLIE == 1 && I2C_BCLIF == 1) {       // MSSP(I2C)バス衝突割り込み発生か？
//          CollisionCheck = 1 ;
          I2C_BCLIF = 0 ;        // フラグクリア
          interrupt_i2c_bcl();
     }
#endif  // __SSP_ISR_HIGH
    
}

//==============================================================================
//
//==============================================================================
#ifdef ___NOP
#endif

         
//==============================================================================
//
//==============================================================================
void Interrupt_Timer0(void)
{

#ifdef ___NOP    
    if( LATAbits.LATA2 )
    {
        LATAbits.LATA2 = 0;        
    }
    else
    {
        LATAbits.LATA2 = 1;                
    }
#endif
        
    RTCdt.usec ++;
    if( RTCdt.usec > 10 )
    {
        RTCdt.usec = 0;
        RTCdt.msec ++;


        if( RTCdt.msec > 1000 )
        {
            RTCdt.msec = 0;
            RTCdt.sec ++;

            if( RTCdt.sec > 60 )
            {
                RTCdt.sec = 0;
                RTCdt.min ++;
                if( RTCdt.min > 60 )
                {
                    RTCdt.min = 0;
                    RTCdt.hour ++;                
                }
            }
        }
    }
}

void Interrupt_Timer1(void)
{
    timer1_cnt ++;

#ifdef ___NOP
    if( timer1_cnt & 0x01 )
    {
        LATBbits.LATB2 = 1;        
    }
    else
    {
        LATBbits.LATB2 = 0;        
    }
#endif    
    
    timer100msec --;
    if( timer100msec == 0 ){
        timer100msec = TIMER100MS;
        task100ms_flg = 1;
    }
    
    timer1sec --;
    if( timer1sec == 0 ){
        timer1sec = TIMER1SEC;
        task1sec_flg = 1;
    }



    
    
}
void Interrupt_RTC(void)
{
    //printf("RTC Interrupt");
}

#ifdef ___NOP
void Interrupt_Rx1(void)
{
    //printf("RX Interrupt");
}
void Interrupt_Tx1(void)
{
    //printf("TX Interrupt");
}
#endif  // ___NOP

#ifdef __USB_CDC
void Interrupt_USB(void)
{
     USBDeviceTasks();
}
#endif  // __USB_CDC    


bool Check100msTask(void)
{
    if( task100ms_flg == 1 )
        return true;
    else
        return false;
}
void Clear100msTaskFlg(void)
{
    task100ms_flg = 0;
}

bool Check1secTask(void)
{
    if( task1sec_flg == 1 )
        return true;
    else
        return false;
}
void Clear1secTaskFlg(void)
{
    task1sec_flg = 0;
}




//=====================================
// タイマー
//=====================================
#define SKTIMER_NUM 3

typedef enum{
    ST_TIMER_IDLE,
    ST_TIMER_USING
}ST_TIMER;

typedef struct{
    uint8_t flg;
    uint16_t cnt;
}SK_TIMER_DATA;

SK_TIMER_DATA SKtimer_data[SKTIMER_NUM];

//==============================================================================
//
//==============================================================================
uint16_t Get_Timer(int sel)
{
    uint16_t rtn = 0xffff;
    
   if(sel <  SKTIMER_NUM){
       if( SKtimer_data[sel].flg == ST_TIMER_USING){
            rtn = SKtimer_data[sel].cnt;
       }
   }
    
    if(rtn==0)
    {
#ifdef ___NOP
        printf("Get_Timer(index=%d) flg=%d,rtn=%d\r\n",sel,SKtimer_data[sel].flg,rtn);
        SetLogData( LOG_DISP_GET_TM, "Get_Timer()",(uint8_t)sel,SKtimer_data[sel].flg,0,0);
#endif
    }
    return rtn;
}

//==============================================================================
//
//==============================================================================
int Set_Timer(uint16_t dt)
{
    uint8_t i;
    int  rtn;
    
    rtn = -1;
    for( i = 0; i < SKTIMER_NUM; i++ ){
        if( SKtimer_data[i].flg == ST_TIMER_IDLE){
            SKtimer_data[i].flg = ST_TIMER_USING;
            SKtimer_data[i].cnt = dt;
            
            rtn =  i;
            break;
        }
    }
    
#ifdef ___NOP
    SetLogData(LOG_DISP_SET_TM, "Set_Timer()",(uint8_t)dt,(uint8_t)rtn,0,0);
    printf("Set_Timer(%d) index=%d\r\n",dt,rtn);
#endif
       
    
    
    return rtn;
}
//==============================================================================
//
//==============================================================================
void Init_Timer(void)
{
    uint8_t i;
    
    for( i = 0; i < SKTIMER_NUM; i++ ){
        
        SKtimer_data[i].flg = ST_TIMER_IDLE;
        SKtimer_data[i].cnt =  0;
            
    }
}



//==============================================================================
//
//==============================================================================

void ssCount_Timer(void)
{
    uint8_t i;
    
    for( i = 0; i < SKTIMER_NUM; i++ ){
        if( SKtimer_data[i].flg == ST_TIMER_USING){
            if ( SKtimer_data[i].cnt > 0 ){            
                SKtimer_data[i].cnt --;
            }
            if ( SKtimer_data[i].cnt == 0 ){
                // Call back function
            }
        }
    }
}

//==============================================================================
//
//==============================================================================
void Rel_Timer(int sel)
{
    if(sel <  SKTIMER_NUM){
        SKtimer_data[sel].flg = ST_TIMER_IDLE;
        SKtimer_data[sel].cnt = 0;
    }
#ifdef ___NOP
    printf("Rel_Timer(index=%x) \r\n",sel);
    SetLogData(LOG_DISP_REL_TM, "Rel_Timer()",(uint8_t)sel,0,0,0);
#endif
}
