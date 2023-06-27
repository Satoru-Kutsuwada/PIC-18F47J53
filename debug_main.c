#include "main.h"				
#include "system.h"				
#include <xc.h>		

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "debug_main.h"
#include "rtc_main.h"

char CharConv[16]={
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};


void RegiDisp(void);
void uart_start_dispg(void);
void debug_main(void);
void rtc_disp_log(uint8_t flg);
void uint2string(char *buf, uint16_t data, uint8_t flg,uint8_t keta);
uint8_t get_log_onoff(void);


//extern void putrsUSBUSART(const char *data);
extern void putsUSBUSART(char *data);
extern void CDCTxService(void);
extern void Wait2Connect(void);
extern void Wait2TxRedy(void);
extern void CDCTxService22(void);
extern void Test_Set_timer(void);
extern void RTCdisp(void);
extern void vl53l0x_test(void);
extern void VL53_init(void);
extern char IntgetChar(void);
extern void i2c_init(uint8_t sel);
extern uint8_t get_rtc_usec(void);            
extern uint8_t get_rtc_sec(void);
extern uint16_t get_rtc_msec(void);        
extern void vl53l0x_Single_test(void);
extern void vl53l0x_SingleHA_test(void);
extern void vl53l0x_SingleHS_test(void);
extern void vl53l0x_SingleLR_test(void);
extern void putch485(char c);
extern void print485(char *string);



void DispLogData(void);
void SetLogData(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4);
void SetSubStringData( char *cpt, uint16_t *dpt );

//=============================================================================
// 変数宣言
//=============================================================================
uint16_t i2c_sens_cnt = 0;
uint8_t log_sem = 0;
uint8_t log_onoff = 0;
uint8_t log_stop = 0;

INPUT_CHAR_STEP input_char_step = INPUT_INIT;

typedef struct{
char main[INPUT_BUF_SIZE];
char sub1[INPUT_BUF_SIZE/2];
char sub2[INPUT_BUF_SIZE/2];
uint16_t data1;
uint16_t data2;
}INPUT_STRING;

INPUT_STRING input_string;


int input_pos;
 typedef enum{
    DL_RTC,
    DL_REG,
    DL_I2C,        
    DL_VL53,        
    DL_PB0,        
    DL_LOG,        
    DL_LOGOFF,        
    DL_LOGON,        
    DL_LOGCLEAR,        
            
            
    DL_ENDPT 
 }COMMAND_MENUE;
 
#define MAX_COM_LENGTH 10
 typedef struct{
     COMMAND_MENUE pt;
     char command[MAX_COM_LENGTH];
 } COMAND_LIST;
 
 const COMAND_LIST com_list[] = {
     { DL_RTC,      "rtc"  },
     { DL_REG,      "reg" },
     { DL_I2C,      "i2c" },
     { DL_VL53,     "vl53" },
     { DL_PB0,      "pb2" },
     { DL_LOG,      "log" },
     { DL_LOGOFF,   "logoff" },
     { DL_LOGON,    "logon" },
     { DL_LOGCLEAR, "logclr" },
     
     
     
     { DL_ENDPT, "none" }
 };
 
#define CHAR_1_RECORD_MAX 20
#define LOG_SIZE 20
 
 typedef struct{
     uint8_t type;
     uint8_t sec;
     uint16_t msec;
     uint8_t usec;
     char string[CHAR_1_RECORD_MAX];
     uint8_t reg[4];
 }LOG_RECORD;
 
 typedef struct{
     LOG_RECORD log_data[LOG_SIZE];
     uint16_t wpt;
     uint16_t rpt;
 }LOG_DATA;
 
 LOG_DATA log_dt;
 
                     
typedef enum{
    DEB_PROMPT_MODE = 0,
    DEB_VL53_MENUE,
            
    DEB_DISP_MAX
} DEB_MENUE_TYPE;

DEB_MENUE_TYPE dev_menue_type = DEB_PROMPT_MODE;

  // VL53 MENUE
 #define MAX_LENGTH 30
