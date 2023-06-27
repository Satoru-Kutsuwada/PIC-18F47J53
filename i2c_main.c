
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
#include "main.h"

#define I2C_LOG_DISPLAY    

#define I2C_SEND_ACK  0 
#define I2C_SEND_NACK 1 

#define STATUS_OK              0x00
#define STATUS_FAIL            0x01

// I2C　リカバリー回数
#define I2C_RECOVRY_MAX           10
 

#define MAX_MULTI_CNT 20
#define RESTART_CNT_MAX 10

typedef enum{
    I2C_STAT_START = 0,
    I2C_STAT_WITE_ADDRESS_TX,
    I2C_STAT_READ_ADDRESS_TX,
    I2C_STAT_INDEX_TX,
    I2C_STAT_DATA_TX,
    I2C_STAT_STOP,
    I2C_STAT_SUSPEND,
    I2C_STAT_RESTART,
    I2C_STAT_READ,
    I2C_STAT_END,
    I2C_STAT_MAX
            
            
}I2C_STAT_STEP; //i2c_data.status


#define I2C_STRING_LENGTH 20
 typedef struct{
     char string[ I2C_STRING_LENGTH];
 } I2C_STRING_LIST;
 
 const I2C_STRING_LIST i2c_list[] = {
    "START",
    "WITE_ADDRESS_TX",
    "READ_ADDRESS_TX",
    "INDEX_TX",
    "DATA_TX",
    "STOP",
    "SUSPEND",
    "RESTART",
    "READ",
    "END"
 };




typedef enum{
    I2C_NO_ERROR = 0,

    I2C_ERR_STAT_BCL,        
    I2C_ERR_STAT_WCOL,        
    I2C_ERR_STAT_TIMEOUT,        
    I2C_ERR_STAT_NACK,
    I2C_ERR_STAT_ADDRESS_NACK,
    I2C_ERR_STAT_INDEX_NACK,
    I2C_ERR_STAT_DATA_NACK,
            
    I2C_ERROR_ETC
}I2C_ERROR; //i2c_data.status



typedef enum{
    I2C_WRITE_MULTI = 1,
    I2C_READ_MULTI
}I2C_OPERATION_TYPE;


typedef struct{
    I2C_STAT_STEP status;
    I2C_ERROR error;
    I2C_OPERATION_TYPE op_type;
    uint8_t address;
    uint8_t index;
    uint8_t count;
    uint8_t count_step;
    uint8_t dt[MAX_MULTI_CNT];
    uint8_t restart_cnt;
    
}I2C_DATA;

I2C_DATA i2c_data;
uint8_t bcl_error_chk;


        
/** ProtoType *******************************************************/
int32_t i2c_readMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count);
int32_t i2c_writeMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count);
uint8_t i2c_CheckIdleWait(uint8_t mask);
uint8_t i2c_start(void);
uint8_t i2c_stop(void);
uint8_t i2c_write( uint8_t dt );
uint8_t i2c_read( uint8_t acknNak, uint8_t *dt );

bool i2c_CheckIdle(void);
void i2c_interrupt_write(void);
void i2c_interrupt_read(void);
void i2c_int_start_condition(void);

extern void RegiDisp(void);
extern void SetLogData(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4);
extern void SetLogDataM(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4);
extern void print_string(char *string);
extern uint8_t log_onoff(void);
extern uint8_t get_log_onoff(void);

/** typedef *******************************************************/







/********************************************************************
 * Function:
 * 
 *******************************************************************/
#ifdef ___NOP
void i2c_Reg_log(char *string,uint8_t dt,uint8_t flg)
{
#ifdef I2C_LOG_DISPLAY    
    rtc_disp_log(0);
    printf(" SSPxIF=%x", I2C_SSPIF);
    printf(", SSPxSTAT=%x",I2C_SSPSTAT);
    printf(", SSPxCON1=%x",I2C_SSPCON1);
    printf(", SSPxCON2=%x  ",I2C_SSPCON2);
    printf(string);
    if( flg == 1 )
        printf("%x",dt);
    printf("\r\n");
#endif    
}

/********************************************************************
 * Function:
 * 
 *******************************************************************/
void i2c_log2(char *string)
{
#ifdef I2C_LOG_DISPLAY    

    rtc_disp_log(0);
    printf(string);
#endif    
}
                
#endif    

    
/********************************************************************
 * Function:        void i2c_init(void)
 *
 *    speed : I2Cの通信速度を指定(0=100KHz 1=400KHz)                            *
*                                                                              *
*    注)クロック8MHzでの設定です、他のクロックはSSPADDを変更する必要が有ります *
*             4MHz  8MHz  16MHz  32MHz  40MHz  48MHz                           *
*    100KHz   0x09  0x13   0x27   0x4F   0x63   0x77                           *
*    400kHz         0x04   0x09   0x13   0x18   0x1D                           
 ********************************************************************/

void i2c_init(uint8_t sel)
{


#ifdef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    printf("i2c_init(%d)\r\n",sel);
     I2C_SSPCON1= 0;
     
     Wait(1);

    I2C_SCL = 1;
    I2C_SDA = 1;

     I2C_SSPSTAT= 0b10000000 ;     // 標準速度モードに設定する(100kHz/1MHz)
     I2C_SSPCON1= 0b00101000 ;     // SDA/SCLピンはI2Cで使用し、マスターモードとする
     if (sel == 0) {
          I2C_SSPADD = 0x77       ;   // クロック=FOSC/((SSPADD + 1)*4) 48MHz/((0x13+1)*4)=0.1(100KHz)
     } else {
          I2C_SSPADD = 0x1D       ;   // クロック=FOSC/((SSPADD + 1)*4) 48MHz/((0x04+1)*4)=0.4(400KHz)          
          I2C_SSPSTAT_SMP = 0 ;       // 高速速度モードに設定する(400kHz)
     }
     //I2C_SSPIE = 1 ;               // SSP(I2C)割り込みを許可する
     //I2C_BCLIE = 1 ;               // MSSP(I2C)バス衝突割り込みを許可する
     PEIE      = 1 ;               // 周辺装置割り込みを許可する
     GIE       = 1 ;               // 全割り込み処理を許可する 
     I2C_SSPIF = 0 ;               // SSP(I2C)割り込みフラグをクリアする
     I2C_BCLIF = 0 ;               // MSSP(I2C)バス衝突割り込みフラグをクリアする

     
#ifdef __SSP_ISR_HIGH
    I2C_SSP_PRIO = 1;
    I2C_BCL_PRIO = 1;
#else
    I2C_SSP_PRIO = 0;
    I2C_BCL_PRIO = 0;
#endif
     
     
     INTCON_RBPU = 1;
     

#endif //__I2C_MASTER ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
#ifndef __I2C_MASTER  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    uint8_t dt;

        //==========================================================================
        // TRISD 
        //==========================================================================
        I2C_SCL = 1;
        I2C_SDA = 1;

        //==========================================================================
        // SSP1STAT 
        //==========================================================================
        I2C_SSPSTAT = 0x00;

        //==========================================================================
        // SSP1STAT 
        //==========================================================================
        I2C_SSPCON1 = 0x28;

        //==========================================================================
        // SSP1STAT 
        //==========================================================================
        I2C_SSPCON2 = 0;

        //==========================================================================
        // SSP1ADD 
        //==========================================================================
        I2C_SSPADD = 0;


        dt = (_XTAL_FREQ / (4*I2C_CLOCK)) - 1 ;

        if( dt > 127 ){
            i2c_log2("\r\nI2C Clock Error\r\n");
        }
        else{
            printf("\r\nI2C SSPADD=0x%x\r\n",dt,0,0);
            I2C_SSPADD = dt;
        }   
        printf("  SSP1ADD=%x\r\n",SSP1ADD);

        //==========================================================================
        // PIR3 
        //==========================================================================
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;
        //==========================================================================
        // PIR3 
        //==========================================================================
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;
#endif //__I2C_MASTER ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
}
/********************************************************************
 * Function:        
 *
 *******************************************************************/
