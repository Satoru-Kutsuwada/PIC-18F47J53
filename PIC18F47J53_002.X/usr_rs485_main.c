
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
#include <string.h>

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

/* Private variables ---------------------------------------------------------*/
typedef enum{
	// スタートテキスト
	COM_START_TXT_00 = 0,
	COM_START_TXT_01,
	COM_START_TXT_02,
	COM_START_TXT_03,
	COM_START_TXT_04,
	COM_START_TXT_05,

	// 通信カウンタ
	COM_COUNTER_L,
	COM_COUNTER_H,

	// アドレス
	COM_ADDRESS_ID,
	COM_ADDRESS_DIST,
	COM_ADDRESS_SORC,
	COM_ADDRESS_RESERVD,

	// コマンド・レスポンス
	COM_COMMAND_ID,
	COM_COMMAND,
	COM_COMMAND_RESP,
	COM_COMMAND_01,
	COM_COMMAND_02,
	COM_COMMAND_03,
	COM_COMMAND_04,
	COM_COMMAND_05,
	COM_COMMAND_06,
	COM_COMMAND_07,
	COM_COMMAND_08,
	COM_COMMAND_09,
	COM_COMMAND_10,
	COM_COMMAND_11,

	//　チェックサム
	COM_CHKSUM_ID,
	COM_CHKSUM,

	// エンドテキスト
	COM_END_TXT_00,
	COM_END_TXT_01,
	COM_END_TXT_02,
	COM_END_TXT_03,
	COM_END_TXT_04,
	COM_END_TXT_05,

	COM_TABLE_MAX

}COM_TABLE;


/* Command 01 -------------------------------*/
/* Command 01 RESPONS -----------------------*/
#define COM_ERR_DITAIL 		COM_COMMAND_01
#define COM_SES_KIND 		COM_COMMAND_02

/* Command 02 ---------*/
/* Command 02 RESPONS -----------------------*/
#define COM_SLV_VERSION_L 	COM_COMMAND_01
#define COM_SLV_VERSION_H 	COM_COMMAND_02
#define COM_SNS_VERSION_L 	COM_COMMAND_03
#define COM_SNS_VERSION_H 	COM_COMMAND_04

/* Command 03 ---------*/
#define COM_SENS_CTRL 		COM_COMMAND_01

/* Command 03 RESPONS -----------------------*/

/* Command 04 ---------*/
#define COM_DATA_KIND 		COM_COMMAND_01

/* Command 04 RESPONS -----------------------*/
#define COM_MESUR_DATA_L 	COM_COMMAND_01
#define COM_MESUR_DATA_ML 	COM_COMMAND_02
#define COM_MESUR_DATA_MH 	COM_COMMAND_03
#define COM_MESUR_DATA_H 	COM_COMMAND_04


uint8_t		Cmd_mesg[COM_TABLE_MAX];
uint8_t		Res_mesg[COM_TABLE_MAX];
uint8_t		cmd_char[COM_TABLE_MAX];
uint8_t		cmd_ptr = 0;
uint8_t		res_ptr = 0;
uint16_t	com_counter = 0;



uint8_t     rcsta;
uint8_t     txsta;


