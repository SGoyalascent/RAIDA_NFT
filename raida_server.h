#ifndef AGENT_SERVER_H
#define AGENT_SERVER_H
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

//--------------------------------------------
#define RAIDA_SERVER_MAX 					25
//--------------------------------------------
#define SERVER_CONFIG_BYTES 				20
#define SECURITY_CHECK_MUL_FACTOR  			1000
#define BACKUP_FREQ_MUL_FACTOR 			10
#define REFRESH_DNS_MUL_FACTOR 			1000
//--------------------------------------------
#define SHARDS_MAX 						32
#define SHARD_AVAILABLE_VALUE 				255
#define SHARD_NOT_AVAILABLE_VALUE   			0
//-------------------------------------------
#define DNS_LEN_MAX 						4
#define DNS_PORT_MAX 					2
//------------------------------------------
#define AN_MAX							32
//------------------------------------------
#define TICKETS_MAX 						10000
#define TICKET_SR_NOS_MAX 				2000
//------------------------------------------
#define COIN_CONFIG_BYTES 				5
#define COIN_ID_INDEX		 				0
#define COIN_CLOUD_INDEX		 			1
#define COIN_TABLES_CNT					1     // NFT COIN_ID 1
//-------------------------------------------
#define RAIDA_LEGACY_IP_MAX 				4
#define RAIDA_LEGACY_PORT_MAX 				2
//-------------------------------------------
#define ENCRYPTION_CONFIG_BYTES			16
//-------------------------------------------
#define COIN_KEY_AN_SIZE					16
#define COIN_KEY_FILE_ROWS				28
#define COIN_KEY_FILE_SIZE					COIN_KEY_FILE_ROWS * COIN_KEY_AN_SIZE

#define DENO_TYPE_ALL					0	//( has 1,5,25,100,250)
#define DENO_TYPE_SINGLE					1	//( has 1)

#include "udp_socket.h"
#include "sky_wallet.h"
#include "NFT.h"
#include <dirent.h> 
//This is used for inter RAIDA Communication 
struct my_id_coins {
	unsigned char AN[RAIDA_SERVER_MAX][COIN_KEY_AN_SIZE];
	unsigned int sr_no;
};

struct server_config {
	unsigned char raida_id;
	unsigned int port_number;
	unsigned int security_check_frequency;
	unsigned int backup_frequency;
	unsigned int refresh_dns;
	unsigned int show_regs_coins_max;
	unsigned int show_denom_coins_max;
	unsigned int show_change_coins_max;
	unsigned int bytes_per_frame;
	unsigned int del_ticket_time_intervel;
	unsigned int del_free_id_time_intervel;	//days
	unsigned char del_encryp2_time_intervel;	//in secs 255 (max), this is the deletion time of the encryption keys set using put key
	unsigned char deno_type;	// denomination type 0, for all , 1 for single 
	unsigned int my_id_coins_cnt;//count of id coins (for communcation between raida's
};
struct coin_config {
	unsigned char coin_id;
	unsigned int page_size;
	unsigned int no_of_pages;
	unsigned char *pages_changed;
};

struct dns_config {
	int sockfd;
	struct sockaddr_in servaddr,cliaddr;
	unsigned char buffer[1024];
};

struct raida_legacy_config {
	int sockfd;
	struct sockaddr_in servaddr,cliaddr;
	unsigned char buffer[1024];
};

struct master_ticket{
	uint32_t ticket_no;
	uint32_t raida_claim;	//0-24 bits represent the ticket spent or not
	uint32_t time_stamp;
};
struct date {
	unsigned int year;
	unsigned char month,day,hh,mm,ss;
};

struct coin_id {
	unsigned char **AN;
	unsigned char *MFS;
	unsigned char **GUID;
	struct master_ticket **TICKETS;
	unsigned int AN_CNT;
	unsigned char *free_id_days;

};
//-------------------------------------------------
void welcomeMsg();
int load_my_id_coins();
int load_raida_no();
int load_server_config();
int load_shards_config();
int load_dns_config();
int load_raida_legacy_config();
int load_an(unsigned int,unsigned int);
int load_mfs(unsigned int);
void add_ticket(unsigned int, uint32_t,uint32_t);
//void delete_old_tickets(uint32_t time_diff);
//int find_serial_no(uint32_t ticket_no);
long get_time_cs();
char compare_date(struct date,struct date);
uint32_t compare_struct(const void *,const void *);
void update_an_pages(unsigned int );
void update_coin_owner();
void* update_coin_owner_details(void *);
void* free_id_days_left_thread(void *);
int load_coin_owner();
//-------------------------------------------------
extern char execpath[256];
extern unsigned char shards_config[SHARDS_MAX];
extern struct server_config server_config_obj;
extern struct dns_config dns_config_obj[RAIDA_SERVER_MAX];
extern struct raida_legacy_config raida_legacy_config_obj[RAIDA_SERVER_MAX];
extern unsigned int coin_id_cnt;
extern struct coin_id coin_id_obj[255];
extern struct coin_config *coin_config_obj;
extern unsigned int *pages_changed;
extern uint8_t legacy_encrypt_key[ENCRYPTION_CONFIG_BYTES];
extern struct my_id_coins *my_id_coins_obj;
#endif
