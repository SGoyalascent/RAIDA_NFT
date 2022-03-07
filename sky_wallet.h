#ifndef SKY_WALLET_H
#define SKY_WALLET_H
#include <stdlib.h>
#include "library.h"
#include "aes.h"
//-----------------------------------------------------------------
#define COIN_OWNER_MAX					5000
#define COIN_OWNER_DETAILS_MAX			5000
#define COIN_UPGRADE_TICKETS_MAX			250
//----------Command codes-------------------------------
#define CMD_FREE_ID_VER_0					30	
#define CMD_FREE_ID_VER_1					31	
#define CMD_DEPOSITE						100
#define CMD_DEPOSITE_PANG				102
#define CMD_WITHDRAW					104
#define CMD_TRANSFER					108
#define CMD_SHOW_BALANCE				110
#define CMD_SHOW_REGISTRY				112
#define CMD_SHOW_COINS_DENOM				114
#define CMD_SHOW_COINS_BY_TYPE			115
#define CMD_SHOW_CHANGE					116	
#define CMD_JOIN						120
#define CMD_JOIN_IN_BANK					121
#define CMD_BREAK						122
#define CMD_BREAK_IN_BANK					123
#define CMD_SHOW_STATEMENT				130
#define CMD_DEL_ALL_STATEMENT				131
#define CMD_SHOW_PAYMENT				132
#define CMD_CHANGE_COIN_TYPE				135
#define CMD_SYNC_TRANS_ADD				150
#define CMD_SYNC_TRANS_DEL				152
#define CMD_SYNC_TRANS_RESP				154
#define CMD_UPGRADE_COIN					215
#define CMD_COIN_CLAIM					216
//----------Transaction Types-----------------------------
#define TRANS_DEPOSITE					0
#define TRANS_WITHDRAW					1
#define TRANS_TRANSFER_IN					2
#define TRANS_TRANSFER_OUT				3
#define TRANS_BREAK						4
#define TRANS_JOIN						5
#define TRANS_PAY_IN						6
#define TRANS_PAY_OUT					7

//----------Coin Types--------------------------------------
#define ANONYMOUS						0
#define PAYMENT 						1
#define RAIDA							2
#define CHANGE							3	
#define COIN_TYPE_PUBLIC					4
#define FOR_SALE						5
#define NFT							6
#define COIN_TYPE_INVALID					255
#define COIN_TYPE_MAX					255
#define COIN_CONV_MAX_SR_NO				300
//---------------------------------------------------------
#define PUBLIC_CHANGE_ID					2
#define SYNC_RESP_RAIDA_CNT				9
#define FREE_ID_VER_0						0
#define FREE_ID_VER_1						1
//-------------Denominations-------------------------------
#define 	DENO_CNT			5
#define 	DENO_1_VAL		1
#define 	DENO_5_VAL		5
#define 	DENO_25_VAL		25
#define 	DENO_100_VAL		100
#define 	DENO_250_VAL		250

#define 	DENO_1_MIN		1	
#define 	DENO_1_MAX		2097152
#define 	DENO_5_MIN  		2097153
#define 	DENO_5_MAX		4194304
#define 	DENO_25_MIN		4194305
#define	DENO_25_MAX		6291456
#define	DENO_100_MIN		6291457
#define	DENO_100_MAX		14680064
#define	DENO_250_MIN		14680065
#define	DENO_250_MAX		16777215
/*
//For testing only
#define 	DENO_1_MIN		1	
#define 	DENO_1_MAX		10
#define 	DENO_5_MIN  		11
#define 	DENO_5_MAX		15
#define 	DENO_25_MIN		16
#define	DENO_25_MAX		21
#define	DENO_100_MIN		22
#define	DENO_100_MAX		26
#define	DENO_250_MIN		27
#define	DENO_250_MAX		32
*/

struct upgrade_coin_tickets{
	unsigned int ticket;
	unsigned int value;
};

struct coin_owner{
	unsigned int	owner_id;
	unsigned int	serial_no;
	unsigned char coin_type;
	unsigned char changed;
};
struct coin_owner_details{
	unsigned int	owner_id, amt,balance;				
	unsigned char trans_type;
	unsigned char guid[16],time_stamp[6],meta_data[50];
	unsigned char changed;
};
extern struct coin_owner			coin_owner_obj[COIN_OWNER_MAX];	
extern struct coin_owner_details		coin_owner_details_obj[COIN_OWNER_DETAILS_MAX];	
extern struct upgrade_coin_tickets          upgrade_coin_tickets_obj[COIN_UPGRADE_TICKETS_MAX];
//-----------Functions----------------------------------
void display();
void execute_join(unsigned int);
void execute_break(unsigned int);
void execute_join_in_bank(unsigned int);
void execute_break_in_bank(unsigned int);
void execute_transfer(unsigned int);
void execute_deposite(unsigned int);
void execute_deposite_pang(unsigned int);
void execute_withdraw(unsigned int);
void execute_show_change(unsigned int);
void execute_show_registry(unsigned int);
void execute_show_balance(unsigned int);
void execute_show_payment(unsigned int);
void execute_show_statement(unsigned int);
void execute_del_statements(unsigned int );
void execute_show_by_denom(unsigned int);
void execute_show_by_coin_type(unsigned int);
void execute_change_coin_type(unsigned int);
void execute_sync_trans_add(unsigned int);
void execute_sync_trans_resp(unsigned int );
void execute_sync_trans_del(unsigned int);
void execute_upgrade_coin(unsigned int);
void execute_coin_claim(unsigned int);
int cmpfunc (const void * , const void * ); 
unsigned long get_denomination(unsigned int);
unsigned char remove_coin_owner(uint32_t ,uint32_t,unsigned char changed);
unsigned char add_coin_owner(uint32_t ,uint32_t ,unsigned char ,unsigned char changed);
unsigned char remove_coin_owner_details(uint32_t ,unsigned char changed);
unsigned char add_coin_owner_details(uint32_t ,unsigned char *,unsigned char *,unsigned char ,unsigned char *,uint32_t ,uint32_t,unsigned char changed);
unsigned char add_upgrade_coin_ticket(uint32_t ,uint32_t );
unsigned char remove_upgrade_coin_ticket(uint32_t);
#endif
