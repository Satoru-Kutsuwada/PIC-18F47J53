
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

#define MY_RS485_ADDRESS RS485_AD_SLEVE02

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
uint8_t		work_buf[RCV_BUF_SIZE];
uint8_t		rcvnum = 0;
uint8_t		rcv_wpt = 0;
uint8_t		rcv_rpt = 0;
uint16_t    work_buf_num = 0;

typedef enum{
	COM_PROTOCOL_SEND,
	COM_PROTOCOL_RECIVE,
	COM_PROTOCOL_RESPONS

} COM_PROTOCOL_STEP;

COM_PROTOCOL_STEP cp_step = COM_PROTOCOL_RECIVE;

/* Private variables ---------------------------------------------------------*/
#define TEXT_LENGTH		6
const char com_start_text[]   = "COM-ST\0";
const char res_start_text[]   = "RES-ST\0";
const char message_end_text[] = "MSGEND\0";
char comp_buf[TEXT_LENGTH+1];


#define RS485_MSG_MAX	32

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
  
    while( rcvnum > 0 ){
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


                sum = 0;
                for( i = 0; i< (cmd_ptr -1); i++){
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

        printf("dt=0x%02x, com_step_flg=%d,rcvnum=%d\r\n", dt, com_step_flg,rcvnum);
        
    }

    return  com_step_flg;
}

