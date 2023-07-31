
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
#include "rtc_main.h"

RTC_DATA RTCdt;


/** ProtoType *******************************************************/
uint8_t bin2bcd(unsigned int num) ;


//==============================================================================
// ＢＣＤをバイナリ(10進数)に変換する処理
//==============================================================================
uint8_t bcd2bin(unsigned char dt)
{
     return ((dt >> 4) * 10) + (dt & 0xf) ;
}

#ifdef ___NOP
//==============================================================================
// バイナリ(10進数)をＢＣＤに変換する処理
//==============================================================================
uint8_t bin2bcd(unsigned int num) 
{
    uint8_t dt1;
    uint8_t dt2;
    
    dt1 = num % 100;
    
    dt2 = dt1/10;
    dt1 = dt1 % 10;
    
     return ( (dt2 << 4) | dt1 ) ;
}
#endif

/********************************************************************
 * Function:       void RTC_init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
#ifdef ___NOP

void RTC_init(RTC_DATA  *rtc)
{
    //=====================================        
    // RTCCFG
    //=====================================        
    RTCCFG = 0;
    
    /****************************************************************************
     * RTCWREN: RTCC 値レジスタ書き込みイネーブルビット
     * 1 = RTCVALH および RTCVALL レジスタはユーザーによる書き込みが可能
     * 0 = RTCVALH および RTCVALL レジスタはユーザーによる書き込みがロックアウトされます。
     ***************************************************************************/
    RTCCFGbits.RTCWREN = 1;     // RTCVALH および RTCVALL レジスタはユーザーによる書き込みが可能
    EECON2 = 0x55;
    EECON2 = 0xAA;
    RTCCFGbits.RTCWREN = 1;     // RTCVALH および RTCVALL レジスタはユーザーによる書き込みが可能
    
    /****************************************************************************
     * TCEN: RTCC イネーブル ビット
     * 1 = RTCC モジュールが有効です
     * 0 = RTCC モジュールは無効です
     ***************************************************************************/
    RTCCFGbits.RTCEN = 1;   

    /****************************************************************************
     * RTCOE: RTCC 出力イネーブルビット
     * 1 = RTCC クロック出力が有効
     * 0 = RTCC クロック出力が無効
     ***************************************************************************/
    RTCCFGbits.RTCOE = 1;     
    
    
    //=====================================        
    // RTCCAL
    //=====================================        
    
    RTCCAL = 0;                 // キャリブレーション ビット

    //=====================================        
    // PADCFG1
    //=====================================        
    PADCFG1 = 0;
    
    
    /***************************************************************************
     * RTSECSEL<1:0>: RTCC 秒クロック出力選択ビット
     * 11 = 予約済み。 使ってはいけません
     * 10 = RTCC ソース クロックが RTCC ピンに選択されます (ピンは RTCOSC (CONFIG3L<1>) 設定に応じて、INTRC または T1OSC になります)
     * 01 = RTCC 秒クロックが RTCC ピンに選択されます
     * 00 = RTCC アラーム パルスが RTCC ピンに選択される
     ***************************************************************************/
    uint8_t output = 2;

    switch(output){
        case 0:
            // RTCC アラーム パルス
            PADCFG1bits.RTSECSEL0 = 0;
            PADCFG1bits.RTSECSEL1 = 0;  
            break;
        case 1:
            // RTCC 秒クロック
            PADCFG1bits.RTSECSEL0 = 1;
            PADCFG1bits.RTSECSEL1 = 0;  
            break;
        case 2:
            // RTCC ソース クロック
            PADCFG1bits.RTSECSEL0 = 0;  
            PADCFG1bits.RTSECSEL1 = 1;  
            break;
    }
    
    PADCFG1bits.PMPTTL = 0;     // 1 = PMP モジュールは TTL 入力バッファを使用します
                                // 0 = PMP モジュールはシュミット入力バッファを使用します

    
    ALRMCFG = 0;
    ALRMRPT = 0;
     
    // 日時読込みレジスター(RTCVAL)のポインターを設定する
    RTCCFGbits.RTCPTR0 = 1;     
    RTCCFGbits.RTCPTR0 = 1;     
    
     // RTCC(RTCVAL)に指定日時をBCDに変換して設定を行う
    RTCVALL = (char)bin2bcd(rtc->year) ;    // Yearsの設定(カレンダの年00-99)
    RTCVALH = 0x00 ;                        // Unused Data
    RTCVALL = (char)bin2bcd(rtc->day) ;     // Daysの設定(カレンダの日1-31)
    RTCVALH = (char)bin2bcd(rtc->month)  ;  // Monthsの設定(カレンダの月1-12)
    RTCVALL = (char)bin2bcd(rtc->hour) ;    // Hoursの設定(時刻の時0-23)
    RTCVALH = (char)bin2bcd(rtc->wday) ;    // WeekDaysの設定(カレンダの曜日0:日-6:土)
    RTCVALL = (char)bin2bcd(rtc->sec)  ;    // Secondsの設定(時刻の秒0-59)
    RTCVALH = (char)bin2bcd(rtc->min)  ;    // Minutesの設定(時刻の分0-59)

   
    

    RTCCFGbits.RTCWREN = 0;     // RTCVALH および RTCVALL レジスタへの書き込みをマスク
    
    PIE3bits.RTCCIE = 0;        // 割込みを無効化
    PIE3bits.RTCCIE = 1;        // 割込みを有効化

 }

#endif

void Rtc_Read( RTC_DATA *rtc )
{
    uint8_t dt ;
    
    while( RTCCFGbits.RTCSYNC )
    {
        
    }

    RTCCFGbits.RTCPTR0 = 1; 
    RTCCFGbits.RTCPTR1 = 1;


     // RTCC(RTCVAL)から時刻を読み出す
     dt = RTCVALL;
     rtc->year = bcd2bin(dt) ;
     dt = RTCVALH;            // Unused Dataだが空読込みする
     dt = RTCVALL & 0x3F;
     rtc->day = bcd2bin(dt) ;
     dt = RTCVALH & 0x1F;
     //rtc->month = bcd2bin(dt) - 1 ; // tm型に合わせる為わざと１引く
     rtc->month = bcd2bin(dt); // tm型に合わせる為わざと１引く
     dt = RTCVALL & 0x3F;
     rtc->hour = bcd2bin(dt) ;
     dt = RTCVALH & 0x07;
     rtc->wday = bcd2bin(dt) ;
     dt = RTCVALL & 0x7F;
     rtc->sec = bcd2bin(dt) ;
     dt = RTCVALH & 0x7F;
     rtc->min = bcd2bin(dt) ;
    
}

void rtc_disp_log(uint8_t flg)
{
    uint16_t dt;
    
    dt = RTCdt.usec;

    if( dt == 10 )
        dt = 0;
    
    printf("%d:%d:%d %d.%d ", RTCdt.hour, RTCdt.min, RTCdt.sec, RTCdt.msec, dt);
    if( flg == 1 )
        printf("\r\n");    
}

void rtc_data_init(void)
{
    RTCdt.hour = 0;
    RTCdt.min = 0;
    RTCdt.sec = 0;
    RTCdt.msec = 0;
    RTCdt.usec = 0;
}



//==============================================================================    
//
//==============================================================================    
uint8_t get_rtc_sec(void)
{
        return RTCdt.sec;
}            
//==============================================================================    
//
//==============================================================================    
uint16_t get_rtc_msec(void)            
{
        return RTCdt.msec;
} 
//==============================================================================    
//
//==============================================================================    
uint8_t get_rtc_usec(void)            
{
        return RTCdt.usec;
}             
/*******************************************************************************
 End of File
*/
