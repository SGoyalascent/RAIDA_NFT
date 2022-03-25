#ifndef NFT_H
#define NFT_H
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>


#include "raida_server.h"
#include "sky_wallet.h"
//--------------------------------------------------------------------
#define FRAME_TIME_OUT_SECS		1 
#define UDP_BUFF_SIZE 			65535
//------------------------------------------------------------------
#define RESPONSE_HEADER_MAX 		65535
//-----------------------------------------------------------------
//Indexs at which the bytes start
#define REQ_CL  					0
#define REQ_SP  					1
#define REQ_RI  					2
#define REQ_SH  					3
#define REQ_CM  					4
#define REQ_VE  					6
#define REQ_CI  					7
#define REQ_NO_1  				9
#define REQ_NO_2  				10
#define REQ_NO_3  				11
#define REQ_NO_4  				12
#define REQ_NO_5  				13
#define REQ_EC  					12
#define REQ_FC  					14
#define REQ_EN  					16
#define REQ_COIN_ID				17
#define REQ_NO_6				19
#define REQ_NO_7				20
#define REQ_NO_8				21
#define REQ_SR_N0				19


#define REQ_HEAD_MIN_LEN 			22
//-------------------------------------------------------------
#define TY_BYTES_CNT				1
#define TT_BYTES_CNT				1
#define RA_BYTES_CNT				1
#define DENOM_BYTES_CNT			1
#define PAGE_NO_BYTES_CNT		1
#define RAIDA_BYTE_CNT			1
#define ROWS_BYTES_CNT			1
#define YY_BYTES_CNT				1
#define MM_BYTES_CNT			1
#define DD_BYTES_CNT				1
#define COIN_TYPE_BYTES_CNT		1
#define MFS_BYTES_CNT			1
#define FREE_ID_BYTES_CNT			1
#define CT_BYTES_CNT				1
#define CMD_END_BYTES_CNT		2
#define SN_BYTES_CNT				3	
#define OWNER_ID_BYTES_CNT		3
#define RECORD_BYTES_CNT			3
#define KEY_ID_BYTES_CNT			3
#define TK_BYTES_CNT				4
#define MS_BYTES_CNT			4
#define AMT_BYTES_CNT			4
#define HS_BYTES_CNT				4
#define UPGRADE_COIN_TK_CNT		4
#define EN_BYTES_CNT				5
#define TIME_STAMP_BYTES_CNT		6
#define FREE_ID_SERV_LOCK_TIME		6
#define DT_BYTES_CNT				7	
#define RESP_BUFF_MIN_CNT			12
#define FIX_SRNO_MATCH_CNT		13
#define GUID_BYTES_CNT 			16
#define AN_BYTES_CNT 			16
#define PAN_BYTES_CNT 			16
#define PG_BYTES_CNT	 			16
#define KEY_BYTES_CNT			16
#define CH_BYTES_CNT				16
#define PWD_BYTES_CNT			16
#define NOUNCE_BYTES_CNT			16	
#define LEGACY_RAIDA_TK_CNT		22
#define MD_HASH_BYTES_CNT		32
#define META_DATA_BYTES_CNT		50
#define REQ_END					62
#define UPGRADE_COIN_SRNO_CNT		2000

//-------------------------------------------------
#define ENCRY2_KEYS_MAX			10000
#define ROWS_MUL_FACTOR			100
#define MAX_ROWS_SHOW_STAT		255	
#define FRAMES_MAX				100
#define EN_CODES_MAX			255
#define DEFAULT_YEAR				2000
#define COINS_MAX				2000
#define SECS_IN_DAY				60 * 60 *24
//#define SECS_IN_DAY				1

#define ENCRYP_NONE					0
#define ENCRYP_128_AES_CTR_SN			1
#define ENCRYP_128_AES_CTR_KEY_TABLE	2		
//------Indexs for Response Header----------------------------
#define RES_RI  						0
#define RES_SH  						1
#define RES_SS 	 					2
#define RES_EX 						3
#define RES_RE 						4
#define RES_EC 						6
#define RES_HS 						8

