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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <xc.h>
#include <stdio.h>
#include <stdbool.h>

#include "buttons.h"
#include "leds.h"

#include "io_mapping.h"
#define MAIN_RETURN void



/*** Function States **************************************************/
// #define __USB_CDC
#define __I2C_MASTER

#define __VL53L_MASTER
#define __SSP_ISR_HIGH
//#define ___VL53_LOG_DISP 
//#define __I2C_INTERRUPT

//#define __INPUT_STRING_LOG

#define ___UART_CH1
#define ___UART485_CH2

void vl53_LogDisp(char *string,int8_t status);
void set_log_stop(uint8_t flg);

/*** System States **************************************************/
typedef enum
{
    SYSTEM_STATE_USB_START,
    SYSTEM_STATE_USB_SUSPEND,
    SYSTEM_STATE_USB_RESUME
} SYSTEM_STATE;


//#define TIMER0_100usec 0xFB50
#define TIMER0_100usec 0xFB60
#define TIMER1_10msec 0x15A0

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define ___I2C_CH1
#define ___I2C_CH2

#ifdef ___I2C_CH1
#define I2C_SCL             TRISBbits.TRISB4
#define I2C_SDA             TRISBbits.TRISB5
#define I2C_SSPSTAT         SSP1STAT
#define I2C_SSPSTAT_SMP     SSP1STATbits.SMP
#define I2C_SSPSTAT_RW      SSP1STATbits.RW2
#define I2C_SSPSTAT_P       SSP1STATbits.P
#define I2C_SSPSTAT_S       SSP1STATbits.S
#define I2C_SSPCON1         SSP1CON1
#define I2C_WCOL            SSP1CON1bits.WCOL
#define I2C_SSPEN           SSP1CON1bits.SSPEN
#define I2C_SSPCON2         SSP1CON2
#define I2C_SEN             SSP1CON2bits.SEN
#define I2C_RSEN            SSP1CON2bits.RSEN
#define I2C_PEN             SSP1CON2bits.PEN
#define I2C_RCEN            SSP1CON2bits.RCEN
#define I2C_ACKDT           SSP1CON2bits.ACKDT
#define I2C_ACKEN           SSP1CON2bits.ACKEN
#define I2C_ACKSTAT         SSP1CON2bits.ACKSTAT
#define I2C_SSPADD          SSP1ADD
#define I2C_SSPBUF          SSP1BUF
#define I2C_SSPIF           PIR1bits.SSP1IF
#define I2C_BCLIF           PIR2bits.BCL1IF
#define I2C_SSPIE           PIE1bits.SSP1IE
#define I2C_BCLIE           PIE2bits.BCL1IE
#define I2C_SSP_PRIO        IPR1bits.SSP1IP
#define I2C_BCL_PRIO        IPR2bits.BCL1IP
#endif
        
#ifdef ___I2C_CH2
#define I2C_SCL             TRISDbits.TRISD0
#define I2C_SDA             TRISDbits.TRISD1
#define I2C_SSPSTAT         SSP2STAT
#define I2C_SSPSTAT_SMP     SSP2STATbits.SMP
#define I2C_SSPSTAT_RW      SSP2STATbits.RW2
#define I2C_SSPSTAT_P       SSP2STATbits.P
#define I2C_SSPSTAT_S       SSP2STATbits.S
#define I2C_SSPCON1         SSP2CON1
#define I2C_WCOL            SSP2CON1bits.WCOL
#define I2C_SSPEN           SSP2CON1bits.SSPEN
#define I2C_SSPCON2         SSP2CON2
#define I2C_SEN             SSP2CON2bits.SEN
#define I2C_RSEN            SSP2CON2bits.RSEN
#define I2C_PEN             SSP2CON2bits.PEN
#define I2C_RCEN            SSP2CON2bits.RCEN
#define I2C_ACKDT           SSP2CON2bits.ACKDT
#define I2C_ACKEN           SSP2CON2bits.ACKEN
#define I2C_ACKSTAT         SSP2CON2bits.ACKSTAT
#define I2C_SSPADD          SSP2ADD
#define I2C_SSPBUF          SSP2BUF
#define I2C_SSPIF           PIR3bits.SSP2IF
#define I2C_BCLIF           PIR3bits.BCL2IF
#define I2C_SSPIE           PIE3bits.SSP2IE
#define I2C_BCLIE           PIE3bits.BCL2IE
#define I2C_SSP_PRIO        IPR3bits.SSP2IP
#define I2C_BCL_PRIO        IPR3bits.BCL2IP

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       
#define INTCON_RBPU         INTCON2bits.RBPU


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef ___UART_CH1
    
// PORT 設定
#define UART_TXPORT        TRISCbits.TRISC6
#define UART_RXPORT        TRISCbits.TRISC7
    