void i2c_test(void)
{
    uint8_t dst[5];
    
#ifdef ___NOP    
    LATBbits.LATB2 = 0;
    
    i2c_init(1);
    
    RegiDisp();
    Wait(1);
#endif    
    
    dst[0] = 0x00;
    i2c_writeMulti(0x52,0x88,dst,1);
    
    i2c_readMulti(0x52, 0xf8,dst,2);
    printf("dst[0]=0x%x,dst[1]=0x%x,dst[2]=0x%x,dst[3]=0x%x,dst[4]=0x%x\r\n",dst[0],dst[1],dst[2],dst[3],dst[4]);
    
}

#ifdef ___NOP
void i2c_test(void)
{
    
    bool status;
    
    LATBbits.LATB2 = 0;
    
    i2c_init(1);
    
    RegiDisp();
    Wait(1);
    
       // i2c_log2("i2c_test\r\n");
    while( i2c_CheckIdle() == true ){
        //i2c_log2("i2c_CheckIdle() == true\r\n");

        //======================================================================
        //  START
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(1)\r\n");
            break;
        }
        
        status = i2c_start();
        if( status  == false ){
            i2c_log2("Error start()\r\n");
            break;
        }

        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(2)\r\n");
            break;
        }

        status = i2c_write(0x52);
        if( status  == false ){
            i2c_log2("Error ADD Write()\r\n");
            break;
        }

        status = i2c_write(0x88);
        if( status  == false ){
            i2c_log2("Error Index Writet()\r\n");
            break;
        }
        status = i2c_write(0x55);
        if( status  == false ){
            i2c_log2("Error Data writet()\r\n");
            break;
        }
        status = i2c_stop();
        if( status  == false ){
            i2c_log2("Error stop()\r\n");
            break;
        }
    }
    
    if( status  == false ){
        status = i2c_stop();
    }
}
#endif  // ___NOP

/********************************************************************
 * Function:        void i2c_init(void)
 *
 *******************************************************************/

bool i2c_CheckIdle(void)
{
    bool rtn = true;


    if ((I2C_SSPSTAT & 0x04) || (I2C_SSPCON2 & 0x1F)){
        rtn = false;            
    }
    
    return rtn;
}

#ifdef ___NOP
#endif  // ___NOP

#ifndef  __I2C_INTERRUPT