#define RES_HEAD_MIN_LEN 				12
//---------Status Error codes----------------------------------------
#define INVALID_CLOUD_ID 		   				1
#define RAIDA_ OFFLINE 			   			2
#define INVALID_FRAME_CNT 	 				15
#define INVALID_PACKET_LEN 	 				16
#define FRAME_TIME_OUT	 	 				17
#define WRONG_RAIDA 			 				18
#define INVALID_SPLIT_ID 		 				19
#define SHARD_NOT_AVAILABLE   					20
#define VALIDATE_TICKET_FOUND					21
#define VALIDATE_TICKET_NOT_FOUND				22
#define VALIDATE_TICKET_INVALID_RAIDA			23
#define INVALID_CMD 			  				24
#define COIN_ID_NOT_FOUND 	  				25
#define COIN_LIMIT_EXCEED			 	  		26
#define INVALID_EN_CODE		 	  			27
#define COIN_OWNER_ID_NOT_FOUND		 	  	28
#define POWN_AN_PAN_SAME					30
#define PACKET_ORDER_LOSS					31
#define LEN_OF_BODY_CANT_DIV_IN_COINS			32
#define INVALID_END_OF_REQ					33
#define ENCRYPTION_ERROR					34
#define EMPTY_REQ_BODY						36
#define VALIDATE_TICKET_CLAIMED_EARLIER			37
#define VALIDATE_TICKET_CLAIMED				38
#define COIN_NO_NOT_FOUND					39
#define SN_ALL_READY_IN_USE					40
#define SERVICE_LOCKED						41
#define FAILED_TO_AUTHENTICATE					64
#define PAGE_NOT_FOUND						66
#define BREAK_COUNTER_FEIT					70
#define BREAK_COINS_NOT_FOUND					72
#define BREAK_COINS_SUM_NOT_MATCH				74
#define BREAK_CANNOT_BREAK					76
#define JOIN_COUNTER_FEIT						80
#define JOIN_COINS_NOT_FOUND					82
#define JOIN_COINS_SUM_NOT_MATCH				84
#define JOIN_CANNOT_JOIN						86
#define FIX_ALL_TICKET_ZERO					90
#define RAIDA_WORKING						96
#define KEY_NOT_CONFIG_ENCRY_2				100
#define NO_STATEMENTS_FOUND					120		
#define IDENTIFY_COIN_FOUND					192
#define IDENTIFY_COIN_NOT_FOUND				193
#define SYNC_ADD_COIN_EXIST					200
#define SYNC_DEL_COIN_NOT_EXIST				202
#define FIND_ALL_NONE						208
#define FIND_ALL_AN							209
#define FIND_ALL_PA							210
#define FIND_MIXED							211
#define UPGRADE_COIN_SRNO_CONT				213
#define UPGRADE_COIN_SRNO_NON_CONT			214
#define ALL_PASS							241
#define ALL_FAIL								242
#define MIX								243
#define LEGACY_RAIDA_TICKET_NOT_FOUND                	245
#define LEGACY_RAIDA_TIME_OUT					246
#define LEGACY_RAIDA_FAIL						247
#define RAIDA_TICKET_NOT_FOUND                		248
#define SUCCESS								250
#define FAIL								251
#define NO_ERR_CODE	 		   				255

