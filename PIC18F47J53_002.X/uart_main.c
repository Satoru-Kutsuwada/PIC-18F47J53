
#include "system.h"
#include "main.h"			
#include <xc.h>		

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>


//==============================================================================
//
//==============================================================================

typedef struct{
    uint8_t BRGHx;
    uint8_t SPBRGHx;
    uint8_t SPBRGx;
} UART_SET_DATA;



void putch(char c);
int getch(void);
void Set_UasrtBauRaite(UART_SET_DATA *dt);
void print485(char *string);

//==============================================================================
//
//==============================================================================
void Set_UasrtBauRaite(UART_SET_DATA *dt)
{
    uint16_t l_buf;
    
    dt->BRGHx = 0;
    dt->SPBRGHx = 0;
    
    l_buf = _XTAL_FREQ/(64*UART_BAUDRATE) - 1;
    dt->SPBRGx = (uint8_t)l_buf;
}                    
                    






#ifdef __PRINTF_UART
//==============================================================================
//
//==============================================================================

void putch(char c)

{
//    while(!TXSTA1bits.TRMT);
//    TXREG = c;
    while (!UART_TXSTA_TRMT); // 送信バッファが空になるまで待機
    UART_TXREG = c; // データを送信
}
//==============================================================================
//
//==============================================================================

//unsigned char getch(void)
int getch(void)
{
    // while(!RCIF);
    
    if(!UART_RCIF){
        return (char)0;
        
    }
    else{
        return (int) UART_RCREG; // 受信データを返す
    }        
}
#endif

void uint2string(char *buf, uint16_t data, uint8_t flg,uint8_t keta);
//==============================================================================
//
//==============================================================================
int getch485(void)
{
    uint8_t dt;
    char buf[10];
    
    // while(!RCIF);
    //printf("getch485()\r\n");
    if(!UART485_RCIF){
        return (char)0;
        
    }
    else{
        dt = UART485_RCREG;
        printf("getch485() data=%x\r\n",dt);
        //uint2string(buf,dt,16,0);
        //print485("getch485()=");
        //print485(buf);
        //print485("\r\n");

        return (int) dt; // 受信データを返す
    }        
}

void putch485(char c)
{
//    while(!TXSTA1bits.TRMT);
//    TXREG = c;
    UART485_CTRL  = 1;    
    while (!UART485_TXSTA_TRMT); // 送信バッファが空になるまで待機
    UART485_TXREG = c; // データを送信
    while (!UART485_TXSTA_TRMT); // 送信バッファが空になるまで待機
    UART485_CTRL  = 0;    
}

void print485(char *string)
{
    while(*string != '\0'){
        putch485(*string);
        string ++;
    }
}

    
//==============================================================================
//
//==============================================================================
void uart_init(void)
{
    UART_SET_DATA dt;
    
#ifndef ___UART_CH1

    // Unlock Registers
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;


    // Configure Input Functions
    RPINR16 = 22; // Assign RX2 To Pin RP22 PD5
    
    // Configure Output Functions
    RPOR23 = 0x06; // Assign TX2 To Pin RP23 PD6


    // Lock Registers
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;

#endif
    
        UART_TXPORT = 0;        // TX1ピンを出力に設定
        UART_RXPORT = 1;        // RX1ピンを入力に設定
        
        // UART1モジュールの設定
        UART_TXSTA = 0;
        UART_TXSTA_TXEN = 1;    // 送信有効化
        UART_TXSTA_BRGH = 0;

        UART_RCSTA = 0;
        UART_RCSTA_SPEN = 1;
        UART_RCSTA_CREN = 1;    // 受信有効化
        
        UART_BAUDCON = 0;
        UART_BAUDCON_BRG16 = 0; // ボーレートの設定による   0:8bit,1:16bit
        
        Set_UasrtBauRaite(&dt);
        UART_SPBRGH = dt.SPBRGHx;
        UART_SPBRG  = dt.SPBRGx;
        UART_TXSTA_BRGH = dt.BRGHx;
        
    
}

//==============================================================================
//
//==============================================================================
void uart485_init(void)
{
        UART_SET_DATA dt;
        
    // アドレス選択方式スレーブ側
        //printf("uart485_init()\r\n");

#ifdef ___UART485_CH2    
    // Unlock Registers
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 0;

    // Configure Input Functions
    RPINR16 = 22; // Assign RX2 To Pin RP22 PD5
    
    // Configure Output Functions
    RPOR23 = 0x06; // Assign TX2 To Pin RP23 PD6

    // Lock Registers
    EECON2 = 0x55;
    EECON2 = 0xAA;
    PPSCONbits.IOLOCK = 1;
#endif    

    UART485_TXPORT = 0;
    UART485_RXPORT = 1;
    
    UART485_CTRL = 0;
    
    // UART1モジュールの設定
    UART485_TXSTA = 0;
    UART485_TXSTA_TXEN = 1;    // 送信有効化
    UART485_TXSTA_BRGH = 0;

    UART485_RCSTA = 0;
    UART485_RCSTA_SPEN  = 1;
    UART485_RCSTA_CREN  = 1;    // 受信有効化
    
    //UART485_RCSTA_RX9   = 1;    // 9bitモード
    //UART485_RCSTA_ADDEN = 1;

    UART485_BAUDCON = 0;
    UART485_BAUDCON_BRG16 = 0; // ボーレートの設定による   0:8bit,1:16bit

    Set_UasrtBauRaite(&dt);
    UART485_SPBRGH = dt.SPBRGHx;
    UART485_SPBRG  = dt.SPBRGx;
    UART485_TXSTA_BRGH = dt.BRGHx;

}