//==============================================================================
//
//==============================================================================
void Send_RS485(uint8_t *msg, uint8_t num)
{
    uint8_t     i;
    uint8_t     c[2];

    for( i=0; i < num; i++){
        cmd_char[i] = ((msg[i]<0x20||msg[i]>=0x7f)? '.': msg[i]);
    }
    
    printf("\r\nRESPONS MESSAGE = \r\n ");
    for( i=0; i < num; i++){
		printf("%02x ", msg[i]);
	}
    printf("\r\n ");
    c[1] = '\0';
    for( i=0; i < num; i++){
        c[0] = cmd_char[i];
		printf(" %s ", c);
	}
    printf("\r\n");
    
        
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
uint16_t  Get_end_test_pt(uint16_t num,uint8_t *buf )
{
	uint16_t	i;
	uint16_t	rtn;

	rtn = 0;

	for( i=0; i<num; i++){
		if( (i+TEXT_LENGTH) > num ){
			rtn = 0;
			break;
		}
		else if( buf[i] == message_end_text[0]
				&& buf[i+1] == message_end_text[1]
				&& buf[i+2] == message_end_text[2]
				&& buf[i+3] == message_end_text[3]
				&& buf[i+4] == message_end_text[4]
				&& buf[i+5] == message_end_text[5] ){

			rtn = i;
			break;
		}
	}

	return rtn;
}
//==============================================================================
//
//==============================================================================
RETURN_STATUS  Check_Message(uint16_t num, uint8_t *src, uint8_t *dist)
{
	uint16_t	i;
	uint16_t	j;
	uint16_t	start;
	uint16_t	end;
    uint8_t     c[2];

    RETURN_STATUS   status;
    status = RET_FALSE;

            
            
    start = 0xff;
	for( i=0; i<num; i++){
		if( src[i] == com_start_text[0]
				&& src[i+1] == com_start_text[1]
				&& src[i+2] == com_start_text[2]
				&& src[i+3] == com_start_text[3]
				&& src[i+4] == com_start_text[4]
				&& src[i+5] == com_start_text[5] ){
 
            start = i;
            
            if( src[i+5+2] ==  MY_RS485_ADDRESS ){
                status = RET_TRUE;
            }
            
			break;
		}
	}
    
    if( start == 0xff ){
        for( i=0; i<num; i++){
            if( src[i] == res_start_text[0]
                    && src[i+1] == res_start_text[1]
                    && src[i+2] == res_start_text[2]
                    && src[i+3] == res_start_text[3]
                    && src[i+4] == res_start_text[4]
                    && src[i+5] == res_start_text[5] ){

                start = i;
                break;
            }
        }
    }

	for( i=0; i<num; i++){
		if( src[i] == message_end_text[0]
				&& src[i+1] == message_end_text[1]
				&& src[i+2] == message_end_text[2]
				&& src[i+3] == message_end_text[3]
				&& src[i+4] == message_end_text[4]
				&& src[i+5] == message_end_text[5] ){

			end = i+5+1;
			break;
		}
	}

	j = 0;
	for( i=start; i < end; i++){
		dist[j++] = src[i];
	}

	for( i=0; i < j; i++){
		cmd_char[i] = ((dist[i]<0x20||dist[i]>=0x7f)? '.': dist[i]);
	}    
    
    printf("\r\nCOMMAND MESSAGE = \r\n ");
    for( i=0; i < j; i++){
		printf("%02x ",dist[i]);
	}
    printf("\r\n ");
    c[1] = '\0';
    for( i=0; i < j; i++){
        c[0] = cmd_char[i];
		printf(" %s ", c);
	}
    printf("\r\n");
    
    
    
    
    
   return  status;

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
    
    uint8_t command ;
 

    switch(cp_step){
    case COM_PROTOCOL_RECIVE:
        while( rcvnum  > 0 ){
            work_buf[work_buf_num  ++] = Get_rcv_data();

            if( Get_end_test_pt(work_buf_num, work_buf) != 0 ){
                if( Check_Message(work_buf_num, work_buf, cmd_mesg) == RET_TRUE ){
                    cp_step = COM_PROTOCOL_RESPONS;
                }
                else{
                    printf("Discard received message \r\n");
                }
                work_buf_num = 0;
                
                break;
            }
        }        
        
        break;
    case COM_PROTOCOL_RESPONS:
        
        command = cmd_mesg[ TEXT_LENGTH + RS485_CMD_ID + 1] ;
        printf("COMMAND=%d\r\n", command);

        // START Text
        for( i=0; i < TEXT_LENGTH; i++ ){
            Res_mesg[i] = res_start_text[i];
        }
        
        num = TEXT_LENGTH;        
        Res_mesg[num++] = '#';
        Res_mesg[num++] = RS485_AD_MASTER;
        Res_mesg[num++] = MY_RS485_ADDRESS;

        Res_mesg[num++] = '*';
        Res_mesg[num++] = command;

            
        switch( command ){
        case RS485_CMD_STATUS:
            Res_mesg[num++] = 0;
            Res_mesg[num++] = SLV_STATUS_IDLE;
            Res_mesg[num++] = 0;
            Res_mesg[num++] = SENS_NON;
            break;

        case RS485_CMD_VERSION:
            Res_mesg[num++] = 0;
            Res_mesg[num++] = SLV_VERSION & 0x00ff;
            Res_mesg[num++] = SLV_VERSION >> 8;
            Res_mesg[num++] = 0;
            Res_mesg[num++] = 0;
            break;
            
        case RS485_CMD_MESUR:
            Res_mesg[num++] = 0;
            break;
       
        case RS485_CMD_MESUR_DATA:
            Res_mesg[num++] = 0;
            Res_mesg[num++] = (uint8_t) dt32;
            Res_mesg[num++] = (uint8_t) ( dt32 >> 8 );
            Res_mesg[num++] = (uint8_t) ( dt32 >> 16 );
            Res_mesg[num++] = (uint8_t) ( dt32 >> 24 );
            break;
        default:
            break;
            
        }
        
        // チェックサム
        for( i=TEXT_LENGTH; i < num; i++){
            sum += Res_mesg[i];
        }

        Res_mesg[num++] = '$';
        Res_mesg[num++] = sum;

		// END Text
		for( i=0; i < TEXT_LENGTH; i++ ){
			Res_mesg[num++] = message_end_text[i];
		}

        // レスポンス送信
        Send_RS485(Res_mesg, num);

        cp_step = COM_PROTOCOL_RECIVE;

        break;
    }
    
}