typedef struct{
    char    name[MAX_LENGTH];
} MENUE;

const MENUE Deb_menue00[] = {
     "\r\nVL53 MENUE",
     " 1.VL53_INIT",
     " 2.CONTIIOUS RANGING",
     " 3.SINGLE RANGING",
     " 4.SINGLE RANGING HA",
     " 5.SINGLE RANGING HS",
     " 6.SINGLE RANGING LR",

     " r.EXIT"
 };   
const MENUE Deb_menue01[] = {
     "\r\nXXXX MENUE",
     " 1.VL53_INIT",
     " 2.CONTIIOUS RANGING",
     " 3.SINGLE RANGING",
     " 4.SINGLE RANGING HA",
     " 5.SINGLE RANGING HS",
     " 6.SINGLE RANGING LR",

     " r.EXIT"
 };   
 
typedef struct
{
    MENUE *pt;
    uint8_t clumn;
} MENUE_NUM_PAGE;

 const MENUE_NUM_PAGE MenueList[]={
    Deb_menue00, (uint8_t)(sizeof(Deb_menue00 )/sizeof(MENUE)),
    Deb_menue01, (uint8_t)(sizeof(Deb_menue01 )/sizeof(MENUE))
};
 
 
 
//extern  RTC_DATA RTCdt;


