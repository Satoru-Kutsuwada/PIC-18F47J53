
/*
 * usr_rs485_main.c
 *
 *  Created on: 2023/07/31
 *      Author: nosak
 */


/* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include "usr_system.h"
//#include "rs485_com.h"
#include "main.h"				
#include "system.h"				
#include <xc.h>		

/* Public includes -----------------------------------------------------------*/

/* Public typedef ------------------------------------------------------------*/

typedef enum {
	RS485_AD_MASTER = 0,
	RS485_AD_SLEVE01,
	RS485_AD_SLEVE02,


	RS485_AD_MAX
}RA485_ADDRESS;

#define MY_RS485_ADDRESS RS485_AD_SLEVE01

#define SLV_VERSION     0x0110

typedef enum {
	RS485_CMD_STATUS = 1,
	RS485_CMD_VERSION,
	RS485_CMD_MESUR,
	RS485_CMD_MESUR_DATA,

	RS485_CMD_MAX
}RA485_COMMAND;

typedef enum {
	RET_FALSE = 0,
	RET_TRUE

}RETURN_STATUS;

typedef enum {
	SLV_STATUS_IDLE = 0,
	SLV_STATUS_MESURING,
	SLV_STATUS_ERROR,

}SLV_STATUS;

typedef enum {
	SENS_NON        = 0,
    SENS_VL530X     = 0x01,
    SENS_SONIX      = 0x02
            
}SENS_KIND;


/* Public define -------------------------------------------------------------*/

/* Public macro --------------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/


/* Public function prototypes ------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

#define RS485_TX	1
#define RS485_RX	0


/* Private typedef -----------------------------------------------------------*/
typedef struct{
	RA485_COMMAND		command;
	RA485_ADDRESS		address;
	uint8_t				sub1;
	uint8_t				rcv_byte;
} CMD_MSG;


/* Private define ------------------------------------------------------------*/
#define RS485_ADD_ID	0x00
#define RS485_CMD_ID	0x03

#define RS485_ADD_BYTE	3
#define RS485_SUM_BYTE	2


#define RS485_COM_01_BYTE	6
#define RS485_COM_02_BYTE	7
#define RS485_COM_03_BYTE	3
#define RS485_COM_04_BYTE	7



/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
typedef enum {
    COM_RCV_INIT = 0,
    COM_RCV_ADD_ID,
    COM_RCV_ADD_ID_DIST,
    COM_RCV_ADD_ID_SOURCE,
    COM_RCV_CMD_ID,
    COM_RCV_COMMAND,
    COM_RCV_CSUM_ID,
    COM_RCV_CSUM,
    COM_RCV_COMPLITE
            
}COM_STEP;

COM_STEP    com_step_flg;


typedef struct {

	uint8_t		resp_dt;
	uint8_t		status;
	uint8_t		status_err;
	uint8_t		sens_kind;

	uint16_t	slv_ver;
	uint16_t	sens_ver;
	float		sens_dt;

}SLAVE_DATA;

SLAVE_DATA slv_dt[RS485_AD_MAX];



#define  RCV_BUF_SIZE 128
uint8_t		rcvbuf[RCV_BUF_SIZE];
uint8_t		rcvnum = 0;
uint8_t		rcv_wpt = 0;
uint8_t		rcv_rpt = 0;





#define RS485_MSG_MAX	20

uint8_t		cmd_mesg[RS485_MSG_MAX];
uint8_t		Res_mesg[RS485_MSG_MAX];
uint8_t		cmd_char[RS485_MSG_MAX];
uint8_t		cmd_ptr = 0;

uint8_t     rcsta;
uint8_t     txsta;






const CMD_MSG	com[] = {
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_02_BYTE	 },
	{ RS485_CMD_VERSION, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_02_BYTE	 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE01, 	1,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_03_BYTE	 },
	{ RS485_CMD_MESUR, 		RS485_AD_SLEVE02, 	1,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_03_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_STATUS, 	RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_01_BYTE	 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE01, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_04_BYTE	 },
	{ RS485_CMD_MESUR_DATA, RS485_AD_SLEVE02, 	0,	RS485_ADD_BYTE+RS485_SUM_BYTE+RS485_COM_04_BYTE	 },
	{ RS485_CMD_MAX, 		0, 					0,	0 }
};