// UART1モジュールの設定
#define UART_TXSTA             TXSTA1
#define UART_TXSTA_TXEN        TXSTA1bits.TXEN
#define UART_TXSTA_BRGH        TXSTA1bits.BRGH
#define UART_TXSTA_TRMT        TXSTA1bits.TRMT
#define UART_RCSTA              RCSTA1
#define UART_RCSTA_SPEN        RCSTA1bits.SPEN
#define UART_RCSTA_CREN        RCSTA1bits.CREN
#define UART_BAUDCON            BAUDCON1
#define UART_BAUDCON_BRG16      BAUDCON1bits.BRG16
#define UART_SPBRGH             SPBRGH1
#define UART_SPBRG              SPBRG1
#define UART_TXREG              TXREG1
#define UART_RCREG              RCREG1

// 割込みモジュールの設定
#define UART_RCIF              PIR1bits.RC1IF
#define UART_RCIE              PIE1bits.RC1IE
#define UART_RCIP              PIR1bits.RC1IP
#define UART_TXIF              PIR1bits.TX1IF
#define UART_TXIE              PIE1bits.TX1IE
#define UART_TXIP              PIR1bits.TX1IP

#else

// PORT 設定
#define UART_TXPORT        TRISBbits.TRISB1
#define UART_RXPORT        TRISBbits.TRISB0
    
// UART2モジュールの設定
#define UART_TXSTA             TXSTA2
#define UART_TXSTA_TXEN        TXSTA2bits.TXEN
#define UART_TXSTA_BRGH        TXSTA2bits.BRGH
#define UART_TXSTA_TRMT        TXSTA2bits.TRMT
#define UART_RCSTA              RCSTA2
#define UART_RCSTA_SPEN        RCSTA2bits.SPEN
#define UART_RCSTA_CREN        RCSTA2bits.CREN
#define UART_BAUDCON            BAUDCON2
#define UART_BAUDCON_BRG16      BAUDCON2bits.BRG16
#define UART_SPBRGH             SPBRGH2
#define UART_SPBRG              SPBRG2
#define UART_TXREG              TXREG2
#define UART_RCREG              RCREG2

// 割込みモジュールの設定
#define UART_RCIF              PIR3bits.RC2IF
#define UART_RCIE              PIE3bits.RC2IE
#define UART_RCIP              PIR3bits.RC2IP
#define UART_TXIF              PIR3bits.TX2IF
#define UART_TXIE              PIE3bits.TX2IE
#define UART_TXIP              PIR3bits.TX2IP


#endif  // ___UART_CH1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef ___UART485_CH2
    
// PORT 設定
#define UART485_TXPORT        TRISBbits.TRISB1
#define UART485_RXPORT        TRISBbits.TRISB0
    
// UART1モジュールの設定
#define UART485_TXSTA             TXSTA2
#define UART485_TXSTA_TXEN        TXSTA2bits.TXEN
#define UART485_TXSTA_BRGH        TXSTA2bits.BRGH
#define UART485_TXSTA_TRMT        TXSTA2bits.TRMT
#define UART485_RCSTA             RCSTA2
#define UART485_RCSTA_SPEN        RCSTA2bits.SPEN
#define UART485_RCSTA_RX9         RCSTA2bits.RX9
#define UART485_RCSTA_ADDEN       RCSTA2bits.ADDEN
#define UART485_RCSTA_CREN        RCSTA2bits.CREN
#define UART485_RCSTA_RX9D        RCSTA2bits.RX9D
#define UART485_BAUDCON           BAUDCON2
#define UART485_BAUDCON_BRG16     BAUDCON2bits.BRG16
#define UART485_SPBRGH            SPBRGH2
#define UART485_SPBRG             SPBRG2
#define UART485_TXREG             TXREG2
#define UART485_RCREG             RCREG2

// 割込みモジュールの設定
#define UART485_RCIF              PIR3bits.RC2IF
#define UART485_RCIE              PIE3bits.RC2IE
#define UART485_RCIP              PIR3bits.RC2IP
#define UART485_TXIF              PIR3bits.TX2IF
#define UART485_TXIE              PIE3bits.TX2IE
#define UART485_TXIP              PIR3bits.TX2IP

#endif  // ___UART485_CH2




typedef enum{
    LOG_DISP_I2C,
    LOG_DISP_I2C_STAT,
    LOG_DISP_GET_TM,
    LOG_DISP_SET_TM,
    LOG_DISP_REL_TM,
    LOG_DISP_NO_DATA,
            
    LOG_DISP_ETC
            
}LOG_DISP_KIND;

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
void SYSTEM_Initialize( SYSTEM_STATE state );

uint16_t Get_Timer(int sel);
int Set_Timer(uint16_t dt);
void ssCount_Timer(void);
void Rel_Timer(int sel);
void Init_Timer(void);

void Wait(uint16_t num);

/*********************************************************************
* Function: void SYSTEM_Tasks(void)
*
* Overview: Runs system level tasks that keep the system running
*
* PreCondition: System has been initalized with SYSTEM_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
//void SYSTEM_Tasks(void);
//#define SYSTEM_Tasks()

#endif //SYSTEM_H