//-----------------------------------------------------------------------------
//4
//-----------------------------------------------------------------------------
void log_init(void)
{
    log_dt.wpt =0;     
    log_dt.rpt =0;     
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------


 void SetLogDataM(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4)
 {
     uint16_t i;

   if( get_log_onoff() == 2 ) {
        printf("I=%d,S=%02x,C1=%02x,C2=%02x,BF=%02x  %s\r\n",type,dt1,dt2,dt3,dt4, string);
   }
    SetLogData(type, string, dt1, dt2, dt3,dt4);
    
 }
 
 void set_log_stop(uint8_t flg)
 {
     if(flg != 0)
        SetLogData(LOG_DISP_NO_DATA ,"LOG STOP", 0,0,0,0);

        log_stop = flg;
     
 }
 
 void SetLogData(uint8_t type, char *string, uint8_t dt1, uint8_t dt2, uint8_t dt3,uint8_t dt4)
 {
     uint16_t i;

     if( log_stop != 0 )
         return;

    while(log_sem != 0);
    log_sem = 1;

     //while(RTCdt.usec==10);

     log_dt.log_data[log_dt.wpt].sec  = get_rtc_sec();
     log_dt.log_data[log_dt.wpt].msec = get_rtc_msec();
     log_dt.log_data[log_dt.wpt].usec = get_rtc_usec(); 
                

            

     log_dt.log_data[log_dt.wpt].reg[0] = dt1;
     log_dt.log_data[log_dt.wpt].reg[1] = dt2;
     log_dt.log_data[log_dt.wpt].reg[2] = dt3;
     log_dt.log_data[log_dt.wpt].reg[3] = dt4;
     log_dt.log_data[log_dt.wpt].type = type;
     
     
     
     i=0;
     while( *string != '\0' ){
        log_dt.log_data[log_dt.wpt].string[i] = *string;
        if( i > CHAR_1_RECORD_MAX ){
           log_dt.log_data[log_dt.wpt].string[i] = '\0';
           break;
        }
        i ++;
        string ++;
    } 
    log_dt.log_data[log_dt.wpt].string[i] = '\0';
     
    log_dt.wpt ++;
    if( log_dt.wpt > LOG_SIZE ){
        log_dt.wpt = 0;
    }
    
    if( log_dt.wpt == log_dt.rpt ){
        log_dt.rpt ++;
        if( log_dt.rpt > LOG_SIZE ){
            log_dt.rpt = 0;
        }
    }

    log_sem = 0;

 }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

 void DispLogData(void)
 {
     uint16_t i;
     uint16_t rpt;

    printf("\r\n");

    printf("WPT=%d,RPT=%d\r\n",log_dt.wpt,log_dt.rpt);
    
    rpt = log_dt.rpt;
    for(i=0; i<  LOG_SIZE; i++ ){
     
       
        if( log_dt.log_data[rpt].usec == 10 )
            log_dt.log_data[rpt].usec = 0;

        // 時刻
        printf("%02d.%03d.%d ",  log_dt.log_data[rpt].sec,   log_dt.log_data[rpt].msec,  log_dt.log_data[rpt].usec );
        
        
        switch(log_dt.log_data[rpt].type ){
            case LOG_DISP_I2C:
                printf("S=%02x,C1=%02x,C2=%02x,BF=%02x ", 
                        log_dt.log_data[rpt].reg[0],
                        log_dt.log_data[rpt].reg[1],
                        log_dt.log_data[rpt].reg[2],
                        log_dt.log_data[rpt].reg[3]);
                break;
            case LOG_DISP_I2C_STAT:
                 printf("S=%d ", log_dt.log_data[rpt].reg[0]);
                break;
            case LOG_DISP_GET_TM:
                printf("C=%d,index=%d,flg=%d ",log_dt.log_data[rpt].reg[0],log_dt.log_data[rpt].reg[1],log_dt.log_data[rpt].reg[2]);
                break;
            case LOG_DISP_SET_TM:
                printf("D=%d,index=%d ",log_dt.log_data[rpt].reg[0],log_dt.log_data[rpt].reg[1]);
                break;
            case LOG_DISP_REL_TM:
                printf("index=%d ",log_dt.log_data[rpt].reg[0]);
                break;
            case LOG_DISP_NO_DATA:
            default:
                break;
        }

        printf("%s\r\n", &log_dt.log_data[rpt].string[0]);
                

        rpt ++;
        if( rpt > LOG_SIZE ){
            rpt = 0;
        }
        if( rpt == log_dt.wpt ){
            break;
        }
    }
     
 }
 
 
 
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
#ifdef ___NOP
void uint2string(char *buf, uint16_t data, uint8_t flg,uint8_t keta)
{
    uint16_t dt;
    uint16_t i;
    uint16_t sw;
    uint16_t kdt = 1;
    
    sw = 0;
    
    
    dt = data;
    if( flg == 10){
        if( keta == 0 ){
            kdt = 10000;
        }
        else if( keta > 5 ){
                kdt = 10000;
        }
        else {
            for( i=1; i< keta; i++ ){
                kdt = kdt * 10;
            }
        }
        
        for(i=10000; i>0; i/=10){
            if( i == kdt )
                sw = 1;
                  
            dt = dt/i;
            if(sw==0){
                if(dt != 0){
                    *buf = CharConv[dt];
                    sw=1;
                }
            }
            else{
                    *buf = CharConv[dt];            
            }
            dt = data - dt*i;
            buf ++;
        }
    }
    else{
        *buf = '0';
        buf ++;
        *buf = 'x';
        buf ++;

        dt = (data >> 12)& 0x0f;
        *buf = CharConv[dt];
        buf ++;
        
        dt = (data >> 8)& 0x0f;
        *buf = CharConv[dt];
        buf ++;
        
        dt = (data >> 4)& 0x0f;
        *buf = CharConv[dt];
        buf ++;
        
        dt = data & 0x0f;
        *buf = CharConv[dt];
        buf ++;
                
    }
    
    *buf =(char) '\0';
    
    
}


 //-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------


void print_string(char *string)
{
    while( *string != '\0' ){
        putch(*string);
        string ++;
    }
}

#endif


#ifdef ___NOP 

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void print_uint16(char *string,uint16_t dt1,uint16_t dt2,uint16_t dt3)
{
    char buf[10];
    uint8_t cnt = 0;
    uint8_t i;
    uint16_t data[3];

    data[0] = dt1;
    data[1] = dt2;
    data[2] = dt3;
    
    while( *string != '\0' ){
        if(( *string == '%' ) && ( cnt < 3 )){
            string ++;
            if( *string == 'd' ){
                uint2string(buf, data[cnt], 0);
                cnt ++;
                for( i=0; 1 < 10; i++ ){
                    if( buf[i] != '\0' ){
                        putch('buf[i]');
                    }
                }
            }
            else if( *string == 'x' ){
                uint2string(buf, data[cnt], 1);
                cnt ++;
                for( i=0; 1 < 10; i++ ){
                    if( buf[i] != '\0' ){
                        putch('buf[i]');
                    }
                }
            }
            else{
                putch('%');
            }
        }

        putch(*string);
        string ++;
    }    
}


#endif
//==============================================================================
//
//==============================================================================
        
 COMMAND_MENUE input2menu(void)
 {
     uint16_t i = 0;
    COMMAND_MENUE rtn = DL_ENDPT;
             
        while( com_list[i].pt != DL_ENDPT ){
            //printf("input=%s,%s \r\n",input_string,&com_list[i].command[0]);
            if (strcmp(&input_string.main[0], &com_list[i].command[0]) == 0){
                
                
                
                rtn = com_list[i].pt;
                break;
            }
            i++;
        }
        
    return rtn;
}

//==============================================================================
//
//==============================================================================
#ifdef _____NOP

void USB_start_dispg(void)
{
    
    Wait2Connect();
    
    Wait2TxRedy();

    
    printf("\r\n");
    printf("\r\n");
    printf("*******************\r\n");
    putsUSBUSART("\r\n*** USB-CDC Setup ***\r\n");

    printf("*******************\r\n");
    printf("\r\n");

    
    //CDCTxService();
    CDCTxService22();
                    
}
#endif

//==============================================================================
//
//==============================================================================
void uart_start_dispg(void)
{
#ifdef _____NOP
    
    printf("\r\n");
    printf("\r\n");
    printf("*******************\r\n");
#endif

    printf("*** USART Setup ***\r\n");

#ifdef _____NOP
    printf("*******************\r\n");
    printf("\r\n");
#endif

                    
}
//==============================================================================
//
//==============================================================================

void uart485_start_dispg(void)
{
#ifdef _____NOP
    
    printf("\r\n");
    printf("\r\n");
    printf("*******************\r\n");
#endif

    print485("*** USART Setup ***\r\n");

#ifdef _____NOP
    printf("*******************\r\n");
    printf("\r\n");
#endif

                    
}
//==============================================================================
//
//==============================================================================
void main_loop_disp(void)
{
    //printf("\r ***** MAIN LOOP START *****\n");


    printf("\r\n");
    printf("Initialize End\r\n\r\n");
    printf(" *     *    *     ***  *   * \r\n");
    printf(" * * * *   * *     *   **  * \r\n");
    printf(" *  *  *  *****    *   * * * \r\n");
    printf(" *     *  *    *   *   *  ** \r\n");
    printf(" *     * *      * ***  *   * \r\n\r\n");

    // Show prompt. 
    printf("PIC> ");

                    
}

//==============================================================================
//
//==============================================================================
void DispMenue(uint8_t type)
{
    uint8_t i;
    if( type==DEB_PROMPT_MODE ){
        printf("PIC> ");
    }
    else{
        for( i=0; i<MenueList[type-1].clumn; i++){
            printf("%s\r\n", &MenueList[type-1].pt[i].name[0]);
        }
    }
}
   

//==============================================================================
//
//==============================================================================


 INPUT_CHAR_STEP read_line_streem(void)
 
{
    char c;
    uint16_t i;
    uint16_t j;
    uint16_t keta;
    uint8_t num;
    uint8_t sub_cnt;
    


    c = (char)getch();
    //c = IntgetChar();
    
    if( c != 0 ){
        
        //printf("Get C = %x\r\n",c,0,0);

        switch(input_char_step){
            case INPUT_INIT:
                //printf("INPUT_INIT\r\n");
                input_pos = 0;
                input_char_step = INPUT_SAVING;
                
           
            case INPUT_SAVING:
                if( isprint(c) && (input_pos <= INPUT_BUF_SIZE - 2)){
                    //printf("INPUT_SAVING 01\r\n");
                    input_string.main[input_pos] = c;
                    input_pos ++;
                    putch(c);
                }
                else if (c == 0x08 && input_pos > 0) {      // Back Space
                    //printf("INPUT_SAVING 02\r\n");
                    input_pos --;
                    printf("\x08 \x08");
                } 
                else if (c == '\r') {
                    //printf("INPUT_SAVING 03\r\n");
                    input_string.main[input_pos] = '\0';
                    printf("\r\n");
                    input_char_step = INPUT_DATA_FIX;
              

                    //++++++++++++++++++++++++++++++++++++
                    // Sub String
                    //++++++++++++++++++++++++++++++++++++
                    sub_cnt = 0;
                    input_string.sub1[0] = '\0';
                    input_string.sub2[0] = '\0';
                    input_string.data1 = 0;
                    input_string.data2 = 0;
                    j=0;
                    for(i=0; i<INPUT_BUF_SIZE ; i++ ){
                         if( input_string.main[i] == '\0' ){
                            switch(sub_cnt){
                            case 0:
                                break;
                            case 1:
                                input_string.sub1[j] = '\0';
                                j++;
                                break;
                            case 2:
                                input_string.sub2[j] = '\0';
                                break;
                            }
                            break;
                         }
                         else if(input_string.main[i]==' '){
                            switch(sub_cnt){
                            case 0:
                                input_string.main[i] = '\0';
                                sub_cnt ++;
                                break;
                            case 1:
                                if( input_string.sub1[0] != '\0'){
                                    input_string.sub1[j] = '\0';
                                    sub_cnt ++;
                                    j=0;
                                }
                                break;
                            case 2:
                                if( input_string.sub2[0] != '\0'){
                                    input_string.sub2[j] = '\0';
                                    //sub_cnt ++;
                                }
                                break;
                            }
                         }
                         else{
                            switch(sub_cnt){
                            case 0:
                                break;
                            case 1:
                                input_string.sub1[j] = input_string.main[i] ;
                                j++;
                                break;
                            case 2:
                                input_string.sub2[j] = input_string.main[i] ;
                                j++;
                                break;
                            }
                         }
                    }
#ifdef __INPUT_STRING_LOG
    printf("main = %s\r\n", &input_string.main[0]);
    printf("sub1 = %s\r\n", &input_string.sub1[0]);
    printf("sub2 = %s\r\n", &input_string.sub2[0]);
#endif

                    SetSubStringData( &input_string.sub1[0],&input_string.data1);
                    SetSubStringData( &input_string.sub2[0],&input_string.data2);



                    

                }
            default:
                break;              
        }
    }
    //printf("read_line_streem\r\n");
    return(input_char_step);
 }

//==============================================================================
//
//==============================================================================
 
void SetSubStringData( char *cpt, uint16_t *dpt )
{
    uint8_t i;
    uint8_t  num = 1;
    uint16_t keta = 0;
    *dpt = 0;

#ifdef __INPUT_STRING_LOG
    printf("Input sub = %s\r\n",cpt);
#endif
    if(( cpt[0] == '0') && ( cpt[1] == 'x')){
        for(i=2; i<INPUT_BUF_SIZE/2; i++ ){
            if(cpt[i]=='\0'){
                break;
            }
            /* アルファベットか数字かを判定 */
            if((cpt[i] >='a' &&  cpt[i]<='f')||(cpt[i]>='A' && cpt[i]<='F')||(cpt[i]>='0'&& cpt[i]<='9')){
                if(keta == 0){
                    keta = 1;
                }
                else{
                    keta *= 16;
                }
            }
            else{
                num = 0;
                break;
            }
        }
        
#ifdef __INPUT_STRING_LOG
        printf("num=%d,keta=0x%x\r\n",num,keta);
#endif
        if( num == 1){
            for(i=2; i<INPUT_BUF_SIZE/2; i++ ){
                if(cpt[i]=='\0'){
                    break;
                }
                else{
                    if(cpt[i]>='0'&& cpt[i]<='9'){
                        *dpt += (cpt[i] & 0x0f)*keta;
                    }
                    else{
                        *dpt += ((cpt[i] & 0x0f)+9)*keta;
                    }
                    keta /= 16;
                }
            }
        }
#ifdef __INPUT_STRING_LOG
        printf("*dpt=0x%x\r\n",*dpt);
#endif
    }
    else{
        for(i=0; i<INPUT_BUF_SIZE/2; i++ ){
            if(cpt[i]=='\0'){
                break;
            }
            /* 数字かを判定 */
            if(cpt[i]>='0'&& cpt[i]<='9'){
                if(keta == 0){
                    keta = 1;
                }
                else{
                    keta *= 10;
                }
            }
            else{
                num = 0;
                break;
            }
        }

#ifdef __INPUT_STRING_LOG
        printf("num=%d,keta=%d\r\n",num,keta);
#endif
        if( num == 1){
            for(i=0; i<INPUT_BUF_SIZE/2; i++ ){
                if(cpt[i]=='\0'){
                    break;
                }
                else{
                    *dpt += (cpt[i] & 0x0f)*keta;
                    keta /= 10;
                }
            }
        }
#ifdef __INPUT_STRING_LOG
        printf("*dpt=%d\r\n",*dpt);
#endif
    }
}
 //==============================================================================
//
//==============================================================================
 void debug_main485(void)
 {
     char c;

     c = (char )getch485();
     if( c != 0 ){
         printf("RS485 C=0x%x\r\n",c);
         putch485(c);
     }

 }           

//==============================================================================
//
//==============================================================================
 void debug_main(void)
 {
     uint8_t i;
     
     if(read_line_streem() == INPUT_DATA_FIX){
            input_char_step = INPUT_INIT;
            
            
            if (strcmp(&input_string.main[0], "") != 0){

                if( dev_menue_type == DEB_VL53_MENUE ){
                    switch( input_string.main[0] ){
                        case '1':
                            VL53_init();
                            break;
                        case '2':
                            vl53l0x_test();
                            break;
                        case '3':
                            vl53l0x_Single_test();
                            break;
                        case '4':
                            vl53l0x_SingleHA_test();
                            break;
                        case '5':
                            vl53l0x_SingleHS_test();
                            break;
                        case '6':
                            vl53l0x_SingleLR_test();
                            break;
                        case '7':
                              break;
                        case '8':
                            break;
                        case '9':
                            break;
                        case 'r':
                        case 'R':
                            dev_menue_type = DEB_PROMPT_MODE;  
//                            DispMenue(dev_menue_type); 
                        
                        
                        default:
                            break;
                    }
                }
                else{
                    switch( input2menu() ){
                    case DL_RTC:
                        //RTCdisp();
                        rtc_disp_log(1);
                        break;
                    case DL_REG:
                        RegiDisp();
                        break;           
                    case DL_I2C:
                        // Clock 変更  i2c -c 100
                        if( input_string.sub1[0]=='-' && input_string.sub1[1]=='c' ){
                            if(input_string.data2 == 100){
                                printf("SCL 100KHz\r\n");
                                i2c_init(0);
                            }
                            else{
                                printf("SCL 400KHz\r\n");
                                i2c_init(1);
                            }
                        }
                        else{
                            if(input_string.data1 == 0){
                                i2c_sens_cnt = 1;
                            }
                            else{
                                i2c_sens_cnt = input_string.data1;
                            }
                        }
                        break;           
                    case DL_VL53:
                        
                        dev_menue_type = DEB_VL53_MENUE;
                         //DispMenue(dev_menue_type ); 
                        
                        break;
                    case DL_PB0:
                        if( LATBbits.LATB2 )
                           LATBbits.LATB2 = 0;
                        else
                            LATBbits.LATB2 = 1;
                        break;
                    case DL_LOG:
                        DispLogData();
                        break;
                    case DL_LOGOFF:
                        log_onoff = 0;
                        printf("LOG OFF = %d\r\n",log_onoff);
                        break;
                    case DL_LOGON:
                        log_onoff += 1;
                        printf("LOG ON = %d\r\n",log_onoff);
                        break;
                    case DL_LOGCLEAR:
                        printf("LOG CLEAR\r\n");
                        log_init();
                        set_log_stop(0);
                        break;
                    case DL_ENDPT:
                        printf("Command not found. \r\n");
                        break;
                    default:
                        break;
                    }
                }
            }
                         
            // Show prompt. 
            DispMenue(dev_menue_type);
     }
 }
uint8_t get_log_onoff(void)
{
    return log_onoff;
}
 
void RegiDisp(void)
{
    uint8_t *pt;
//================================================   
//ポート   
//================================================   

//================================================   
//I2Cモジュール初期化   
//================================================   
printf("\r\n");
printf("*** I2C ************\r\n");
printf(" SSP1STAT = 0x%02x\r\n",  SSP1STAT);
printf(" SSP1CON1 = 0x%02x\r\n",  SSP1CON1);
printf(" SSP1CON2 = 0x%02x\r\n",  SSP1CON2);
printf(" SSP1ADD  = 0x%02x\r\n\n",  SSP1ADD);
printf(" SSP2STAT = 0x%02x\r\n",  SSP2STAT);
printf(" SSP2CON1 = 0x%02x\r\n",  SSP2CON1);
printf(" SSP2CON2 = 0x%02x\r\n",  SSP2CON2);
printf(" SSP2ADD  = 0x%02x\r\n\n",  SSP2ADD);

printf("*** PORT ************\r\n");
printf(" TRISA  = 0x%02x\r\n",  TRISA);
printf(" TRISB  = 0x%02x\r\n",  TRISB);
printf(" TRISC  = 0x%02x\r\n",  TRISC);
printf(" TRISD  = 0x%02x\r\n",  TRISD);
printf(" TRISE  = 0x%02x\r\n\n",  TRISE);

printf("*** OSCILLATOR ************\r\n");
printf(" OSCCON  = 0x%02x\r\n",  OSCCON);
printf(" OSCCON2 = 0x%02x\r\n",  OSCCON2);
printf(" OSCTUNE = 0x%02x\r\n\n",  OSCTUNE);

printf("*** INTERRUPT ************\r\n");
printf(" RCON    = 0x%02x\r\n",  RCON );
printf(" INTCON  = 0x%02x\r\n",  INTCON );
printf(" INTCON2 = 0x%02x\r\n",  INTCON2 );
printf(" INTCON3 = 0x%02x\r\n",  INTCON3 );
printf(" PIR     = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",  PIR1,PIR2,PIR3,PIR4,PIR5 );
printf(" PIE     = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",  PIE1,PIE2,PIE3,PIE4,PIE5 );
printf(" IPR     = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",  IPR1,IPR2,IPR3,IPR4,IPR5 );

printf("\n*** UART1 ************\r\n");
printf(" TXSTA1    = 0x%02x\r\n",  TXSTA1 );
printf(" RCSTA1    = 0x%02x\r\n",  RCSTA1 );
printf(" BAUDCON1  = 0x%02x\r\n",  BAUDCON1 );
printf(" SPBRG1    = 0x%02x%02x\r\n",  SPBRGH1,SPBRG1 );
printf("\n*** UART2 ************\r\n");
printf(" TXSTA2    = 0x%02x\r\n",  TXSTA2 );
printf(" RCSTA3    = 0x%02x\r\n",  RCSTA2 );
printf(" BAUDCON2  = 0x%02x\r\n",  BAUDCON2 );
printf(" SPBRG2    = 0x%02x%02x\r\n",  SPBRGH2,SPBRG2 );




#ifdef ___NOP
printf("*** CONFIG ************\r\n");
pt = (uint8_t *)0x1FFF8;
printf(" CONFIG1L  = 0x%x\r\n",  *pt);
pt = (uint8_t *)0x1FFF9;
printf(" CONFIG1H  = 0x%x\r\n",  *pt);
pt = (uint8_t *)0x1FFFA;
printf(" CONFIG2L  = 0x%x\r\n",  *pt);
pt = (uint8_t *)0x1FFFB;
printf(" CONFIG2H  = 0x%x\r\n",  *pt);
#endif


#ifdef _____NOP
print_uint16(" ADCON1 = 0x%x\r\n",  ADCON1 ,0,0);
print_uint16(" TRISA  = 0x%x\r\n",  TRISA  ,0,0);
print_uint16(" TRISB  = 0x%x\r\n",  TRISB  ,0,0);
print_uint16(" TRISC  = 0x%x\r\n",  TRISC  ,0,0);
print_uint16(" TRISD  = 0x%x\r\n",  TRISD  ,0,0);
print_uint16(" TRISE  = 0x%x\r\n\n",  TRISE  ,0,0);
print_uint16(" LATA   = 0x%x\r\n",  LATA  ,0,0);
print_uint16(" LATB   = 0x%x\r\n",  LATB  ,0,0);
print_uint16(" LATC   = 0x%x\r\n",  LATC  ,0,0);
print_uint16(" LATD   = 0x%x\r\n",  LATD  ,0,0);
print_uint16(" LATE   = 0x%x\r\n\n",  LATE  ,0,0);
//================================================   
//USART   
//================================================   
print_uint16(" BRGH    = 0x%x\r\n",  BRGH  ,0,0);
print_uint16(" SPBRG   = 0x%x\r\n",  SPBRG  ,0,0);
print_uint16(" SYNC    = 0x%x\r\n",  SYNC  ,0,0);
print_uint16(" SPEN    = 0x%x\r\n",  SPEN  ,0,0);
print_uint16(" CREN    = 0x%x\r\n",  CREN  ,0,0);
print_uint16(" TXEN    = 0x%x\r\n\n",  TXEN  ,0,0);
//================================================   
//I2Cモジュール初期化   
//================================================   
print_uint16(" SSPSTAT = 0x%x\r\n",  SSPSTAT  ,0,0);
print_uint16(" SSPCON1 = 0x%x\r\n",  SSPCON1  ,0,0);
print_uint16(" SSPCON2 = 0x%x\r\n",  SSPCON2  ,0,0);
print_uint16(" SSPADD  = 0x%x\r\n\n",  SSPADD  ,0,0);
//================================================   
//OSCILLATOR_Initialize   
//================================================   
print_uint16(" OSCCON  = 0x%x\r\n",  OSCCON  ,0,0);
print_uint16(" OSCTUNE = 0x%x\r\n\n",  OSCTUNE  ,0,0);
print_uint16(" TMR1L   = 0x%x\r\n",  TMR1L  ,0,0);
print_uint16(" TMR1H   = 0x%x\r\n",  TMR1H  ,0,0);
print_uint16(" INTCON  = 0x%x\r\n",  INTCON  ,0,0);
print_uint16(" IPR1    = 0x%x\r\n",  IPR1  ,0,0);
print_uint16(" T1CON   = 0x%x\r\n",  T1CON ,0,0 );
print_uint16(" RCON    = 0x%x\r\n",  RCON ,0,0 );
print_uint16(" INTCON2 = 0x%x\r\n",  INTCON2 ,0,0 );
print_uint16(" INTCON3 = 0x%x\r\n",  INTCON3 ,0,0 );
print_uint16(" PIR1    = 0x%x\r\n",  PIR1 ,0,0 );
print_uint16(" PIR2    = 0x%x\r\n",  PIR2 ,0,0 );
print_uint16(" PIE1    = 0x%x\r\n",  PIE1 ,0,0 );
print_uint16(" PIE2    = 0x%x\r\n",  PIE2 ,0,0 );
print_uint16(" IPR2    = 0x%x\r\n",  IPR2 ,0,0 );
#endif  // _____NOP
}
 
 
 
 

 
