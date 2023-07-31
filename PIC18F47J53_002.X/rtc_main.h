/* 
 * File:   rtc_main.h
 * Author: nosak
 *
 * Created on 2023/06/06, 8:55
 */

#ifndef RTC_MAIN_H
#define	RTC_MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif


/*******************************************************************************
 *    Year : 日付の年(0-99)を指定する(西暦の下２ケタ 2000年-2099年)
 *    Mon  : 日付の月(1-12)を指定する
 *    mDay : 日付の日(1-31)を指定する(２月は28/29日)
 *    wDay : カレンダの曜日を指定する(0=日 1=月 ～ 6=土) 
 *    Hour : 時刻の時(0-23)を指定する(24時間で指定)
 *    Min  : 時刻の分(0-59)を指定する  
 *    Sec  : 時刻の秒(0-59)を指定する       
 ******************************************************************************/ 
typedef struct{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t wday;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint16_t msec;
    uint8_t usec;
} RTC_DATA ;



extern void RTC_init(RTC_DATA *rtc);
extern void Rtc_Read(RTC_DATA *rtc);

#ifdef	__cplusplus
}
#endif

#endif	/* RTC_MAIN_H */