//=============================================================================
//
//=============================================================================
uint8_t i2c_CheckIdleWait(uint8_t mask)
{
    uint8_t status;
    int index;

    status = STATUS_OK ;    

    index = Set_Timer(100);
    while ((I2C_SSPSTAT & mask) || (I2C_SSPCON2 & 0x1F)){
        if( Get_Timer(index) == 0 ){
            SetLogDataM( LOG_DISP_I2C, "ERR(TOUT): CheckIdle",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            i2c_data.error = I2C_ERR_STAT_TIMEOUT;        
            status = STATUS_FAIL;    
            break;   
        }
    }
    Rel_Timer(index);
    
    return  status;
}



//=============================================================================
//
//=============================================================================
uint8_t i2c_start(void)
{
    uint8_t status;
    int index;

    status = STATUS_OK ;    
    bcl_error_chk = 0;
    
    status = i2c_CheckIdleWait(0x05);

    if( status == STATUS_OK ){
        SetLogDataM( LOG_DISP_I2C, "START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        

        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        I2C_SEN = 1;

        index = Set_Timer(100);
        while( I2C_SSPIF == 0 ){
            if(bcl_error_chk){
                SetLogDataM( LOG_DISP_I2C, "ERR(BCL): START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_BCL;      
                status = STATUS_FAIL;    
            }
            if(I2C_WCOL){
                SetLogDataM( LOG_DISP_I2C, "ERR(WCOL): START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                I2C_WCOL = 0;
                i2c_data.error = I2C_ERR_STAT_WCOL;        
                status = STATUS_FAIL;    
                break         ;   
            }
            if( Get_Timer(index) == 0 ){
                SetLogDataM( LOG_DISP_I2C, "ERR(TOUT): START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_TIMEOUT;        
                status = STATUS_FAIL;    
                break         ;   
            }
        }
        Rel_Timer(index);
    }    


    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_stop(void)
{
    uint8_t status;
    int index;

    status = STATUS_OK ;    
    bcl_error_chk = 0;

    status = i2c_CheckIdleWait(0x05);

    if( status == STATUS_OK ){
        SetLogDataM( LOG_DISP_I2C, "STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);

        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        I2C_PEN = 1;

        index = Set_Timer(100);
        while(I2C_SSPIF == 0 )
        {
            if(bcl_error_chk){
                SetLogDataM( LOG_DISP_I2C, "ERR(BCL): STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_BCL;      
                status = STATUS_FAIL;    
            }
            if(I2C_WCOL){
                SetLogDataM( LOG_DISP_I2C, "ERR(WCOL): STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                I2C_WCOL = 0;
                i2c_data.error = I2C_ERR_STAT_WCOL;        
                status = STATUS_FAIL;    
                break;           
            }
            if( Get_Timer(index) == 0 ){
                SetLogDataM( LOG_DISP_I2C, "ERR(TOUT): STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_TIMEOUT;        
                status = STATUS_FAIL;    
                break;   
            }
        }
        Rel_Timer(index);
    }

    return  status;
}


//=============================================================================
//
//=============================================================================
uint8_t i2c_write( uint8_t dt )
{
    uint8_t status;
    int index;

    status = STATUS_OK ;    
    bcl_error_chk = 0;

    status = i2c_CheckIdleWait(0x05);

    if( status == STATUS_OK ){
        if( i2c_data.status < I2C_STAT_MAX ){
            SetLogDataM( LOG_DISP_I2C, &i2c_list[i2c_data.status].string[0],I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,dt);
        }
        else{
            printf("ERROR STATUS = %d\r\n",i2c_data.status);
        }
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        I2C_SSPBUF = dt;

        index = Set_Timer(100);
        while( I2C_SSPIF == 0 ) {
            if(bcl_error_chk){
                SetLogDataM( LOG_DISP_I2C, "ERR(BCL): WRITE",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_BCL;      
                status = STATUS_FAIL;    
            }
            if(I2C_WCOL){
                SetLogDataM( LOG_DISP_I2C, "ERR(WCOL): WRITE",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                I2C_WCOL = 0;
                i2c_data.error = I2C_ERR_STAT_WCOL;        
                status = STATUS_FAIL;    
                break;           
            }
            if( Get_Timer(index) == 0 ){
                SetLogDataM( LOG_DISP_I2C, "ERR(TOUT): WRITE",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_TIMEOUT;        
                status = STATUS_FAIL;    
                break;   
            }
        }
        Rel_Timer(index);
    
        if( status == STATUS_OK ){
            if( I2C_ACKSTAT ){
                SetLogDataM( LOG_DISP_I2C, "ERR(NACK): WRITE",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,dt);
                i2c_data.error = I2C_ERR_STAT_NACK;
                status = STATUS_FAIL;    
            }
            else{
                SetLogDataM( LOG_DISP_I2C, "WRITE ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,dt);
            }
        }
    }
    
    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_read( uint8_t acknNak, uint8_t *dt )
{
    uint8_t status;
    int index;

    status = STATUS_OK ;    
    bcl_error_chk = 0;

    status = i2c_CheckIdleWait(0x05);

    if( status == STATUS_OK ){
        SetLogDataM( LOG_DISP_I2C, "READ START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);

        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        I2C_RCEN   = 1;

        index = Set_Timer(100);
        while(I2C_SSPIF == 0 ){
            if(bcl_error_chk){
                SetLogDataM( LOG_DISP_I2C, "ERR(BCL): READ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_BCL;      
                status = STATUS_FAIL;    
            }
            if(I2C_WCOL){
                SetLogDataM( LOG_DISP_I2C, "ERR(WCOL): READ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                I2C_WCOL = 0;
                i2c_data.error = I2C_ERR_STAT_WCOL;        
                status = STATUS_FAIL;    
                break;           
            }
            if( Get_Timer(index) == 0 ){
                SetLogDataM( LOG_DISP_I2C, "ERR(TOUT): READ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.error = I2C_ERR_STAT_TIMEOUT;        
                status = STATUS_FAIL;    
                break;   
            }
        }
        Rel_Timer(index);
    }
    
    if( status == STATUS_OK ){
        *dt = I2C_SSPBUF;
    
        status = i2c_CheckIdleWait(0x05);

        if( status == STATUS_OK ){
            I2C_ACKDT = acknNak ;        // ACKデータのセット
            I2C_ACKEN = 1 ;          // ACKデータを返す
        }
        SetLogDataM( LOG_DISP_I2C, "READ END",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,*dt);
    }

    return  status;
}
//=============================================================================
//
//=============================================================================
int32_t i2c_readMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t acknack;
    uint8_t status;
    
    if( get_log_onoff() > 0  ) {
        printf(" i2c_readMulti(Add=0x%x,Indx=0x%x,Cnt=%d)\r\n",address,reg,count);
    }
    status = STATUS_OK ;    


    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // I2C START
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        I2C_SSPEN = 1;
        I2C_ACKDT =0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // データ設定
        //++++++++++++++++++++++++++++++++++++++++++++++++++

        i2c_data.address = address;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.restart_cnt = 0;
        i2c_data.op_type = I2C_READ_MULTI;
        i2c_data.status = I2C_STAT_START;
        i2c_data.error = I2C_NO_ERROR;

        for( i=0; i<count; i++ ){
            i2c_data.dt[i] = 0;
        }

        // 割込みフラグクリア
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        // 割込みイネーブル
        I2C_SSPIE = 0;
        I2C_BCLIE = 1; 
        
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // 処理
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        while( status == STATUS_OK ){
            //SetLogDataM( LOG_DISP_I2C_STAT, "STATUS",i2c_data.status,0,0,0);
            switch( i2c_data.status ){
                case I2C_STAT_START:
                    status = i2c_start();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_WITE_ADDRESS_TX;
                    }
                    break;
                case I2C_STAT_WITE_ADDRESS_TX:
                    status = i2c_write(i2c_data.address);
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_INDEX_TX;
                    }
                    break;
                case I2C_STAT_INDEX_TX:
                    status = i2c_write(i2c_data.index);
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_SUSPEND;
                    }
                    break;
                case I2C_STAT_SUSPEND:
                    status = i2c_stop();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_RESTART;
                    }
                    break;

                case I2C_STAT_RESTART:
                    status = i2c_start();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_READ_ADDRESS_TX;
                    }
                    break;

                case I2C_STAT_READ_ADDRESS_TX:
                    status = i2c_write(i2c_data.address|0x01);
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_READ;
                    }
                    break;
                case I2C_STAT_READ:
                    
                    if( i2c_data.count_step < i2c_data.count ){
                        if( i2c_data.count_step  < (i2c_data.count-1) )
                            acknack = I2C_SEND_ACK;
                        else
                            acknack = I2C_SEND_NACK;
                       
                        status = i2c_read( acknack, &i2c_data.dt[i2c_data.count_step] );
                        i2c_data.count_step ++;
                    }
                    
                    if( status == STATUS_OK ){
                        if( i2c_data.count_step >= i2c_data.count ){
                            i2c_data.status = I2C_STAT_STOP;
                        }
                    }
                    break;
                case I2C_STAT_STOP:
                    status = i2c_stop();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_END;
                    }
                    break;
                default:
                    printf("ERROR (READ MAIN) STATUS = %d\r\n",i2c_data.status);
                    break;
            }
            
            if( status == STATUS_OK ){
                if( i2c_data.status == I2C_STAT_END ){
                    // 正常終了
                    SetLogDataM( LOG_DISP_NO_DATA, "I2C END",0,0,0,0);
                    

                    for( i=0; i<i2c_data.count ; i++ ){
                        *dst = i2c_data.dt[i];
                        dst ++;
                    }
                    
                    break;
                }
            }
            else{
                // リカバリー処理　今は強制終了
                I2C_SSPEN = 0;
                I2C_SSPIE = 0;
                I2C_BCLIE = 0; 

                
                Wait(1);
                
                if(i2c_data.restart_cnt <I2C_RECOVRY_MAX){
                    i2c_data.restart_cnt ++;

                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    // I2C START
                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    I2C_SSPEN = 1;

                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    // データ設定
                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    status = STATUS_OK;
                    i2c_data.count_step = 0;
                    i2c_data.status = I2C_STAT_START;
                    i2c_data.error = I2C_NO_ERROR;

                    // 割込みフラグクリア
                    I2C_SSPIF = 0;
                    I2C_BCLIF = 0;

                    // 割込みイネーブル
                    I2C_SSPIE = 0;
                    I2C_BCLIE = 1; 

                    SetLogDataM( LOG_DISP_I2C, "RESTART",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                }
                else{
                    SetLogDataM( LOG_DISP_I2C, "ERROR_END",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                }
            }
        }
    }   

    if( get_log_onoff() > 0  ) {
        if(status == STATUS_OK){
            switch( count ){
            case 1:
                printf("   Read data = 0x%x\r\n",address,reg,count,dst[0]);
                break;
            case 2:
                printf("   Read data = 0x%x,0x%x\r\n",address,reg,count,dst[0],dst[1]);
                break;
            case 4:
            default:
                printf("   Read data = 0x%x,0x%x,0x%x,0x%x\r\n",address,reg,count,dst[0],dst[1],dst[2],dst[3]);
                break;
            }
        }
    }
    return  status;
}       


//=============================================================================
//
//=============================================================================
int32_t i2c_writeMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t *dt;
    uint8_t status;
    int index;
    
    
    status = STATUS_OK ;    
    
    if( get_log_onoff() > 0  ) {
        switch( count ){
        case 1:
            printf(" i2c_writeMulti(Add=0x%x,Indx=0x%x,Cnt=%d,Dt=0x%x)\r\n",address,reg,count,dst[0]);
            break;
        case 2:
            printf(" i2c_writeMulti(Add=0x%x,Indx=0x%x,Cnt=%d,Dt=0x%x,0x%x)\r\n",address,reg,count,dst[0],dst[1]);
            break;
        case 4:
        default:
            printf(" i2c_writeMulti(Add=0x%x,Indx=0x%x,Cnt=%d,Dt=0x%x,0x%x,0x%x,0x%x)\r\n",address,reg,count,dst[0],dst[1],dst[2],dst[3]);
            break;
        }
    }    
    
    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // I2C START
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        I2C_SSPEN = 1;
        I2C_ACKDT =0;
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // データ設定
        //++++++++++++++++++++++++++++++++++++++++++++++++++

        i2c_data.address = address;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.restart_cnt = 0;
        i2c_data.op_type = I2C_WRITE_MULTI;
        i2c_data.status = I2C_STAT_START;
        i2c_data.error = I2C_NO_ERROR;

        for( i=0; i<count; i++ ){
            i2c_data.dt[i] = *dst;
//            printf("dt=0x%x,",*dst);
            dst ++;
        }
//            printf("\r\n");

        // 割込みフラグクリア
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        // 割込みイネーブル
        I2C_SSPIE = 0;
        I2C_BCLIE = 1; 
        
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // 処理
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        while( status == STATUS_OK ){
            //SetLogDataM( LOG_DISP_I2C_STAT, "STATUS",i2c_data.status,0,0,0);
            switch( i2c_data.status ){
                case I2C_STAT_START:
                    status = i2c_start();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_WITE_ADDRESS_TX;
                    }
                    break;
                case I2C_STAT_WITE_ADDRESS_TX:
                    status = i2c_write(i2c_data.address);
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_INDEX_TX;
                    }
                    break;
                case I2C_STAT_INDEX_TX:
                    status = i2c_write(i2c_data.index);
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_DATA_TX;
                    }
                    break;
                case I2C_STAT_DATA_TX:
                    if( i2c_data.count_step < i2c_data.count ){
                        dt = i2c_data.dt[i2c_data.count_step];
                        i2c_data.count_step ++;
                        status = i2c_write(dt);
                    }

                    if( status == STATUS_OK ){
                        if( i2c_data.count_step >= i2c_data.count ){
                            i2c_data.status = I2C_STAT_STOP;
                        }
                    }
                    break;
                case I2C_STAT_STOP:
                    status = i2c_stop();
                    if( status == STATUS_OK ){
                        i2c_data.status = I2C_STAT_END;
                    }
                    break;
                default:
                    printf("ERROR (WRITE MAIN) STATUS = %d\r\n",i2c_data.status);

                    break;
            }
            
            if( status == STATUS_OK ){
                if( i2c_data.status == I2C_STAT_END ){
                    // 正常終了
                    SetLogDataM( LOG_DISP_NO_DATA, "I2C END",0,0,0,0);
                    break;
                }
            }
            else{
                // リカバリー処理　今は強制終了
                I2C_SSPEN = 0;
                I2C_SSPIE = 0;
                I2C_BCLIE = 0; 
            
                Wait(1);
                
                if(i2c_data.restart_cnt < I2C_RECOVRY_MAX){
                    i2c_data.restart_cnt ++;

                    
                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    // I2C START
                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    I2C_SSPEN = 1;

                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    // データ設定
                    //++++++++++++++++++++++++++++++++++++++++++++++++++
                    status = STATUS_OK;

                    i2c_data.count_step = 0;
                    i2c_data.status = I2C_STAT_START;
                    i2c_data.error = I2C_NO_ERROR;

                    // 割込みフラグクリア
                    I2C_SSPIF = 0;
                    I2C_BCLIF = 0;

                    // 割込みイネーブル
                    I2C_SSPIE = 0;
                    I2C_BCLIE = 1; 
                    
                    SetLogDataM( LOG_DISP_I2C, "RESTART",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                }
                else{
                    SetLogDataM( LOG_DISP_I2C, "ERROR_END",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                }
            }
        }
    }   

    return  status;
}       
            
#endif  //  __I2C_INTERRUPT


#ifdef ___NOP

bool i2c_read(uint8_t acknNak, uint8_t *dt)
{
    int index;
    bool rtn = true;

    rtn = i2c_CheckIdleWait(0x05);
    if( rtn == false ){
        return rtn;    
    }

    i2c_Reg_log("i2c_read(1)",0,0);
    
    I2C_SSPIF = 0;
    I2C_BCLIF = 0;
    
    I2C_RCEN   = 1;
    
    index = Set_Timer(100);
    while(I2C_SSPIF == 0 )
    {
        if(I2C_WCOL){
            I2C_WCOL = 0;
            i2c_log2("Error:i2c_read() WCOL\r\n");
            rtn = false;        
            break ;           
        }
        if( Get_Timer(index) == 0 ){
            i2c_log2("Error: i2c_read() Time out\r\n");
            rtn = false;        
            break         ;   
        }
    }
    I2C_SSPIF = 0;
    Rel_Timer(index);

    i2c_Reg_log("i2c_read(2)",0,0);

    *dt = I2C_SSPBUF;

    if( rtn == true ){
        rtn = i2c_CheckIdleWait(0x05);
        if( rtn == false ){
            return rtn;    
        }

        I2C_ACKDT = acknNak ;        // ACKデータのセット
        I2C_ACKEN = 1 ;          // ACKデータを返す
    }

    return rtn;
}
#endif



#ifdef  __I2C_INTERRUPT


//-------------------------------------------------------------------------------------
// Read an arbitrary number of bytes from the sensor, starting at the given
// register, into the given array
//-------------------------------------------------------------------------------------
int32_t i2c_readMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t *dt;
    uint8_t status;
    
    status = STATUS_OK ;    
    
    printf("\r\ni2c_readMulti(ad=%x, index=%x,cnt=%x\r\n",address,reg,count);

    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        while( i2c_CheckIdle() != true );

        i2c_data.address = address;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.op_type = I2C_READ_MULTI;

        i2c_data.status = I2C_STAT_START;
        
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        I2C_SEN = 1;
        
        I2C_SSPIE = 1;
        I2C_BCLIE = 1; 

        while( i2c_data.status != I2C_STAT_END ){
            if( i2c_data.status == I2C_STAT_ADDRESS_NACK )
            {
                status = STATUS_FAIL;          
                break;
            }
        }
    }

    return status;
}
#endif


#ifdef ___NOP
int32_t i2c_readMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)

{
    uint8_t i;
    uint8_t status = true;
    
    
    printf("\r\ni2c_readMulti(ad=%x, index=%x,cnt=%x\r\n",address,reg,count);
       

    
    while( i2c_CheckIdle() == true ){
        //======================================================================
        //  START
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(1)\r\n");
            break;
        }
        
        status = i2c_start();
        if( status  == false ){
            i2c_log2("Error start()\r\n");
            break;
        }

        //======================================================================
        //  Write Address
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(2)\r\n");
            break;
        }

        status = i2c_write(address);
        if( status  == false ){
            i2c_log2("Error ADD Write()\r\n");
            break;
        }

        //======================================================================
        //  Write Index
        //======================================================================
        status = i2c_write(reg);
        if( status  == false ){
            i2c_log2("Error Index Writet()\r\n");
            break;
        }

        //======================================================================
        //  STOP
        //======================================================================

        status = i2c_stop();
        if( status  == false ){
            i2c_log2("Error stop()\r\n");
            break;
        }

        //======================================================================
        //  START
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(1)\r\n");
            break;
        }
        
        status = i2c_start();
        if( status  == false ){
            i2c_log2("Error start()\r\n");
            break;
        }

        //======================================================================
        //  Write Address|Read
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(2)\r\n");
            break;
        }

        status = i2c_write(address|0x01);
        if( status  == false ){
            i2c_log2("Error ADD Write()\r\n");
            break;
        }

        //======================================================================
        //  Read
        //======================================================================
        
        for (i = 0; i < count; i++ )
        {
            if ( i < (count-1) )
            {
                status =  i2c_read( I2C_SEND_ACK, &dst[i]);
                if( status  == false ){
                    i2c_log2("Error Read(1)\r\n");
                    break;
                }
            } 
            else 
            {
                status =  i2c_read( I2C_SEND_NACK, &dst[i]);
                if( status  == false ){
                    i2c_log2("Error Read(2)\r\n");
                    break;
                }
            }
        }

        if( status  == false ){
            break;
        }

        //======================================================================
        //  STOP
        //======================================================================

        status = i2c_stop();
        if( status  == false ){
            i2c_log2("Error stop()\r\n");
            break;
        }
        break;
    }
    
    if( status  == false ){
        status = i2c_stop();
        status = STATUS_FAIL;
    }
    else{
        status = STATUS_OK ;        
    }    
        
        return status;
}
#endif  // ___NOP


#ifdef ___NOP
int32_t i2c_writeMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{

    uint8_t *dt;
    uint8_t status;
    uint8_t i;

    
    dt = dst;
    printf("\r\ni2c_writeMulti(ad=%x, index=%x,cnt=%x\r\n  data=",address,reg,count);
    for( i=0; i<count; i++){
        printf(" %x,", *dt);
        dt ++;
    }
    printf("\r\n");
        
        
        
    dt = dst;
    
    while( i2c_CheckIdle() == true ){
        //======================================================================
        //  START
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(1)\r\n");
            break;
        }
        
        status = i2c_start();
        if( status  == false ){
            i2c_log2("Error start()\r\n");
            break;
        }

        //======================================================================
        //  Write Address
        //======================================================================
        status = i2c_CheckIdleWait(0x05);
        if( status  == false ){
            i2c_log2("Error IdleCheck(2)\r\n");
            break;
        }

        status = i2c_write(address);
        if( status  == false ){
            i2c_log2("Error ADD Write()\r\n");
            break;
        }

        //======================================================================
        //  Write Index
        //======================================================================
        status = i2c_write(reg);
        if( status  == false ){
            i2c_log2("Error Index Writet()\r\n");
            break;
        }

            
        //======================================================================
        //  Data write
        //======================================================================
        for (i = 0; i < count; i++) 
        {
            status = i2c_write( *dt );
            if( status  == false ){
                i2c_log2("Error Data Writet()\r\n");
                break;
            }
            dt ++;
        }

        if( status  == false ){
                break;
        }

        //======================================================================
        //  STOP
        //======================================================================

        status = i2c_stop();
        if( status  == false ){
            i2c_log2("Error stop()\r\n");
            break;
        }

    }
    
    if( status  == false ){
        status = i2c_stop();
        status = STATUS_FAIL;
    }
    else{
        status = STATUS_OK ;        
    }    
        
    return status;    

}

#endif  // ___NOP

#ifdef ___NOP

void Newi2c_start(void)
{
    I2C_SSPIF = 0;
    I2C_SEN = 1;
}

void Newi2c_write(uint8_t dt)
{
    I2C_SSPIF = 0;
    I2C_SSPBUF = dt;
}

#endif  // ___NOP

#ifndef  __I2C_INTERRUPT



#ifdef ___NOP

//=============================================================================
//
//=============================================================================
uint8_t i2c_main_start(void)
{
    if( I2C_SSPIF == 0 ){
        SetLogDataM( LOG_DISP_I2C, "START_REQ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_START_REQ;
        I2C_SEN = 1;
    }   
    return  STATUS_OK;
}
            
//=============================================================================
//
//=============================================================================
uint8_t i2c_main_start_resp(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 1 ){       //if( I2C_SEN == 0){
        I2C_SSPIF = 0;
        SetLogDataM( LOG_DISP_I2C, "START_OK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_ADDRESS_TX;
    }
    else if( Get_Timer(index) == 0 ){
        SetLogDataM( LOG_DISP_I2C, "START TOUT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        status = STATUS_FAIL;  
        i2c_data.status = I2C_STAT_START_TOUT;
    }
    
    return  status;
}
//=============================================================================
//
//=============================================================================
uint8_t i2c_main_address_tx(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 0 ){
        SetLogDataM( LOG_DISP_I2C, "ADDRESS TX",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.address);
        i2c_data.status = I2C_STAT_ADDRESS_RESPONS;
        I2C_SSPBUF = i2c_data.address; 
    }

    return  status;
}
//=============================================================================
//
//=============================================================================
uint8_t i2c_main_address_resp(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 1 ){
        I2C_SSPIF = 0;
        if( I2C_ACKSTAT ){
            SetLogDataM( LOG_DISP_I2C, "ADDRESS_NACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            status = STATUS_FAIL;  
            i2c_data.status = I2C_STAT_ADDRESS_NACK;
            // エラー
        }
        else{
            SetLogDataM( LOG_DISP_I2C, "ADDRESS_ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            i2c_data.status = I2C_STAT_INDEX_TX;
        }
    }
    else if( Get_Timer(index) == 0 ){
        SetLogDataM( LOG_DISP_I2C, "ADDRESS_RESS TOUT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_ADDRESS_TOUT;
        status = STATUS_FAIL;  
    }

    return  status;
}
//=============================================================================
//
//=============================================================================
uint8_t i2c_main_index_tx(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    SetLogDataM( LOG_DISP_I2C, "INDEX TX",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.index);
    i2c_data.status = I2C_STAT_INDEX_RESPONS;
    I2C_SSPBUF = i2c_data.index;

    return  status;
}
//=============================================================================
//
//=============================================================================
uint8_t i2c_main_index_resp(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 1 ){
        I2C_SSPIF = 0;
        if( I2C_ACKSTAT ){
            SetLogDataM( LOG_DISP_I2C, "INDEX_NACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            status = STATUS_FAIL;  
            i2c_data.status = I2C_STAT_INDEX_NACK;
            // エラー
        }
        else{
            SetLogDataM( LOG_DISP_I2C, "INDEX_ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            i2c_data.status = I2C_STAT_DATA_TX;
        }
    }
    else if( Get_Timer(index) == 0 ){
        SetLogDataM( LOG_DISP_I2C, "INDEX_RESS TOUT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        status = STATUS_FAIL;  
        i2c_data.status = I2C_STAT_INDEX_RESS_TOUT;

    }

    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_main_data_tx(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( i2c_data.count_step < i2c_data.count ){
        SetLogDataM( LOG_DISP_I2C, "DATA TX",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_DATA_RESPONS;
        i2c_data.count_step ++;
        I2C_SSPIF = 0;
        I2C_SSPBUF = i2c_data.dt[i2c_data.count_step];
    }
    else{
        SetLogDataM( LOG_DISP_I2C, "DATA END",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_STOP_REQ;
    }

    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_main_data_res(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 1 ){
        I2C_SSPIF = 0;
        if( I2C_ACKSTAT ){
            SetLogDataM( LOG_DISP_I2C, "INDEX_NACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            status = STATUS_FAIL;  
            i2c_data.status = I2C_STAT_DATA_NACK;
            // エラー
        }
        else{
            SetLogDataM( LOG_DISP_I2C, "INDEX_ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            i2c_data.status = I2C_STAT_DATA_TX;
        }
    }
    else if( Get_Timer(index) == 0 ){
        Rel_Timer(index);
        SetLogDataM( LOG_DISP_I2C, "DATA_RESS TOUT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        status = STATUS_FAIL;  
        i2c_data.status = I2C_STAT_DATA_RESS_TOUT;
    }

    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_main_stop_req(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    SetLogDataM( LOG_DISP_I2C, "STOP REQ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
    i2c_data.status = I2C_STAT_STOP;
    I2C_SSPIF = 0;
    I2C_PEN = 1;

    return  status;
}

//=============================================================================
//
//=============================================================================
uint8_t i2c_main_stop(void)
{
    uint8_t status;
    status = STATUS_OK ;    

    if( I2C_SSPIF == 1 ){
        // 正常終了
        SetLogDataM( LOG_DISP_I2C, "STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_END;

    }
    else if( Get_Timer(index) == 0 ){
        SetLogDataM( LOG_DISP_I2C, "STOP_RESS TOUT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        status = STATUS_FAIL;  
        i2c_data.status = I2C_STAT_STOP_RESS_TOUT;
    }

    return  status;
}
#endif

#ifdef ___NOP

//=============================================================================
//
//=============================================================================
int32_t i2c_readMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t *dt;
    uint8_t status;
    int index;
    
    
    status = STATUS_OK ;    

    printf("\r\ni2c_writeMulti() Add=0x%x,Indx=0x%x\r\n",address,reg);

    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // I2C START
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        I2C_SSPEN = 1;
        while( i2c_CheckIdle() != true );

        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // データ設定
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        i2c_data.address = address|0x01;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.restart_cnt = 0;
        i2c_data.op_type = I2C_READ_MULTI;


        i2c_data.status = I2C_STAT_START;
        // 割込みフラグクリア
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        // 割込みイネーブル
        I2C_SSPIE = 0;
        I2C_BCLIE = 1; 

        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // スタートコンディション
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        while( status == STATUS_OK ){
                    
            switch(i2c_data.status ){
            case I2C_STAT_START:
                status = i2c_main_start();
                if( i2c_data.status == I2C_STAT_START_REQ ){
                    index = Set_Timer(100);
                }
                break;
            case I2C_STAT_START_REQ:
                status = i2c_main_start_resp();
                if( i2c_data.status === I2C_STAT_ADDRESS_TX || i2c_data.status == I2C_STAT_START_TOUT ){
                    Rel_Timer(index);
                }
                break;
            case I2C_STAT_ADDRESS_TX_RETRY:
                    SetLogDataM( LOG_DISP_I2C, "ADDRESS_TX_RETRY",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                    Rel_Timer(index);
                    i2c_data.status = I2C_STAT_ADDRESS_TX;
                break;

            case I2C_STAT_ADDRESS_TX:
                status = i2c_main_address_tx();
                if( i2c_data.status = I2C_STAT_ADDRESS_RESPONS ){
                    index = Set_Timer(100);
                }
                break;
                
            case I2C_STAT_ADDRESS_RESPONS:
                status = i2c_main_address_resp();
                switch(i2c_data.status){
                    case I2C_STAT_ADDRESS_NACK:
                    case I2C_STAT_INDEX_TX:
                    case I2C_STAT_ADDRESS_TOUT:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;
                
            case I2C_STAT_INDEX_TX:
                status = i2c_main_index_tx();
                index = Set_Timer(100);
                break;

            case I2C_STAT_INDEX_RESPONS:
                status = i2c_main_index_resp();
                switch(i2c_data.status){
                    case I2C_STAT_INDEX_RESS_TOUT:
                    case I2C_STAT_DATA_TX:
                    case I2C_STAT_INDEX_NACK:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;
            case I2C_STAT_DATA_TX:
                status = i2c_main_data_tx();
                if(i2c_data.status == I2C_STAT_DATA_RESPONS){
                    index = Set_Timer(100);
                }
                break;

            case I2C_STAT_DATA_RESPONS:
                status =  i2c_main_data_res();
                switch(i2c_data.status){
                    case I2C_STAT_DATA_RESS_TOUT:
                    case I2C_STAT_DATA_TX:
                    case I2C_STAT_DATA_NACK:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;

            case I2C_STAT_STOP_REQ:
                status =  i2c_main_stop_req();
                index = Set_Timer(100);
                break;

            case I2C_STAT_STOP:
                status = i2c_main_stop();
                switch(i2c_data.status){
                    case I2C_STAT_STOP_RESS_TOUT:
                    case I2C_STAT_END:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           

                break;
            default:
                printf("Status= %d\r\n",i2c_data.status);
                SetLogDataM( LOG_DISP_I2C, "Error default",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.status = I2C_STAT_UNKWON_ERROR;

                    
                break;
            }
            
            
            if( i2c_data.status == I2C_STAT_END ){
                // 正常終了
                break;
            }
        }

    }        
}


//=============================================================================
//
//=============================================================================
int32_t i2c_writeMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t *dt;
    uint8_t status;
    int index;
    
    
    status = STATUS_OK ;    

    printf("\r\ni2c_writeMulti() Add=0x%x,Indx=0x%x\r\n",address,reg);

    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // I2C START
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        I2C_SSPEN = 1;
        

        while( i2c_CheckIdle() != true );

        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // データ設定
        //++++++++++++++++++++++++++++++++++++++++++++++++++

        i2c_data.address = address;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.restart_cnt = 0;
        i2c_data.op_type = I2C_WRITE_MULTI;

        for( i=0; i<count; i++ ){
            i2c_data.dt[i] = *dst;
            printf("dt=0x%x,",*dst);
            dst ++;
        }
            printf("\r\n");

        i2c_data.status = I2C_STAT_START;
        // 割込みフラグクリア
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        // 割込みイネーブル
        I2C_SSPIE = 0;
        I2C_BCLIE = 1; 
        
        
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        // スタートコンディション
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        while( status == STATUS_OK ){
                    
            switch(i2c_data.status ){
            case I2C_STAT_START:
                status = i2c_main_start();
                if( i2c_data.status == I2C_STAT_START_REQ ){
                    index = Set_Timer(100);
                }
                break;
            case I2C_STAT_START_REQ:
                status = i2c_main_start_resp();
                if( i2c_data.status === I2C_STAT_ADDRESS_TX || i2c_data.status == I2C_STAT_START_TOUT ){
                    Rel_Timer(index);
                }
                break;
            case I2C_STAT_ADDRESS_TX_RETRY:
                    SetLogDataM( LOG_DISP_I2C, "ADDRESS_TX_RETRY",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                    Rel_Timer(index);
                    i2c_data.status = I2C_STAT_ADDRESS_TX;
                break;

            case I2C_STAT_ADDRESS_TX:
                status = i2c_main_address_tx();
                if( i2c_data.status = I2C_STAT_ADDRESS_RESPONS ){
                    index = Set_Timer(100);
                }
                break;
                
            case I2C_STAT_ADDRESS_RESPONS:
                status = i2c_main_address_resp();
                switch(i2c_data.status){
                    case I2C_STAT_ADDRESS_NACK:
                    case I2C_STAT_INDEX_TX:
                    case I2C_STAT_ADDRESS_TOUT:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;
                
            case I2C_STAT_INDEX_TX:
                status = i2c_main_index_tx();
                index = Set_Timer(100);
                break;

            case I2C_STAT_INDEX_RESPONS:
                status = i2c_main_index_resp();
                switch(i2c_data.status){
                    case I2C_STAT_INDEX_RESS_TOUT:
                    case I2C_STAT_DATA_TX:
                    case I2C_STAT_INDEX_NACK:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;
            case I2C_STAT_DATA_TX:
                status = i2c_main_data_tx();
                if(i2c_data.status == I2C_STAT_DATA_RESPONS){
                    index = Set_Timer(100);
                }
                break;

            case I2C_STAT_DATA_RESPONS:
                status =  i2c_main_data_res();
                switch(i2c_data.status){
                    case I2C_STAT_DATA_RESS_TOUT:
                    case I2C_STAT_DATA_TX:
                    case I2C_STAT_DATA_NACK:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           
                break;

            case I2C_STAT_STOP_REQ:
                status =  i2c_main_stop_req();
                index = Set_Timer(100);
                break;

            case I2C_STAT_STOP:
                status = i2c_main_stop();
                switch(i2c_data.status){
                    case I2C_STAT_STOP_RESS_TOUT:
                    case I2C_STAT_END:
                        Rel_Timer(index);
                        break;
                    default:
                        break;
                }                           

                break;
            default:
                printf("Status= %d\r\n",i2c_data.status);
                SetLogDataM( LOG_DISP_I2C, "Error default",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
                i2c_data.status = I2C_STAT_UNKWON_ERROR;

                    
                break;
            }
            
            
            if( i2c_data.status == I2C_STAT_END ){
                // 正常終了
                break;
            }
        }
    }
}
#endif 

#endif  // __I2C_INTERRUPT

#ifdef  __I2C_INTERRUPT

//=============================================================================
//
//=============================================================================
int32_t i2c_writeMulti(uint8_t address,uint8_t reg, uint8_t *dst, uint8_t count)
{
    uint8_t i;
    uint8_t *dt;
    uint8_t status;
    int index;

    
    status = STATUS_OK ;    
    printf("\r\ni2c_writeMulti() Add=0x%x,Indx=0x%x\r\n",address,reg);

    if( count > MAX_MULTI_CNT ){
        printf("Error I2CData Size Over");
        status = STATUS_FAIL;          
    }
    else{
        I2C_SSPEN = 1;
        
        SetLogData( LOG_DISP_I2C, "ISR START",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);

        while( i2c_CheckIdle() != true );

        i2c_data.address = address;
        i2c_data.index = reg;
        i2c_data.count = count;
        i2c_data.count_step = 0;
        i2c_data.restart_cnt = 0;
        i2c_data.op_type = I2C_WRITE_MULTI;

        for( i=0; i<count; i++ ){
            i2c_data.dt[i] = *dst;
            printf("dt=0x%x,",*dst);
            dst ++;
        }
            printf("\r\n");

        i2c_data.status = I2C_STAT_START;
        
        // 割込みフラグクリア
        I2C_SSPIF = 0;
        I2C_BCLIF = 0;

        // 割込みイネーブル
        I2C_SSPIE = 0;
        I2C_BCLIE = 1; 
        
        //LATBbits.LATB2 = 0;     // for debug
        printf("write_multi() 0001\r\n");

       
        // I2C START
        I2C_SEN = 1;
        
        while( I2C_SSPIF == 1 );
        printf("write_multi() 0002\r\n");
        I2C_SSPIF = 0;
        I2C_SSPIE = 1;

        i2c_int_start_condition();
        
        
        printf("write_multi() 0003\r\n");
        
        


        index = Set_Timer(100);
        while( i2c_data.status != I2C_STAT_END ){
            if( i2c_data.status == I2C_STAT_ADDRESS_NACK )
            {
                printf("Error ADDRESS_NACK\r\n");
                status = STATUS_FAIL;          
                break;
            }
            if( i2c_data.status == I2C_STAT_INDEX_NACK )
            {
                printf("Error INDEX_NACK\r\n");
                status = STATUS_FAIL;          
                break;
            }
            if( i2c_data.status == I2C_STAT_DATA_NACK )
            {
                printf("Error DATA_NACK\r\n");
                status = STATUS_FAIL;          
                break;
            }
            if( i2c_data.status == I2C_COL_ERROR )
            {
                printf("Error COLLISION \r\n");
                status = STATUS_FAIL;          
                break;
            }
            if( Get_Timer(index) == 0 ){
                printf("Error:  Time out\r\n");
                status = STATUS_FAIL;          
                break;
            }
        }
        Rel_Timer(index);
        //printf("write_multi() 0002\r\n");
    
        if(status = STATUS_FAIL){
            SetLogData( LOG_DISP_I2C, "ERROR _writeMulti()",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
            // 割込みイネーブル
            I2C_SSPIE = 0;
            I2C_BCLIE = 0; 
            I2C_SSPEN = 0;
 //               I2C_PEN = 1;
 //               while(I2C_SSPIF == 0 )
        }
    }
    

    return status;
}
#endif  // __I2C_INTERRUPT



//=============================================================================
//
//=============================================================================
void interrupt_i2c_bcl(void)
{
    
        //print_string("int_bcl()\r\n");

        SetLogData( LOG_DISP_I2C, "ISR COLLISION ERR",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        //i2c_data.status = I2C_COL_ERROR;
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;
        bcl_error_chk = 1;
    
#ifdef ___NOP        
    
    if( i2c_data.status == I2C_STAT_ADDRESS_RESPONS ){
        // 再送
        SetLogData( LOG_DISP_I2C, "ISR COLLISION_RETRY",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_ADDRESS_TX_RETRY;
    }
    else{
        SetLogData( LOG_DISP_I2C, "ISR COLLISION ERR",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        //i2c_data.status = I2C_COL_ERROR;
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;
    }
#endif
}


//=============================================================================
//
//=============================================================================
void interrupt_i2c_ssp(void)
{
#ifdef ___NOP
    if( i2c_data.op_type == I2C_WRITE_MULTI ){
        i2c_interrupt_write();
    }
    else{
        i2c_interrupt_read();
    }
#endif
}

//=============================================================================
//
//=============================================================================
#ifdef  __I2C_INTERRUPT

void i2c_int_start_condition(void)
{
    SetLogData( LOG_DISP_I2C, "ISR START RES",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.address);
    i2c_data.status = I2C_STAT_ADDRESS_RESPONS;
    //LATBbits.LATB2 = 1;     // for debug
    I2C_SSPBUF = i2c_data.address;
}
//=============================================================================
//
//=============================================================================
void i2c_int_address_tx(void)
{
    //LATBbits.LATB2 = 0;     // for debug
    if( I2C_ACKSTAT ){
        SetLogData( LOG_DISP_I2C, "ISR ADD NACKT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.index);
        i2c_data.status = I2C_STAT_ADDRESS_NACK;
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;

#ifdef ___NOP            
            if( i2c_data.restart_cnt < RESTART_CNT_MAX){
                // NACKのためリスタートする
                SetLogData( LOG_DISP_I2C, "ISR ADD NACKT RESTART",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.index);
                i2c_data.status = I2C_STAT_RESTART;
                i2c_data.restart_cnt ++;
                //while(I2C_SSPSTAT_RW );
                I2C_RSEN = 1;
            }
            else{
                SetLogData( LOG_DISP_I2C, "ISR ADD NACKT",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.index);
                i2c_data.status = I2C_STAT_ADDRESS_NACK;
                I2C_SSPIE = 0;
                I2C_BCLIE = 0;
            }
#endif  // ___NOP            
    }
    else {
        SetLogData( LOG_DISP_I2C, "ISR ADD RES ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.index);
        i2c_data.status = I2C_STAT_INDEX_RESPONS;
        I2C_SSPBUF = i2c_data.index;
    }
}
//=============================================================================
//
//=============================================================================
void i2c_int_index_tx(void)
{
    SetLogData( LOG_DISP_I2C, "ISR INDEX",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2, i2c_data.dt[i2c_data.count_step]);
    if( I2C_ACKSTAT )
    {
        SetLogData( LOG_DISP_I2C, "ISR INDEX_NACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2, i2c_data.dt[i2c_data.count_step]);
        i2c_data.status = I2C_STAT_INDEX_NACK;
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;
    }
    else if( i2c_data.count_step < i2c_data.count ){
        i2c_data.status = I2C_STAT_DATA_RESPONS;
        I2C_SSPBUF = i2c_data.dt[i2c_data.count_step];
        I2C_SSPIF = 0;
        i2c_data.count_step ++;
    }
    else
    {
        SetLogData( LOG_DISP_I2C, "ISR STOP REQ",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_STOP;
        I2C_PEN = 1;
        I2C_SSPIF = 0;
    }
}
//=============================================================================
//
//=============================================================================
void i2c_int_data_tx(void)
{
    if( I2C_ACKSTAT ) {
        SetLogData( LOG_DISP_I2C, "ISR DATA NACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.dt[i2c_data.count_step]);
        i2c_data.status = I2C_STAT_DATA_NACK;
        I2C_SSPIE = 0;
        I2C_BCLIE = 0;
    }
    else if( i2c_data.count_step < i2c_data.count ){
        SetLogData( LOG_DISP_I2C, "ISR DATA ACK",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,i2c_data.dt[i2c_data.count_step]);
        i2c_data.status = I2C_STAT_DATA_RESPONS;
        I2C_SSPBUF = i2c_data.dt[i2c_data.count_step];
        I2C_SSPIF = 0;
        i2c_data.count_step ++;
    }
    else {
        SetLogData( LOG_DISP_I2C, "ISR STOP REQ2",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
        i2c_data.status = I2C_STAT_STOP;
        I2C_PEN = 1;
        I2C_SSPIF = 0;
    }
}
//=============================================================================
//
//=============================================================================
void i2c_int_stop_condition(void)
{
    SetLogData( LOG_DISP_I2C, "ISR STOP",I2C_SSPSTAT,I2C_SSPCON1,I2C_SSPCON2,0);
    i2c_data.status = I2C_STAT_END;
    I2C_SSPIE = 0;
    I2C_BCLIE = 0;
}

#endif  //  __I2C_INTERRUPT


//=============================================================================
//
//=============================================================================
void i2c_interrupt_write(void)
{
#ifndef  __I2C_INTERRUPT
    //print_string("i2c_int()\r\n");
#endif

#ifdef  __I2C_INTERRUPT
    
    
    switch(i2c_data.status){
    case I2C_STAT_START:
        i2c_int_start_condition();
        break;
     case I2C_STAT_ADDRESS_RESPONS:
        i2c_int_address_tx();
        break;
     case I2C_STAT_INDEX_RESPONS:
        i2c_int_index_tx();
        break;
     case I2C_STAT_DATA_RESPONS:
        i2c_int_data_tx();
        break;
     case I2C_STAT_STOP:
        i2c_int_stop_condition();
        break;
    default:
        SetLogData(LOG_DISP_I2C_STAT, "ERROR status",i2c_data.status,0,0,0);
        break;
         
    }
    
    // LATBbits.LATB2 = 1;     // for debug
#endif

}         

#ifdef ___NOP
//=============================================================================
//
//=============================================================================
void i2c_interrupt_read(void)
{
    switch(i2c_data.status){
    case I2C_STAT_START:
        i2c_data.status = I2C_STAT_ADDRESS_RESPONS;
        I2C_SSPBUF = i2c_data.address;
        break;
     case I2C_STAT_ADDRESS_RESPONS:
        if( I2C_ACKSTAT )
        {
            i2c_data.status = I2C_STAT_ADDRESS_NACK;
            I2C_SSPIE = 0;
        }
        else
        {
            i2c_data.status = I2C_STAT_INDEX_RESPONS;
            I2C_SSPBUF = i2c_data.index;
        }
        break;
    case I2C_STAT_INDEX_RESPONS:
        if( I2C_ACKSTAT )
        {
            i2c_data.status = I2C_STAT_INDEX_NACK;
            I2C_SSPIE = 0;
        }
        else
        {
            i2c_data.status = I2C_STAT_STOP_SUSPEND;
            I2C_PEN = 1;
        }
        break;
    case I2C_STAT_STOP_SUSPEND:
        i2c_data.status = I2C_STAT_RESTART;
        I2C_SEN = 1;
        break;
    case I2C_STAT_RESTART:
        i2c_data.status = I2C_STAT_RCV_ADDRESS_RESPONS;
        I2C_SSPBUF = i2c_data.address | 0x01;
         break;
    case I2C_STAT_RCV_ADDRESS_RESPONS:
        if( I2C_ACKSTAT )
        {
            i2c_data.status = I2C_STAT_ADDRESS_NACK;
            I2C_SSPIE = 0;
        }
        else
        {
            i2c_data.status = I2C_STAT_RCV_DATA;
            I2C_RCEN   = 1;
        }
         break;

    case I2C_STAT_RCV_DATA:
        i2c_data.dt[i2c_data.count_step] = I2C_SSPBUF;

        if( i2c_data.count_step < i2c_data.count ){
            if( i2c_data.count_step < (i2c_data.count-1) ){
                I2C_ACKDT =I2C_SEND_ACK;
                I2C_ACKEN = 1 ;          // ACKデータを返す
                i2c_data.status = I2C_STAT_RCV_DATA;
                I2C_RCEN   = 1;
            }
            else{
                I2C_ACKDT =I2C_SEND_NACK;
                I2C_ACKEN = 1 ;          // NACKデータを返す
                i2c_data.status = I2C_STAT_STOP;
                I2C_PEN = 1;
            }
            i2c_data.count_step ++;
        }
         break;

    case I2C_STAT_STOP:
        i2c_data.status = I2C_STAT_END;
        I2C_SSPIE = 0;
        break;
    }
}         
         
         
#endif  // ___NOP


/*******************************************************************************
 End of File
*/