//----------Command codes-----------------------------------------
#define CMD_POWN 							0
#define CMD_DETECT	 						1
#define CMD_FIND							2
#define CMD_FIX								3
#define CMD_ECHO							4
#define CMD_VALIDATE							5
#define CMD_PUT_KEY							6
#define CMD_GET_KEY							7
#define CMD_IDENTIFY							8
#define CMD_RECOVER						       10
#define CMD_GET_TICKET 					       11
#define CMD_VERSION						       15
#define CMD_NEWS						       16
#define CMD_LOGS						       17
#define CMD_PANG                    21
#define CMD_BECOME_PRIMARY				       42
#define CMD_BECOME_MIRROR				       43
#define CMD_CHECK_UPDATES				       44
#define CMD_FIX_V2						       50	
//----------------------------------------------------------------------------
#define FIND_COIN_FAILED						0
#define FIND_COIN_AN_PASSED					1
#define FIND_COIN_PA_PASSED					2
//---------------------------------------------------------------------------
#define STATE_WAIT_START						1
#define STATE_START_RECVD					2
#define STATE_WAIT_END						3
#define STATE_END_RECVD						4
//--------------------------------------------------------------------------	
#define  UDP_RESPONSE 						0
#define  FIFO_RESPONSE 						1
//--------------------------------------------------------------------------
#define FIX_VERSION_1							1			
#define FIX_VERSION_2							2		
//-------------------------------------------------------------
#define NFTS_CNT_MAX									16777216 	// NO_OF_PAGES * RECORDS_PER_PAGE
#define NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE		8192000		//in bytes(binary) (8MB)
#define TCP_BUFFER_MAX_SIZE								204800000   //in bytes(binary) (200MB) 
#define TCP_BUFF_PER_FRAME_SIZE							65535 		//in bytes(binary) (64 KB)
#define TCP_FRAMES_MAX									102400		//per frame 2KB	
#define TCP_PACKET_SIZE									2048		//in bytes(2KB)								
#define NO_OF_PAGES_MAX									16384
#define RECORDS_PER_PAGE_MAX							1024
#define REQ_BODY_SEPERATOR_BYTES_CNT					3

#define NFT_TEST_CREATE                 200
#define NFT_CREATE                      202
#define NFT_READ_META                   203
#define NFT_READ_DATA                   204
#define NFT_UPDATE                      206
#define NFT_ADD 	                    208

//----------------------------NFT Error Codes--------------------------
#define NFT_CREATE_SUCCESS 											50
#define NFT_CREATE_WARNING_NFT_ALREADY_EXISTS_FOR_COINS				51
#define NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND					52
#define NFT_CREATE_EXCESS_DATA_FOR_COINS							53
#define NFT_CREATE_GUID_NOT_UNIQUE									54
#define NFT_CREATE_COUNTERFIET_COINS								55
#define NFT_UPDATE_GUID_NOT_FOUND									56	
#define NFT_ADD_GUID_NOT_FOUND										57
#define NFT_ADD_COUNTERFIET_COINS									58
//----------------------------------------------------------------------------------------


struct NFT_TABLE {
	unsigned int SN;
	unsigned char AN[AN_BYTES_CNT];
	unsigned char MFS;
	unsigned char GUID[GUID_BYTES_CNT];
};

extern struct NFT_TABLE nft_details[NFTS_CNT_MAX];


int Read_NFT_Configuration_File();
void tcp_set_time_out(unsigned char secs);
int init_tcp_socket();
int tcp_listen_request();
void tcp_process_request(unsigned int packet_len);
void tcp_send_err_resp_header(unsigned char status_code);
void tcp_prepare_resp_header(unsigned char status_code);
void tcp_send_response(unsigned char status_code,unsigned int size);
unsigned char tcp_validate_request_header(unsigned char * buff,int packet_size);
unsigned int validate_request_body_only_coins(unsigned int packet_len,unsigned char bytes_per_coin,unsigned int req_body_without_coins,int *req_header_min);
unsigned char validate_request_body_nfts(unsigned int packet_len,unsigned int index);
unsigned char isGuidUnique(unsigned char *guid);
unsigned char authenticate_coins(unsigned int no_of_coins, unsigned int index, unsigned int coin_id);
void execute_test_create(unsigned int packet_len, int coin_id);
void execute_create(unsigned int packet_len, unsigned int coin_id);
void execute_update(unsigned int packet_len, unsigned int coin_id);
void execute_read_data(unsigned int packet_len, unsigned int coin_id);
void execute_read_meta(unsigned int packet_len, unsigned int coin_id);
void execute_add_coins(unsigned int packet_len, unsigned int coin_id);
#endif