typedef enum{
	RS485_DATA_COMMAND = 0,
	RS485_DATA_RESPONS,
	RS485_DATA_NONE
}COMUNICATION_DATA_KIND;




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
void Send_RS485(uint8_t *msg, uint8_t num)
{
    uint8_t     i;
    uint8_t     c[2];
        
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
//=============================================================================
uint8_t Get_command_chksum(uint8_t start, uint8_t end,uint8_t *dt )
{
	uint8_t		sum = 0;
	uint8_t		i;

	for( i=start; i<end; i++){
		sum += dt[i];
		//printf("%02x ", dt[i]);
	}
	//printf(" sum=%02x\r\n",sum);

	return sum;
}

//==============================================================================
//
//==============================================================================
RETURN_STATUS  Check_Message(uint16_t num, uint8_t *src)
{
    RETURN_STATUS	status = RET_TRUE;
	uint16_t	i;
	uint16_t	j;
	uint16_t	start;
	uint16_t	end;
    uint8_t     c[2];
    uint8_t     flg = RS485_DATA_NONE;
    uint8_t     *pt;

	
    //-------------------------------------------------------------
	// スタートテキスト検索(他スレーブからのレスポンス))
	//-------------------------------------------------------------
	start = 0xffff;
    for( i=0; i<num; i++){
        if( src[i] == res_start_text[0]
                && src[i+1] == res_start_text[1]
                && src[i+2] == res_start_text[2]
                && src[i+3] == res_start_text[3]
                && src[i+4] == res_start_text[4]
                && src[i+5] == res_start_text[5] ){


            flg = RS485_DATA_RESPONS;
            status = RET_FALSE;
            start = i;
            break;
        }
    }

    //-------------------------------------------------------------
	// スタートテキスト検索(マスタからのコマンド))
	//-------------------------------------------------------------
	if(start == 0xffff){
        for( i=0; i<num; i++){
            if( src[i] == com_start_text[0]
                    && src[i+1] == com_start_text[1]
                    && src[i+2] == com_start_text[2]
                    && src[i+3] == com_start_text[3]
                    && src[i+4] == com_start_text[4]
                    && src[i+5] == com_start_text[5] ){

                flg = RS485_DATA_COMMAND;
                start = i;
                break;
            }
        }
    }	
    
    if( start == 0xffff ){
		status = RET_FALSE;
		printf("Error : Respons Data start txt none \r\n");
	}
    
	
    //-------------------------------------------------------------
	// エンドテキスト検索
	//-------------------------------------------------------------
    end = 0xffff;
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
    
    if( end == 0xffff ){
		status = RET_FALSE;
		printf("Error : Command / Respons Data end txt none \r\n");
	}	

    
    if( status == RET_TRUE ){
        //-------------------------------------------------------
        // Command buffer Clesr
        //-------------------------------------------------------
            for( i=0; i < COM_TABLE_MAX; i++ ){
                Cmd_mesg[i] = 0;
            }

        //-------------------------------------------------------------
        // Comに転送
        //-------------------------------------------------------------
        j = 0;
        for( i=start; i < end; i++){
            if( j < COM_TABLE_MAX ){
                Cmd_mesg[j] = src[i];
            }
            else{
                status = RET_FALSE;
                printf("Error : Respons data size over\r\n");
            }
            j++;
        }
    }    
    //-------------------------------------------------------------
    // チェックサム確認
    //-------------------------------------------------------------
    if( status == RET_TRUE ){
        if( Cmd_mesg[COM_CHKSUM] != Get_command_chksum(COM_COUNTER_L, COM_CHKSUM_ID, Cmd_mesg)){
            status = RET_FALSE;
            printf("Error : Respons data chksum eoor");
        }
    }        
	//-------------------------------------------------------------
	// レスポンスデータをログ表示
	//-------------------------------------------------------------
	for( i=0;  i < COM_TABLE_MAX ; i++ ){
		cmd_char[i] =  (uint8_t)((Cmd_mesg[i]<0x20||Cmd_mesg[i]>=0x7f)? '.': Cmd_mesg[i]);
	}

    if( Cmd_mesg[COM_ADDRESS_DIST]==  MY_RS485_ADDRESS ){
        printf("\r\nCOMMAND MESSAGE : Master->My Device \r\n ");
    }
    else{
        printf("\r\nCOMMAND MESSAGE : Master->Another Device \r\n ");
    }
    
	for( i=0; i < COM_TABLE_MAX ; i++){
		printf("%02x ", Cmd_mesg[i]);
	}
	printf("\r\n ");
	c[1] = '\0';
	for( i=0; i < COM_TABLE_MAX ; i++){
		c[0] = cmd_char[i];
		printf(" %s ", c);
	}
	printf("\r\n");


	//-------------------------------------------------------------
	// 他スレーブからマスタへのレスポンスデータ
	//-------------------------------------------------------------
    if( flg == RS485_DATA_RESPONS ){
        status = RET_FALSE;
    }
	//-------------------------------------------------------------
	// 自スレーブ宛のメッセージか
	//-------------------------------------------------------------
    if( Cmd_mesg[COM_ADDRESS_DIST] !=  MY_RS485_ADDRESS ){
        status = RET_FALSE;
    } 
   
    
    
   return  status;

}
//==============================================================================
//
//==============================================================================
void    rs485_init(void)
{

    float   dtf;
    uint8_t dt8[10];
    uint8_t *pt;
    float   *ptf;
    
    

    dtf = 3.14;
    printf("  dtf= %f\r\n",dtf);
    printf("  dtf= %e\r\n",dtf);
    printf("  dtf= %g\r\n",dtf);
    printf("  dtf= %d\r\n\r\n",dtf);

    pt = (uint8_t *)&dtf;
    dt8[0]=*pt;
    pt++;
    dt8[1]=*pt;
    pt++;
    dt8[2]=*pt;
    pt++;
    dt8[3]=*pt;

    printf("dt8[]= %02x %02x %02x %02x \r\n",dt8[0],dt8[1],dt8[2],dt8[3]);
    
    ptf=(float *)dt8;
    printf("  *ptf= %g\r\n\r\n", *ptf);
    
    pt = (uint8_t *)&dtf;
    dt8[3]=*pt;
    pt++;
    dt8[2]=*pt;
    pt++;
    dt8[1]=*pt;
    pt++;
    dt8[0]=*pt;

    printf("dt8[]= %02x %02x %02x %02x \r\n",dt8[0],dt8[1],dt8[2],dt8[3]);
    
    ptf=(float *)dt8;
    printf("  *ptf= %g\r\n", *ptf);
    
printf("char=%d\r\n",sizeof(char));
printf("short=%d\r\n",sizeof(short));
printf("int=%d\r\n",sizeof(int));
printf("long=%d\r\n",sizeof(long));
printf("float=%d\r\n",sizeof(float));
printf("double=%d\r\n",sizeof(double));

    
}

//==============================================================================
//
//==============================================================================
void rs485_com_task(void)
{

    uint8_t     i, j;
    float       dtf;
    uint8_t     command ;
    uint8_t     c[2] ;
    uint8_t     *dt;
    
    

    switch(cp_step){
    case COM_PROTOCOL_RECIVE:
        while( rcvnum  > 0 ){
            work_buf[work_buf_num  ++] = Get_rcv_data();

            if( Get_end_test_pt(work_buf_num, work_buf) != 0 ){
                if( Check_Message(work_buf_num, work_buf) == RET_TRUE ){
                    cp_step = COM_PROTOCOL_RESPONS;
                    command = Cmd_mesg[COM_COMMAND];
                    printf("COMMAND= %d\r\n", command);
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
        //-------------------------------------------------------
        // Command buffer Clesr
        //-------------------------------------------------------
        for( i=0; i < COM_TABLE_MAX; i++ ){
            Res_mesg[i] = 0;
        }

        //-------------------------------------------------------
        // START Text
        //-------------------------------------------------------
        for( i=0; i < TEXT_LENGTH; i++ ){
            j = COM_START_TXT_00 + i ;
            Res_mesg[j] = res_start_text[i];
        }

        //-------------------------------------------------------
        // 通信カウンタ
        //-------------------------------------------------------
        Res_mesg[COM_COUNTER_L] = Cmd_mesg[COM_COUNTER_L];
        Res_mesg[COM_COUNTER_H] = Cmd_mesg[COM_COUNTER_H];

        //-------------------------------------------------------
        // アドレス
        //-------------------------------------------------------
        Res_mesg[COM_ADDRESS_ID] = '#';
        Res_mesg[COM_ADDRESS_DIST] = RS485_AD_MASTER;
        Res_mesg[COM_ADDRESS_SORC] = MY_RS485_ADDRESS;
                
    	//-------------------------------------------------------
        // コマンド
        //-------------------------------------------------------
        switch(command){
        case RS485_CMD_STATUS:
            Res_mesg[COM_COMMAND_ID] = '*';
            Res_mesg[COM_COMMAND] = command;
            Res_mesg[COM_COMMAND_RESP] = SLV_STATUS_IDLE;
            Res_mesg[COM_ERR_DITAIL] = 0;
            Res_mesg[COM_SES_KIND] = SENS_NON;
            break;
        case RS485_CMD_VERSION:
            Res_mesg[COM_COMMAND_ID] = '*';
            Res_mesg[COM_COMMAND] = command;
            Res_mesg[COM_COMMAND_RESP] = 0;

            Res_mesg[COM_SLV_VERSION_L] = 0x12;
            Res_mesg[COM_SLV_VERSION_H] = 0x01;
            Res_mesg[COM_SNS_VERSION_L] = 0x34;
            Res_mesg[COM_SNS_VERSION_H] = 0x20;            
            
            break;
        case RS485_CMD_MESUR:
            Res_mesg[COM_COMMAND_ID] = '*';
            Res_mesg[COM_COMMAND] = command;
            Res_mesg[COM_COMMAND_RESP] = 0;

            break;
        case RS485_CMD_MESUR_DATA:
            Res_mesg[COM_COMMAND_ID] = '*';
            Res_mesg[COM_COMMAND] = command;
            Res_mesg[COM_COMMAND_RESP] = 0;

            dtf = 10.25;
            dt = (uint8_t *)&dtf;
            printf("dtf=%d \r\n",dtf);

            Res_mesg[COM_MESUR_DATA_L]  = dt[0];
            Res_mesg[COM_MESUR_DATA_ML] = dt[1];
            Res_mesg[COM_MESUR_DATA_MH] = dt[2];
            Res_mesg[COM_MESUR_DATA_H]  = dt[3];

            
            break;
        default:
            break;
        }
        
        //-------------------------------------------------------
        // チェックサム計算
        //-------------------------------------------------------
        Res_mesg[COM_CHKSUM_ID] = '$';
        Res_mesg[COM_CHKSUM] =  Get_command_chksum(COM_COUNTER_L, COM_CHKSUM_ID, Res_mesg);

        //-------------------------------------------------------
        // START Text
        //-------------------------------------------------------
        for( i=0; i < TEXT_LENGTH; i++ ){
            j = COM_END_TXT_00 + i ;
            Res_mesg[j] = message_end_text[i];
        }

        //-------------------------------------------------------
        // デバック用ログ
        //-------------------------------------------------------
        for( i=0;  i < COM_TABLE_MAX ; i++ ){
            cmd_char[i] =  (uint8_t)((Res_mesg[i]<0x20||Res_mesg[i]>=0x7f)? '.': Res_mesg[i]);
        }

        printf("\r\nRESPONS MESSAGE : My Device -> Master\r\n ");
        for( i=0; i < COM_TABLE_MAX ; i++){
            printf("%02x ", Res_mesg[i]);
        }
        printf("\r\n ");
        c[1] = '\0';
        for( i=0; i < COM_TABLE_MAX ; i++){
            c[0] = cmd_char[i];
            printf(" %s ", c);
        }
        printf("\r\n");


        // レスポンス送信
        Send_RS485( Res_mesg, COM_TABLE_MAX );

        cp_step = COM_PROTOCOL_RECIVE;

        break;
    }
    
}

