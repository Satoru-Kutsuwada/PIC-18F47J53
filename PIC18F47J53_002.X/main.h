/* 
 * File:   main.h
 * Author: nosak
 *
 * Created on 2023/05/08, 11:55
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

// 秋月のLEDのON/OFFサンプルソース
//#define SAMPLE_LED

// タイマー割込み    
//#define SAMPLE_TIMER0

// UARTのサンプル    
#define SAMPLE_USART
    
//#define SAMPLE_USBCDC

// I2C
#define I2C_INPRI

// VL53の
#define VL53_INPRI
    
    
// log
#define VL53_log

    
    
//==============================================================================
// システム設定の定義				
//==============================================================================

//#define _XTAL_FREQ 20000000 // クリスタルの周波数を設定 (20MHzの場合)
#define _XTAL_FREQ 48000000 

// I2Cクロックの設定
//#define I2C_CLOCK 100000 // 400kHz
#define I2C_CLOCK 400000 // 400kHz
//#define I2C_CLOCK 460800

// UARTボーレートの設定
//#define UART_BAUDRATE 9600L // 115200bps　#define BAUD_RATE   9600L       // USARTのボーレート
#define UART_BAUDRATE 57600L // 115200bps　#define BAUD_RATE   9600L       // USARTのボーレート

// タイマーの設定
#define TIMER0_INT_TIME      1000000   // 1000ms usec単位のデータ
#define TIMER1_INT_TIME       1000   //  0ms usec単位のデータ
    
    

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