/* Private function prototypes -----------------------------------------------*/
uint8_t Get_rcv_data(void);


//==============================================================================
//
//==============================================================================

void Set_rcv_data(uint8_t dt)
{
    rcvbuf[rcv_wpt] = dt;
    rcvnum ++;
    rcv_wpt ++ ;
    if( rcv_wpt > RCV_BUF_SIZE ){
        rcv_wpt = 0;
    }
}

//==============================================================================
//
//==============================================================================

uint8_t Get_rcv_data(void)
{
    uint8_t dt;
    rcvnum --;
    dt =  rcvbuf[rcv_rpt];
    rcv_rpt ++ ;
    if( rcv_rpt > RCV_BUF_SIZE ){
        rcv_rpt = 0;
    }
    return dt;

}


//==============================================================================
//
//==============================================================================
COM_STEP command_rcv(void)
{
    uint8_t     dt;
    uint8_t     i = 0;
    uint8_t     sum = 0;
    
    if( rcsta != UART485_RCSTA){  
        rcsta = UART485_RCSTA;  
        printf("UART485_RCSTA=%02x\r\n",rcsta);
    }
    
    if( txsta != UART485_TXSTA){
        txsta = UART485_TXSTA;
        printf("UART485_TXSTA=%02x\r\n",txsta);
    }
    
    
//    if(UART485_RCIF){
//        dt = UART485_RCREG;
  
    if( rcvnum > 0 ){
        dt = Get_rcv_data();
        
        switch( com_step_flg ){
            case  COM_RCV_INIT:
                if( dt == '#' ){
                    cmd_mesg[cmd_ptr] = dt;
                    com_step_flg = COM_RCV_ADD_ID;
                    cmd_ptr ++;
                }
                break;
            case COM_RCV_ADD_ID:
                cmd_mesg[cmd_ptr] = dt;
                com_step_flg = COM_RCV_ADD_ID_DIST;
                cmd_ptr ++;
                break;

            case COM_RCV_ADD_ID_DIST:
                cmd_mesg[cmd_ptr] = dt;
                com_step_flg = COM_RCV_ADD_ID_SOURCE;
                cmd_ptr ++;
                break;
                
            case COM_RCV_ADD_ID_SOURCE:
                if( dt == '*' ){
                    cmd_mesg[cmd_ptr] = dt;
                    com_step_flg = COM_RCV_CMD_ID;
                    cmd_ptr ++;
                }
                else{
                    com_step_flg = COM_RCV_INIT;
                    cmd_ptr = 0;
                }
                break;

            case COM_RCV_CMD_ID:
                cmd_mesg[cmd_ptr] = dt;
                com_step_flg = COM_RCV_COMMAND;
                cmd_ptr ++;
                break;

            case COM_RCV_COMMAND:
                if( dt == '$' ){
                    cmd_mesg[cmd_ptr] = dt;
                    com_step_flg = COM_RCV_CSUM_ID;
                    cmd_ptr ++;
                }
                else{
                    cmd_mesg[cmd_ptr] = dt;
                    com_step_flg = COM_RCV_COMMAND;
                    cmd_ptr ++;
                }
                break;
            case COM_RCV_CSUM_ID:

                i = 0;
                sum = 0;
                while(cmd_mesg[i] != '$'){
                    sum += cmd_mesg[i];
                }
                
                if( sum == dt ){
                    cmd_mesg[cmd_ptr] = dt;
                    com_step_flg = COM_RCV_COMPLITE;
                    cmd_ptr ++;
                 
                    
                    printf("cmd_mesg= ");
                    for( i=0; i<cmd_ptr; i++ ){
                        printf("%02x ",cmd_mesg[i]);
                        cmd_char[i] = ((cmd_mesg[i]<0x20||cmd_mesg[i]>=0x7f)? '.': cmd_mesg[i]);
                    }
                    cmd_char[i+1] = '\0';
                    printf(" :: %s\r\n", cmd_char);
                    
                    
                }
                else{
                    com_step_flg = COM_RCV_INIT;
                    cmd_ptr = 0;
                }
                
            case COM_RCV_COMPLITE:
            default:
                break;
        }

        printf("dt=0x%02x, com_step_flg=%d\r\n", dt, com_step_flg);
        
    }

    return  com_step_flg;
}

//==============================================================================
//
//==============================================================================
void Send_RS485(uint8_t *msg, uint8_t num)
{
    uint8_t     i;

    UART485_CTRL  = 1;    
    for( i=0; i < num; i++){
        while (!UART485_TXSTA_TRMT); // 送信バッファが空になるまで待機
        UART485_TXREG = msg[i];
    }
    while (!UART485_TXSTA_TRMT); // 送信バッファが空になるまで待機
    UART485_CTRL  = 0;    
}               


//==============================================================================
//
//==============================================================================
void rs485_com_task(void)
{

    uint8_t i = 0;
    uint8_t num = 0;
    
    uint8_t sum = 0;
    float   dt = 10.25;
    uint32_t dt32;
    
    dt32 =  (uint32_t)dt;
    
    
    if( command_rcv() == COM_RCV_COMPLITE){
        if( cmd_mesg[RS485_ADD_ID+1] == MY_RS485_ADDRESS ){
           switch( cmd_mesg[RS485_CMD_ID+1] ){
            case RS485_CMD_STATUS:
                Res_mesg[num++] = '#';
                Res_mesg[num++] = RS485_AD_MASTER;
                Res_mesg[num++] = MY_RS485_ADDRESS;

                Res_mesg[num++] = '*';
                Res_mesg[num++] = cmd_mesg[RS485_CMD_ID+1];
                Res_mesg[num++] = 0;
                
                Res_mesg[num++] = SLV_STATUS_IDLE;
                Res_mesg[num++] = 0;
                Res_mesg[num++] = SENS_NON;
                
                for( i=0; i < num; i++){
                    sum += Res_mesg[i];
                }
                
                Res_mesg[num++] = '$';
                Res_mesg[num++] = sum;

                Send_RS485(Res_mesg, num);
                
                break;
            case RS485_CMD_VERSION:
                i = 0;
                Res_mesg[num++] = '#';
                Res_mesg[num++] = RS485_AD_MASTER;
                Res_mesg[num++] = MY_RS485_ADDRESS;

                Res_mesg[num++] = '*';
                Res_mesg[num++] = cmd_mesg[RS485_CMD_ID+1];
                Res_mesg[num++] = 0;
                
                Res_mesg[num++] = SLV_VERSION & 0x00ff;
                Res_mesg[num++] = SLV_VERSION >> 8;
                Res_mesg[num++] = 0;
                Res_mesg[num++] = 0;
                
                for( i=0; i < num; i++){
                    sum += Res_mesg[i];
                }
                
                Res_mesg[num++] = '$';
                Res_mesg[num++] = sum;

                Send_RS485(Res_mesg, num);

                break;
            case RS485_CMD_MESUR:
                i = 0;
                Res_mesg[num++] = '#';
                Res_mesg[num++] = RS485_AD_MASTER;
                Res_mesg[num++] = MY_RS485_ADDRESS;

                Res_mesg[num++] = '*';
                Res_mesg[num++] = cmd_mesg[RS485_CMD_ID+1];
                Res_mesg[num++] = 0;
                

                for( i=0; i < num; i++){
                    sum += Res_mesg[i];
                }
                
                Res_mesg[num++] = '$';
                Res_mesg[num++] = sum;

                Send_RS485(Res_mesg, num);

                break;
            case RS485_CMD_MESUR_DATA:
                i = 0;
                Res_mesg[num++] = '#';
                Res_mesg[num++] = RS485_AD_MASTER;
                Res_mesg[num++] = MY_RS485_ADDRESS;

                Res_mesg[num++] = '*';
                Res_mesg[num++] = cmd_mesg[RS485_CMD_ID+1];
                Res_mesg[num++] = 0;
                
                Res_mesg[num++] = (uint8_t) dt32;
                Res_mesg[num++] = (uint8_t) ( dt32 >> 8 );
                Res_mesg[num++] = (uint8_t) ( dt32 >> 16 );
                Res_mesg[num++] = (uint8_t) ( dt32 >> 24 );

                for( i=0; i < num; i++){
                    sum += Res_mesg[i];
                }
                
                Res_mesg[num++] = '$';
                Res_mesg[num++] = sum;

                Send_RS485(Res_mesg, num);
                
                break;
            default:
                break;
           }
                      
        }
        else{
            com_step_flg = COM_RCV_INIT;
            cmd_ptr = 0;
        }
    }
}

