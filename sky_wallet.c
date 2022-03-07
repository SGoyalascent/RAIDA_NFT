#include "raida_server.h"
#include "udp_socket.h"
#include "sky_wallet.h"

struct coin_owner		coin_owner_obj[COIN_OWNER_MAX]={0};	
struct coin_owner_details	coin_owner_details_obj[COIN_OWNER_DETAILS_MAX]={0};	
struct upgrade_coin_tickets   upgrade_coin_tickets_obj[COIN_UPGRADE_TICKETS_MAX]={0};
struct sync_resp {
    char dest_raida;
    uint32_t own_id,sn_cnt,sn_nums[COINS_MAX];
    unsigned char coin_type[COINS_MAX];
}sync_resp_obj[RAIDA_SERVER_MAX];

//---------------------------------------------------------------
// add coin to owner wallet
//---------------------------------------------------------------
unsigned char add_upgrade_coin_ticket(uint32_t ticket,uint32_t value){
	unsigned int i=0;	
	for(i=0;i<COIN_UPGRADE_TICKETS_MAX;i++){
		if(upgrade_coin_tickets_obj[i].ticket == 0){
			upgrade_coin_tickets_obj[i].ticket = ticket;
			upgrade_coin_tickets_obj[i].value = value;
			return 1;
		}
	}
	return 0;	
}
//---------------------------------------------------------------
// Remove coin from owner wallet
//---------------------------------------------------------------
unsigned char remove_upgrade_coin_ticket(uint32_t ticket){
	unsigned int i =0;
	for(i=0;i<COIN_UPGRADE_TICKETS_MAX;i++){
		if(upgrade_coin_tickets_obj[i].ticket == ticket){
			upgrade_coin_tickets_obj[i].ticket = 0;
			upgrade_coin_tickets_obj[i].value = 0;
			return 1;
		}
	}
	return 0;
}

//---------------------------------------------------------------
// add coin to owner wallet
//---------------------------------------------------------------
unsigned char add_coin_owner(uint32_t serial_no,uint32_t owner_id,unsigned char coin_type, unsigned char changed){
	unsigned int i=0;	
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_obj[i].owner_id == 0){
			coin_owner_obj[i].serial_no = serial_no;
			coin_owner_obj[i].owner_id = owner_id;
			coin_owner_obj[i].coin_type = coin_type;
			coin_owner_obj[i].changed = changed;
			printf("\n coin owner added %d \n",coin_owner_obj[i].owner_id);
			return 1;
		}
	}
	return 0;	
}
//---------------------------------------------------------------
// Remove coin from owner wallet
//---------------------------------------------------------------
unsigned char remove_coin_owner(uint32_t serial_no,uint32_t owner_id,unsigned char changed){
	unsigned int i =0;
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_obj[i].serial_no == serial_no && coin_owner_obj[i].owner_id == owner_id){
			coin_owner_obj[i].serial_no = 0;
			coin_owner_obj[i].owner_id = 0;
			coin_owner_obj[i].coin_type = 0;
			coin_owner_obj[i].changed = changed;
			return 1;
		}
	}
	return 0;
}
//---------------------------------------------------------------
// add details to coin owner wallet
//---------------------------------------------------------------
unsigned char add_coin_owner_details(uint32_t owner_id,unsigned char *guid,unsigned char *time_stamp,unsigned char trans_type,
									     unsigned char *meta_data,unsigned int amt,unsigned int balance,unsigned char changed){
	unsigned int i=0;
	for(i=0;i<COIN_OWNER_DETAILS_MAX;i++){
		if(coin_owner_details_obj[i].owner_id == 0){
			coin_owner_details_obj[i].owner_id= owner_id;
			memcpy(coin_owner_details_obj[i].guid,guid,GUID_BYTES_CNT);
			memcpy(coin_owner_details_obj[i].time_stamp,time_stamp,TIME_STAMP_BYTES_CNT);
			coin_owner_details_obj[i].trans_type=trans_type;
			if(meta_data == NULL){
				memset(coin_owner_details_obj[i].meta_data,0,META_DATA_BYTES_CNT);
			}
			else{
				memcpy(coin_owner_details_obj[i].meta_data,meta_data,META_DATA_BYTES_CNT);
			}
			coin_owner_details_obj[i].amt=amt;
			coin_owner_details_obj[i].balance=balance;
			coin_owner_details_obj[i].changed=changed;
			return 1;
		}
	}
	return 0;
}
//---------------------------------------------------------------
// remove details from coin owner wallet
//---------------------------------------------------------------
unsigned char remove_coin_owner_details(uint32_t owner_id, unsigned char changed){
	unsigned int i=0;
	for(i=0;i<COIN_OWNER_DETAILS_MAX;i++){
		if(coin_owner_details_obj[i].owner_id == owner_id){
			coin_owner_details_obj[i].owner_id= 0;
			coin_owner_details_obj[i].changed=changed;
		}
	}
	return 1;
}
//---------------------------------------------------------------
// get balance details to coin owner wallet
//---------------------------------------------------------------
uint32_t get_owner_balance(uint32_t owner_id){
	unsigned int i =0;
	uint32_t bal=0;
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_obj[i].owner_id == owner_id){
			bal+=get_denomination(coin_owner_obj[i].serial_no); 
		}
	}
	return bal;
}

//---------------------------------------------------------------
//  get denomination
//---------------------------------------------------------------
unsigned long get_denomination(unsigned int  srno){
	if(server_config_obj.deno_type == DENO_TYPE_ALL){
		if(srno>= DENO_1_MIN	&&  srno<=DENO_1_MAX){
			return 1;
		}else if(srno>= DENO_5_MIN	&&  srno<=DENO_5_MAX){
			return 5;
		}else if(srno>= DENO_25_MIN &&  srno<=DENO_25_MAX){
			return 25;
		}else if(srno>= DENO_100_MIN &&  srno<=DENO_100_MAX){
			return 100;
		}else if(srno>= DENO_250_MIN  &&  srno<=DENO_250_MAX){
			return 250;
		}
	}
	if(server_config_obj.deno_type == DENO_TYPE_SINGLE){
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------
//  get legacy denomination
//---------------------------------------------------------------
unsigned long get_legacy_denomination(unsigned int  srno){
	if(srno>= DENO_1_MIN	&&  srno<=DENO_1_MAX){
		return 1;
	}else if(srno>= DENO_5_MIN	&&  srno<=DENO_5_MAX){
		return 5;
	}else if(srno>= DENO_25_MIN &&  srno<=DENO_25_MAX){
		return 25;
	}else if(srno>= DENO_100_MIN &&  srno<=DENO_100_MAX){
		return 100;
	}else if(srno>= DENO_250_MIN  &&  srno<=DENO_250_MAX){
		return 250;
	}
	return 0;
}

//---------------------------------------------------------------
//  get denomination index
//---------------------------------------------------------------
long get_denomination_index(unsigned int  deno){
	if(server_config_obj.deno_type == DENO_TYPE_ALL){
		if(deno == DENO_1_VAL){
			return 0;
		}else 	if(deno == DENO_5_VAL){
			return 1;
		}else 	if(deno == DENO_25_VAL){
			return 2;
		}else 	if(deno == DENO_100_VAL){
			return 3;
		}else 	if(deno == DENO_250_VAL){
			return 4;
		}
	}
	if(server_config_obj.deno_type == DENO_TYPE_SINGLE){
		return 0;
	}
	return -1;
}
//---------------------------------------------------------------
//  get legacy denomination index
//---------------------------------------------------------------
long get_legacy_denomination_index(unsigned int  deno){

	if(deno == DENO_1_VAL){
		return 0;
	}else 	if(deno == DENO_5_VAL){
		return 1;
	}else 	if(deno == DENO_25_VAL){
		return 2;
	}else 	if(deno == DENO_100_VAL){
		return 3;
	}else 	if(deno == DENO_250_VAL){
		return 4;
	}
	return -1;
}

//---------------------------------------------------------------
//  comparision funtion for q sort
//---------------------------------------------------------------
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
//----------------------------------------------------------------
void display(){
	int i=0,j=0;
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_obj[i].owner_id==0){
			continue;
		}
		printf("Serial number %d \t",coin_owner_obj[i].serial_no);
		printf("Owner id %d \t",coin_owner_obj[i].owner_id);
		printf("\n");
	}
	printf("Details \n");
	for(i=0;i<COIN_OWNER_DETAILS_MAX;i++){
		if(coin_owner_details_obj[i].owner_id==0){
			continue;
		}
		printf("GUID " );
		for(j=0;j<16;j++)
			printf("%d ,",coin_owner_details_obj[i].guid[j]);
		printf("\t");
		printf("TIME STAMP : " );
		for(j=0;j<6;j++)
			printf("%d ,",coin_owner_details_obj[i].time_stamp[j]);
		printf("\t Trans Type: %d ", coin_owner_details_obj[i].trans_type);
		printf("META DATA " );
		for(j=0;j<50;j++)
			printf("%d",coin_owner_details_obj[i].meta_data[j]);
		printf("\n");
		printf("Amt : %d \n", coin_owner_details_obj[i].amt);
	}
}
//---------------------------------------------------------------
// DEPOSITE COMMAND  
//---------------------------------------------------------------
void execute_deposite(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT+OWNER_ID_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT+ COIN_TYPE_BYTES_CNT +
	TY_BYTES_CNT+META_DATA_BYTES_CNT+CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT+AN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0,pass_cnt=0,fail_cnt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0};
	uint32_t	   owner_id,amt=0;
	unsigned char guid[ GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT],coin_type,meta_data[META_DATA_BYTES_CNT];
	printf("DEPOSITE COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT;
	coin_type = udp_buffer[index];
	if(owner_id == PUBLIC_CHANGE_ID){
		coin_type = COIN_TYPE_PUBLIC;
	}
	for(i=0;i<no_of_coins;i++) {
		index = (i * bytes_per_coin) + req_header_min+CH_BYTES_CNT;
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		index +=SN_BYTES_CNT;
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		pass_fail[i]=1;
		if(memcmp(coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32],&udp_buffer[index],AN_BYTES_CNT)!=0){
			pass_fail[i]=0;
			fail_cnt++;
		}
		if(pass_fail[i]==1){
			add_coin_owner(snObj.val32,owner_id,coin_type,1);
			unsigned char k,upper=255,lower=1;
			printf("After change : ");
			for (k=0; k<AN_BYTES_CNT;k++) {
			        unsigned char num = (rand() % (upper - lower + 1)) + lower;
			        printf("%d, ", num);
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][k]=num;
    			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
			amt+=get_denomination(snObj.val32);		
			printf("\n");
			pass_cnt++;	
		}			
	}
	index= (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT ;	
	memcpy(guid,&udp_buffer[index],GUID_BYTES_CNT);
	index+=GUID_BYTES_CNT;
	memcpy(time_stamp,&udp_buffer[index],TIME_STAMP_BYTES_CNT);
	index+=TIME_STAMP_BYTES_CNT;
	index++;
	memcpy(meta_data,&udp_buffer[index],META_DATA_BYTES_CNT);
	if(amt>0){
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_DEPOSITE,meta_data,amt,get_owner_balance(owner_id),1);
		printf("%d\n",amt);
	}
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//---------------------------------------------------------------
// DEPOSITE PANG COMMAND  
//---------------------------------------------------------------
void execute_deposite_pang(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT+ PG_BYTES_CNT + OWNER_ID_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT+ COIN_TYPE_BYTES_CNT +
	TY_BYTES_CNT+META_DATA_BYTES_CNT+CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0,pass_cnt=0,fail_cnt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0},md_input[64],md_output[64],tmp[64],pg[PG_BYTES_CNT];
	uint32_t	   owner_id,amt=0;
	unsigned char guid[ GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT],coin_type,meta_data[META_DATA_BYTES_CNT];
	printf("DEPOSITE COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index =  req_header_min+CH_BYTES_CNT;
	memcpy(pg,&udp_buffer[index],PG_BYTES_CNT);
	index = (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT + PG_BYTES_CNT ;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT;
	coin_type = udp_buffer[index];
	if(owner_id == PUBLIC_CHANGE_ID){
		coin_type = COIN_TYPE_PUBLIC;
	}
	for(i=0;i<no_of_coins;i++) {
		index = (i * bytes_per_coin) + req_header_min+CH_BYTES_CNT;
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		index +=SN_BYTES_CNT;
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		pass_fail[i]=1;
		/*if(memcmp(coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32],&udp_buffer[index],AN_BYTES_CNT)!=0){
			pass_fail[i]=0;
			fail_cnt++;
		}*/
		sprintf(md_input, "%d", udp_buffer[REQ_RI]);
		sprintf(tmp, "%d", snObj.val32);
		strcat(md_input,tmp);
		for(j=0;j<PG_BYTES_CNT;j++){		
			sprintf(tmp,"%02x", pg[j]);
		}
		strcat(md_input,tmp);
		printf("%s\n",md_input);
		md5(md_input,md_output);
		if(pass_fail[i]==1){
			add_coin_owner(snObj.val32,owner_id,coin_type,1);
			printf("After change : ");
			for (j=0; j<AN_BYTES_CNT;j++) {
			        printf("%d, ", md_output[j]);
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j]=md_output[j];
    			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
			amt+=get_denomination(snObj.val32);		
			printf("\n");
			pass_cnt++;	
		}			
	}
	index= (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT ;	
	memcpy(guid,&udp_buffer[index],GUID_BYTES_CNT);
	index+=GUID_BYTES_CNT;
	memcpy(time_stamp,&udp_buffer[index],TIME_STAMP_BYTES_CNT);
	index+=TIME_STAMP_BYTES_CNT;
	index++;
	memcpy(meta_data,&udp_buffer[index],META_DATA_BYTES_CNT);
	if(amt>0){
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_DEPOSITE,meta_data,amt,get_owner_balance(owner_id),1);
		printf("%d\n",amt);
	}
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//----------------------------------------------------------------------------------
//TRANSFER COMMAND
//----------------------------------------------------------------------------------
void execute_transfer(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT+OWNER_ID_BYTES_CNT+AN_BYTES_CNT+OWNER_ID_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT+TY_BYTES_CNT+META_DATA_BYTES_CNT+CMD_END_BYTES_CNT ;
	int bytes_per_coin = SN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0,pass_cnt=0,fail_cnt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0},input_an[AN_BYTES_CNT];
	uint32_t	   owner_id=0,new_owner_id=0,amt=0;
	unsigned char guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT],coin_type,raida_type,meta_data[META_DATA_BYTES_CNT];
	printf("TRANSFER COMMAND. \n");
	printf("%d\n",req_body_without_coins);
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index =req_header_min + (no_of_coins * bytes_per_coin) +CH_BYTES_CNT+OWNER_ID_BYTES_CNT+AN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	new_owner_id=snObj.val32;
	if(new_owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;
		send_response(status_code,RES_HS+HS_BYTES_CNT);
		return;
	}
	for(i=0;i<no_of_coins;i++) {
		index = (i * bytes_per_coin) + req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT+AN_BYTES_CNT;
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		pass_fail[i]=1;
		if(remove_coin_owner(snObj.val32,owner_id,1)==0){
			pass_fail[i] =0;		//coin not found
			fail_cnt++;
		}	
		if(pass_fail[i]==1){
			if(new_owner_id == PUBLIC_CHANGE_ID){
				add_coin_owner(snObj.val32,new_owner_id,COIN_TYPE_PUBLIC,1);
			}else{
				add_coin_owner(snObj.val32,new_owner_id,ANONYMOUS,1);
			}
			unsigned char k,upper=255,lower=1;
			printf("After change : ");
			for (k=0; k<AN_BYTES_CNT;k++) {
			        unsigned char num = (rand() % (upper - lower + 1)) + lower;
			        printf("%d, ", num);
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][k]=num;
    			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
			amt+=get_denomination(snObj.val32);		
			printf("\n");
			pass_cnt++;	
		}			
	}
	index= (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT +AN_BYTES_CNT +  OWNER_ID_BYTES_CNT ;	
	memcpy(guid,&udp_buffer[index],GUID_BYTES_CNT);
	index+=GUID_BYTES_CNT;
	memcpy(time_stamp,&udp_buffer[index],TIME_STAMP_BYTES_CNT);
	index+=TIME_STAMP_BYTES_CNT;
	coin_type = udp_buffer[index];							
	index++;							
	memcpy(meta_data,&udp_buffer[index],META_DATA_BYTES_CNT);
	if(amt>0){
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_TRANSFER_OUT,meta_data,amt,get_owner_balance(owner_id),1);
		add_coin_owner_details(new_owner_id,guid,time_stamp,TRANS_TRANSFER_IN,meta_data,amt,get_owner_balance(new_owner_id),1);
	}
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index =   RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
	display();
}
//----------------------------------------------------------------------------------
//SHOW REGISTRY COMMAND
//----------------------------------------------------------------------------------
void execute_show_registry(unsigned int packet_len){
	unsigned int  req_body=CH_BYTES_CNT  + OWNER_ID_BYTES_CNT +  AN_BYTES_CNT + DENOM_BYTES_CNT +  CMD_END_BYTES_CNT;
	int req_header_min;
	unsigned int i=0,index=0,j=0,size;
	unsigned char status_code,input_an[AN_BYTES_CNT],deno=0;
	uint32_t	   owner_id=0,sr_no[COINS_MAX],sr_no_cnt;

	printf("SHOW REGISTRY COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index+=OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	index+= AN_BYTES_CNT;
	deno=udp_buffer[index];
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[snObj.val32],&input_an,AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	status_code=SUCCESS;
	if(status_code != FAILED_TO_AUTHENTICATE){
		sr_no_cnt =0;
		for(i=0;i<COIN_OWNER_MAX;i++){
			if(coin_owner_obj[i].owner_id==0){
				continue;
			}
			if(coin_owner_obj[i].owner_id == owner_id && get_denomination(coin_owner_obj[i].serial_no)<deno){
				printf("Serial number %d\n",coin_owner_obj[i].serial_no);
				printf("Owner id %d\n",coin_owner_obj[i].owner_id);
				sr_no[sr_no_cnt]=coin_owner_obj[i].serial_no;
				sr_no_cnt++;
				status_code = SUCCESS;
			}
		}
		qsort(sr_no,sr_no_cnt,sizeof(uint32_t),cmpfunc);
		unsigned char prev_denom=1,curr_denom=1;
		unsigned int show_coins_cnt =0;
		for(i=0;i<sr_no_cnt;i++){
			snObj.val32 = sr_no[i];
			curr_denom = get_denomination(snObj.val32);
			show_coins_cnt ++;
			if (show_coins_cnt >server_config_obj.show_regs_coins_max){
				if (prev_denom==curr_denom){
					prev_denom=curr_denom;
					continue;
				}		
			}
			if (prev_denom!=curr_denom){
				show_coins_cnt =0;	
			}
			prev_denom = curr_denom;
			for(j=0;j<SN_BYTES_CNT;j++) {
				response[index+j]=snObj.data[SN_BYTES_CNT-1-j];
			}
			index+=SN_BYTES_CNT;
			size+=SN_BYTES_CNT;
		}
	}
	send_response(status_code,size);
}
//----------------------------------------------------------------------------------
//SHOW COINS BY DENOMINATIONS COMMAND
//----------------------------------------------------------------------------------
void execute_show_by_denom(unsigned int packet_len){
	unsigned int  req_body=CH_BYTES_CNT  +  CMD_END_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT +  DENOM_BYTES_CNT + PAGE_NO_BYTES_CNT;
	int req_header_min;
	unsigned int i=0,index=0,j=0,size;
	unsigned char status_code,input_an[AN_BYTES_CNT],denom,page_no,no_of_pages;
	uint32_t	   owner_id=0,amt=0,sr_no[COINS_MAX],sr_no_cnt;
	printf("SHOW COINS BY DENOMINATIONS COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT+AN_BYTES_CNT;
	denom = udp_buffer[index];
	if(server_config_obj.deno_type == DENO_TYPE_SINGLE){
		denom = 1;
	}
	index++;
	page_no = udp_buffer[index];
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&input_an,AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;									
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	status_code=SUCCESS;
	if(status_code != FAILED_TO_AUTHENTICATE){
		sr_no_cnt =0;
		for(i=0;i<COIN_OWNER_MAX;i++){
			if(coin_owner_obj[i].owner_id == owner_id && get_denomination(coin_owner_obj[i].serial_no)==denom){
				printf("Serial number %d\n",coin_owner_obj[i].serial_no);
				printf("Owner id %d\n",coin_owner_obj[i].owner_id);
				sr_no[sr_no_cnt]=coin_owner_obj[i].serial_no;
				sr_no_cnt++;
				status_code = SUCCESS;
			}
		}
		qsort(sr_no,sr_no_cnt,sizeof(uint32_t),cmpfunc);
		no_of_pages =sr_no_cnt/server_config_obj.show_denom_coins_max;
		if((sr_no_cnt % server_config_obj.show_denom_coins_max)!=0)
			no_of_pages ++;	
		if(page_no >= no_of_pages){
			status_code=PAGE_NOT_FOUND;
		}else{	
			unsigned int start_index  = page_no * server_config_obj.show_denom_coins_max, cntr =0 ;
			for(i=start_index;i<sr_no_cnt;i++){
				snObj.val32 = sr_no[i];
				for(j=0;j<SN_BYTES_CNT;j++) {
					response[index+j]=snObj.data[SN_BYTES_CNT-1-j];
				}
				index+=SN_BYTES_CNT;
				size+=SN_BYTES_CNT;
				cntr++;
				if(cntr>server_config_obj.show_denom_coins_max){
					break;
				}
			}
		}
	}
	send_response(status_code,size);
}
//----------------------------------------------------------------------------------
//SHOW COINS BY TYPE COMMAND
//----------------------------------------------------------------------------------
void execute_show_by_coin_type(unsigned int packet_len){
	unsigned int  req_body=CH_BYTES_CNT  + OWNER_ID_BYTES_CNT + AN_BYTES_CNT +  COIN_TYPE_BYTES_CNT + PAGE_NO_BYTES_CNT + CMD_END_BYTES_CNT;
	int req_header_min;
	unsigned int i=0,index=0,j=0,size;
	unsigned char status_code,input_an[AN_BYTES_CNT],coin_type,page_no,no_of_pages;
	uint32_t	   owner_id=0,amt=0,sr_no[COINS_MAX],sr_no_cnt;
	printf("SHOW COINS BY TYPE COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT+AN_BYTES_CNT;
	coin_type = udp_buffer[index];
	index++;
	page_no = udp_buffer[index];
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&input_an,AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;									
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	status_code=SUCCESS;
	if(status_code != FAILED_TO_AUTHENTICATE){
		sr_no_cnt =0;
		for(i=0;i<COIN_OWNER_MAX;i++){
			if(coin_owner_obj[i].owner_id == owner_id && coin_owner_obj[i].coin_type == coin_type){
				printf("Serial number %d\n",coin_owner_obj[i].serial_no);
				printf("Owner id %d\n",coin_owner_obj[i].owner_id);
				sr_no[sr_no_cnt]=coin_owner_obj[i].serial_no;
				sr_no_cnt++;
				status_code = SUCCESS;
			}
		}
		qsort(sr_no,sr_no_cnt,sizeof(uint32_t),cmpfunc);
		no_of_pages =sr_no_cnt/server_config_obj.show_denom_coins_max;
		if((sr_no_cnt % server_config_obj.show_denom_coins_max)!=0)
			no_of_pages ++;	
		if(page_no >= no_of_pages){
			status_code=PAGE_NOT_FOUND;
		}else{	
			unsigned int start_index  = page_no * server_config_obj.show_denom_coins_max, cntr =0 ;
			for(i=start_index;i<sr_no_cnt;i++){
				snObj.val32 = sr_no[i];
				for(j=0;j<SN_BYTES_CNT;j++) {
					response[index+j]=snObj.data[SN_BYTES_CNT-1-j];
				}
				index+=SN_BYTES_CNT;
				size+=SN_BYTES_CNT;
				cntr++;
				if(cntr>server_config_obj.show_denom_coins_max){
					break;
				}
			}
		}
	}
	send_response(status_code,size);
}

//----------------------------------------------------------------------------------
//SHOW CHANGE  COMMAND
//----------------------------------------------------------------------------------
void execute_show_change(unsigned int packet_len){
	unsigned int req_body=CH_BYTES_CNT + OWNER_ID_BYTES_CNT + DENOM_BYTES_CNT + CMD_END_BYTES_CNT;
	int req_header_min;
	unsigned int i=0,index=0,j=0,size;
	unsigned char status_code,input_an[AN_BYTES_CNT],denom,page_no,no_of_pages;
	uint32_t	   owner_id=0,amt=0,sr_no[COINS_MAX],sr_no_cnt;
	printf("SHOW  CHANGE COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index = req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	denom = udp_buffer[index];
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;

	status_code=SUCCESS;
	sr_no_cnt =0;
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_obj[i].owner_id == owner_id && get_denomination(coin_owner_obj[i].serial_no)<denom){
			printf("Serial number %d\n",coin_owner_obj[i].serial_no);
			printf("Owner id %d\n",coin_owner_obj[i].owner_id);
			sr_no[sr_no_cnt]=coin_owner_obj[i].serial_no;
			sr_no_cnt++;
			status_code = SUCCESS;
		}
	}
	qsort(sr_no,sr_no_cnt,sizeof(uint32_t),cmpfunc);
	unsigned char prev_denom=1,curr_denom=1;
	unsigned int show_coins_cnt =0;
	for(i=0;i<sr_no_cnt;i++){
		snObj.val32 = sr_no[i];
		curr_denom = get_denomination(snObj.val32);
		show_coins_cnt ++;
		if (show_coins_cnt >server_config_obj.show_regs_coins_max){
			if (prev_denom==curr_denom){
				prev_denom=curr_denom;
				continue;
			}		
		}
		if (prev_denom!=curr_denom){
			show_coins_cnt =0;	
		}
		prev_denom = curr_denom;
		for(j=0;j<SN_BYTES_CNT;j++) {
			response[index+j]=snObj.data[SN_BYTES_CNT-1-j];
		}
		index+=SN_BYTES_CNT;
		size+=SN_BYTES_CNT;
	}
	send_response(status_code,size);
}
//----------------------------------------------------------------------------------
//SHOW BALANCE COMMAND
//----------------------------------------------------------------------------------
void execute_show_balance(unsigned int packet_len){
	unsigned int  req_body=CH_BYTES_CNT+ OWNER_ID_BYTES_CNT  + AN_BYTES_CNT + CMD_END_BYTES_CNT;
	int req_header_min;
	unsigned int index=0,size,denoCnt[DENO_CNT]={0};
	unsigned char status_code = FAIL,input_an[AN_BYTES_CNT];
	uint32_t	   owner_id=0,amt=0;
	int i=0,j=0,k=0;
	printf("SHOW BALANCE COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(i=0;i<OWNER_ID_BYTES_CNT;i++)
		snObj.data[i]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-i)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],input_an,AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;
	}
	index =   RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code != FAILED_TO_AUTHENTICATE){
		int deno=0;
		amt =0;
		for(i=0;i<COIN_OWNER_MAX;i++){
			if(coin_owner_obj[i].owner_id == owner_id ){
				printf("Serial number %d\n",coin_owner_obj[i].serial_no);
				printf("Owner id %d\n",coin_owner_obj[i].owner_id);
				deno = get_denomination(coin_owner_obj[i].serial_no);
				amt+= deno;
				if(server_config_obj.deno_type == DENO_TYPE_ALL){
					denoCnt[get_denomination_index(deno)]++;
				}
			}
		}
		snObj.val32=amt;
		for(i=0;i<AMT_BYTES_CNT;i++) {
			response[index+i]=snObj.data[AMT_BYTES_CNT-1-i];
		}
		size+=AMT_BYTES_CNT;
		index+= AMT_BYTES_CNT;
		if(server_config_obj.deno_type == DENO_TYPE_ALL){
			for(i=DENO_CNT-1;i>-1;i--){
				snObj.val32 =denoCnt[i];
				for(k=0;k<SN_BYTES_CNT;k++) {
					response[index+k]=snObj.data[SN_BYTES_CNT-1-k];
				}
				index+=SN_BYTES_CNT;
				size+=SN_BYTES_CNT;
			
			}
		}
	}
	send_response(SUCCESS,size);
}
//---------------------------------------------------------------
// WITHDRAW COMMAND  
//---------------------------------------------------------------
void execute_withdraw(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT  + OWNER_ID_BYTES_CNT+ AN_BYTES_CNT+ PG_BYTES_CNT+GUID_BYTES_CNT+TIME_STAMP_BYTES_CNT+TY_BYTES_CNT+META_DATA_BYTES_CNT+ CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0,pass_cnt=0,fail_cnt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0},pg[PG_BYTES_CNT],input_an[AN_BYTES_CNT],md_input[64],md_output[64],tmp[64];
	unsigned char guid[ GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT],coin_type,raida_type,meta_data[META_DATA_BYTES_CNT];
	uint32_t owner_id,amt=0;
	printf("WITHDRAW COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index =req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&input_an,AN_BYTES_CNT)!=0){
		status_code = FAILED_TO_AUTHENTICATE;
		send_response(status_code,RES_HS+HS_BYTES_CNT);
		return;
	}
	index = (no_of_coins * bytes_per_coin) + req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT;
	index += AN_BYTES_CNT;
	memcpy(pg,&udp_buffer[index],PG_BYTES_CNT);
	index += PG_BYTES_CNT;
	memcpy(guid,&udp_buffer[index],GUID_BYTES_CNT);
	index+=GUID_BYTES_CNT;
	for(j=0;j<TIME_STAMP_BYTES_CNT;j++)
		time_stamp[j]=udp_buffer[index+j];
	index+=TIME_STAMP_BYTES_CNT;
	memcpy(meta_data,&udp_buffer[index],META_DATA_BYTES_CNT);
	index+=META_DATA_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		index = (i * bytes_per_coin) + req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT;
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		
		sprintf(md_input, "%d", udp_buffer[REQ_RI]);
		sprintf(tmp, "%d", snObj.val32);
		strcat(md_input,tmp);
		for(j=0;j<PG_BYTES_CNT;j++){		
			sprintf(tmp,"%02x", pg[j]);
			strcat(md_input,tmp);
		}
		printf("%s\n",md_input);
		md5(md_input,md_output);
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == snObj.val32){
				printf("Serial number %d\n",coin_owner_obj[i].serial_no);
				printf("Owner id %d\n",coin_owner_obj[i].owner_id);
				amt+=get_denomination(coin_owner_obj[i].serial_no);
				pass_fail[i]=1;
				pass_cnt++;
				remove_coin_owner(snObj.val32,owner_id,1);
				break;
			}
		}
		if(pass_fail[i]==0){
			fail_cnt++;
		}else{
			
			for(j=0;j<AN_BYTES_CNT;j++){
				printf("%d,",md_output[j]);
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j] = (unsigned char)md_output[j];
			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
				
		}	
		index +=SN_BYTES_CNT;
	}
	
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	if(status_code != ALL_FAIL){
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_WITHDRAW,meta_data,amt,get_owner_balance(owner_id),1);	
	}
	index =   RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//---------------------------------------------------------------
// BREAK COMMAND  
//---------------------------------------------------------------
void execute_break(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  SN_BYTES_CNT + AN_BYTES_CNT + OWNER_ID_BYTES_CNT + CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0;
	unsigned char status_code,pass_fail,guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT];
	unsigned int sr_no=0,owner_id=0,amt=0;
	printf("BREAK COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<SN_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
	sr_no=snObj.val32;
	if(sr_no >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	if(get_denomination(sr_no)==1){
		send_err_resp_header(BREAK_CANNOT_BREAK);
		return;
	}

	index += SN_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_CLOUD_INDEX].AN[sr_no],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(BREAK_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index +=  AN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	index = req_header_min+CH_BYTES_CNT +  SN_BYTES_CNT + AN_BYTES_CNT + OWNER_ID_BYTES_CNT ;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		printf("Serial number %d \n", snObj.val32);
		pass_fail=0;
		index+=SN_BYTES_CNT;
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == snObj.val32){
				printf("Serial number %d\n",coin_owner_obj[j].serial_no);
				printf("Owner id %d\n",coin_owner_obj[j].owner_id);
				amt+=get_denomination(coin_owner_obj[j].serial_no);
				pass_fail=1;
				break;
			}
		}
		if(pass_fail==0){
			send_response(BREAK_COINS_NOT_FOUND,RES_HS+HS_BYTES_CNT);
			return;
		}
		index += AN_BYTES_CNT;
	}
	printf("Amt is = %d",amt);
	if(get_denomination(sr_no) != amt){
		send_response(BREAK_COINS_SUM_NOT_MATCH,RES_HS+HS_BYTES_CNT);
		return;	
	}else{
		index = req_header_min+CH_BYTES_CNT +  SN_BYTES_CNT + AN_BYTES_CNT + OWNER_ID_BYTES_CNT ;
		for(i=0;i<no_of_coins;i++) {
			memset(snObj.data,0,4);
			for(j=0;j<SN_BYTES_CNT;j++)
				snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
			remove_coin_owner(snObj.val32,owner_id,1);
			index+=SN_BYTES_CNT;
			for (j=0; j<AN_BYTES_CNT;j++) {
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j]= udp_buffer[index+j];
    			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
			index += AN_BYTES_CNT;
		}
		int res=add_coin_owner(sr_no,owner_id,ANONYMOUS,1);
		unsigned char upper=255,lower=1;
		for (i=0; i<AN_BYTES_CNT;i++) {
			coin_id_obj[COIN_CLOUD_INDEX].AN[sr_no][i]=(rand() % (upper - lower + 1)) + lower;
    		}
		for (i=0; i<GUID_BYTES_CNT;i++) {
			guid[i] = (rand() % (upper - lower + 1)) + lower;
    		}
		unsigned int timestamp = time(NULL);
		sprintf((char*)time_stamp,"%d",timestamp);
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_BREAK,NULL,0,get_owner_balance(owner_id),1);
	}
	send_response(SUCCESS,RES_HS+HS_BYTES_CNT);
}
//---------------------------------------------------------------
// BREAK IN BANK COMMAND  
//---------------------------------------------------------------
void execute_break_in_bank(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT + CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT;
	int req_header_min, no_of_coins,size;
	unsigned int i=0,index=0,j=0;
	unsigned char status_code,pass_fail,guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT];
	unsigned int sr_no=0,owner_id=0,public_owner_id=0,amt=0;
	printf("BREAK IN BANK COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id = snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(BREAK_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index +=  AN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<SN_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
	sr_no = snObj.val32;
	if(sr_no >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	if(get_denomination(sr_no)==1){
		send_err_resp_header(BREAK_CANNOT_BREAK);
		return;
	}
	index +=  SN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	public_owner_id = snObj.val32;
	if(public_owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_OWNER_ID_NOT_FOUND);
		return;
	}
	for(j=0;j<COIN_OWNER_MAX;j++){
		if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == sr_no){
			break;
		}
	}
	if(j>=COIN_OWNER_MAX){
		send_response(BREAK_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index = req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		printf("Serial number %d \n", snObj.val32);
		pass_fail=0;
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == public_owner_id  && coin_owner_obj[j].serial_no == snObj.val32){
				printf("Serial number %d\n",coin_owner_obj[j].serial_no);
				printf("Owner id %d\n",coin_owner_obj[j].owner_id);
				amt+=get_denomination(coin_owner_obj[j].serial_no);
				pass_fail=1;
				break;
			}
		}
		if(pass_fail==0){
			send_response(BREAK_COINS_NOT_FOUND,RES_HS+HS_BYTES_CNT);
			return;
		}
		index += bytes_per_coin;
	}
	printf("Amt is = %d",amt);
	if(get_denomination(sr_no) != amt){
		send_response(BREAK_COINS_SUM_NOT_MATCH,RES_HS+HS_BYTES_CNT);
		return;	
	}else{
		unsigned char upper=255,lower=1;
		index = req_header_min+CH_BYTES_CNT +  SN_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT ;
		for(i=0;i<no_of_coins;i++) {
			memset(snObj.data,0,4);
			for(j=0;j<SN_BYTES_CNT;j++)
				snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
			remove_coin_owner(snObj.val32,public_owner_id,1);
			for (j=0; j<AN_BYTES_CNT;j++) {
				//coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j]=(rand() % (upper - lower + 1)) + lower;
    			}
			index += bytes_per_coin;
			add_coin_owner(snObj.val32,owner_id,COIN_TYPE_PUBLIC,1);
		}
		remove_coin_owner(sr_no,owner_id,1);
		for (i=0; i<AN_BYTES_CNT;i++) {
			//coin_id_obj[COIN_CLOUD_INDEX].AN[sr_no][i]=(rand() % (upper - lower + 1)) + lower;
    		}
		for (i=0; i<GUID_BYTES_CNT;i++) {
			guid[i] = (rand() % (upper - lower + 1)) + lower;
    		}
		unsigned int timestamp = time(NULL);
		sprintf((char*)time_stamp,"%d",timestamp);
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_BREAK,NULL,0,get_owner_balance(owner_id),1);
		for (i=0; i<GUID_BYTES_CNT;i++) {
			guid[i] = (rand() % (upper - lower + 1)) + lower;
    		}
		add_coin_owner_details(public_owner_id,guid,time_stamp,TRANS_BREAK,NULL,0,get_owner_balance(public_owner_id),1);
	}
	send_response(SUCCESS,RES_HS+HS_BYTES_CNT);
}
//---------------------------------------------------------------
// JOIN COMMAND  
//---------------------------------------------------------------
void execute_join(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + SN_BYTES_CNT + PAN_BYTES_CNT + CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;
	unsigned int i=0,index=0,j=0;
	int req_header_min, no_of_coins,size;
	uint32_t	  sr_no=0,owner_id=0,amt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0},input_an[AN_BYTES_CNT],input_pan[PAN_BYTES_CNT],guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT];
	printf("JOIN COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index +=OWNER_ID_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<SN_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
	sr_no=snObj.val32;
	if(sr_no >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	for(j=0;j<COIN_OWNER_MAX;j++){
		if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == sr_no){
			break;
		}
	}
	if(j>=COIN_OWNER_MAX){
		send_err_resp_header(JOIN_COUNTER_FEIT);
		return;
	}	
	index += SN_BYTES_CNT;
	memcpy(input_pan,&udp_buffer[index],PAN_BYTES_CNT);
	index += PAN_BYTES_CNT;

	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		index+=SN_BYTES_CNT;
		if(memcmp(coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32],&udp_buffer[index],AN_BYTES_CNT)!=0){
			send_response(JOIN_COINS_NOT_FOUND,RES_HS+HS_BYTES_CNT);			
			return;
		}
		amt+=get_denomination(snObj.val32);
		index += AN_BYTES_CNT;
	}
	printf("Amt = %d",amt);
	if(get_denomination(sr_no) != amt){
		send_response(JOIN_COINS_SUM_NOT_MATCH,RES_HS+HS_BYTES_CNT);
		return;	
	}else{
		unsigned char upper=255,lower=1;
		index = req_header_min+CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + SN_BYTES_CNT +  PAN_BYTES_CNT ;
		for(i=0;i<no_of_coins;i++) {
			memset(snObj.data,0,4);
			for(j=0;j<SN_BYTES_CNT;j++)
				snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
			add_coin_owner(snObj.val32,owner_id,ANONYMOUS,1);
			for (j=0; j<AN_BYTES_CNT;j++) {
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j]=(rand() % (upper - lower + 1)) + lower;
    			}
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
			index += bytes_per_coin;
		}
		remove_coin_owner(sr_no,owner_id,1);
		for (i=0; i<AN_BYTES_CNT;i++) {
			coin_id_obj[COIN_CLOUD_INDEX].AN[sr_no][i]=input_pan[i];// (rand() % (upper - lower + 1)) + lower;
    		}
		for (i=0; i<GUID_BYTES_CNT;i++) {
			guid[i] =  (rand() % (upper - lower + 1)) + lower;
    		}
		uint32_t timestamp = time(NULL);
		sprintf((char*)time_stamp,"%d",timestamp);
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_JOIN,NULL,0,get_owner_balance(owner_id),1);
	}
	send_response(SUCCESS,RES_HS+HS_BYTES_CNT);
}
//---------------------------------------------------------------
// JOIN IN BANK COMMAND  
//---------------------------------------------------------------
void execute_join_in_bank(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT + CMD_END_BYTES_CNT;
	int bytes_per_coin = SN_BYTES_CNT;
	unsigned int i=0,index=0,j=0;
	int req_header_min, no_of_coins,size;
	uint32_t sr_no=0,owner_id=0,public_owner_id=0,amt=0;
	unsigned char status_code,pass_fail[COINS_MAX]={0},input_an[PAN_BYTES_CNT],guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT];
	printf("JOIN IN BANK COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id = snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_response(JOIN_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(JOIN_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index +=  AN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<SN_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
	sr_no = snObj.val32;
	if(get_denomination(sr_no)==1){
		send_err_resp_header(JOIN_CANNOT_JOIN);
		return;
	}
	if(sr_no >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
		send_err_resp_header(JOIN_COUNTER_FEIT);
		return;
	}
	index +=  SN_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	public_owner_id=snObj.val32;
	if(public_owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(JOIN_COUNTER_FEIT);
		return;
	}
	for(j=0;j<COIN_OWNER_MAX;j++){
		if(coin_owner_obj[j].owner_id == public_owner_id  && coin_owner_obj[j].serial_no == sr_no){
			break;
		}
	}
	if(j>=COIN_OWNER_MAX){
		send_response(JOIN_COUNTER_FEIT,RES_HS+HS_BYTES_CNT);
		return;
	}
	index = req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == snObj.val32){
				break;
			}
		}
		if(j>=COIN_OWNER_MAX){
			send_err_resp_header(JOIN_COUNTER_FEIT);
			return;
		}
		index+=SN_BYTES_CNT;
		amt+=get_denomination(snObj.val32);
	}
	printf("Amt = %d",amt);
	if(get_denomination(sr_no) != amt){
		send_response(JOIN_COINS_SUM_NOT_MATCH,RES_HS+HS_BYTES_CNT);
		return;	
	}else{
		unsigned char upper=255,lower=1;
		index = req_header_min+CH_BYTES_CNT + OWNER_ID_BYTES_CNT + AN_BYTES_CNT + SN_BYTES_CNT + OWNER_ID_BYTES_CNT;
		for(i=0;i<no_of_coins;i++) {
			memset(snObj.data,0,4);
			for(j=0;j<SN_BYTES_CNT;j++)
				snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
			add_coin_owner(snObj.val32,public_owner_id,COIN_TYPE_PUBLIC,1);
			remove_coin_owner(snObj.val32,owner_id,1);
			for (j=0; j<AN_BYTES_CNT;j++) {
				coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32][j]=(rand() % (upper - lower + 1)) + lower;
    			}
			index += bytes_per_coin;
		}
		remove_coin_owner(sr_no,public_owner_id,1);
		add_coin_owner(sr_no,owner_id,ANONYMOUS,1);
		for (i=0; i<AN_BYTES_CNT;i++) {
			coin_id_obj[COIN_CLOUD_INDEX].AN[sr_no][i]= (rand() % (upper - lower + 1)) + lower;
		}
		coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
		for (i=0; i<GUID_BYTES_CNT;i++) {
			guid[i] =  (rand() % (upper - lower + 1)) + lower;
    		}
		uint32_t timestamp = time(NULL);
		sprintf((char*)time_stamp,"%d",timestamp);
		add_coin_owner_details(owner_id,guid,time_stamp,TRANS_JOIN,NULL,0,get_owner_balance(owner_id),1);
	}
	send_response(SUCCESS,RES_HS+HS_BYTES_CNT);
}
//---------------------------------------------------------------
//SHOW STATEMENT COMMAND  
//---------------------------------------------------------------
void execute_show_statement(unsigned int packet_len){
	int req_body = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT +  ROWS_BYTES_CNT +  YY_BYTES_CNT + MM_BYTES_CNT + DD_BYTES_CNT+ RA_BYTES_CNT+ KEY_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,j=0, rows_cnt=0;
	int req_header_min,size;
	uint32_t	 sr_no=0,owner_id=0,amt=0,no_of_rows=0;
	unsigned char status_code,input_an[AN_BYTES_CNT],guid[GUID_BYTES_CNT],time_stamp[TIME_STAMP_BYTES_CNT];
	printf("SHOW STATEMENT COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(owner_id >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	memcpy(input_an,&udp_buffer[index],AN_BYTES_CNT);
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(FAILED_TO_AUTHENTICATE,RES_HS+HS_BYTES_CNT);	
		return;
	}
	index += AN_BYTES_CNT;
	no_of_rows = udp_buffer[index] * ROWS_MUL_FACTOR;
	struct date d1, d2;
	index += ROWS_BYTES_CNT;
	d1.year = DEFAULT_YEAR + udp_buffer[index];
	printf("year %d",d1.year);
	index += YY_BYTES_CNT;
	d1.month = udp_buffer[index];
	printf(" month %d",d1.month);
	index += MM_BYTES_CNT;
	d1.day = udp_buffer[index];
	printf(" day %d\n",d1.day);
	d1.hh =0;d1.mm=0;d1.ss=0;	
	index = RES_HS+HS_BYTES_CNT;
	snObj.val32 =get_owner_balance(owner_id);
	for(j=0;j<AMT_BYTES_CNT;j++)
		response[index+j]=snObj.data[(AMT_BYTES_CNT-1-j)];
	index+=AMT_BYTES_CNT;
	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_details_obj[i].owner_id==0){
			continue;
		}
		d2.year = DEFAULT_YEAR + coin_owner_details_obj[i].time_stamp[0];
		d2.month = coin_owner_details_obj[i].time_stamp[1];
		d2.day = coin_owner_details_obj[i].time_stamp[2];
		d2.hh = 0;d2.mm = 0;d2.ss = 0;
		printf("year %d",d2.year);
		printf(" month %d",d2.month);
		printf(" day %d\n",d2.day);
		if(coin_owner_details_obj[i].owner_id == owner_id){
			if(compare_date(d1,d2)==1){
				memcpy(&response[index],coin_owner_details_obj[i].guid,GUID_BYTES_CNT);
				index+=GUID_BYTES_CNT;
				response[index] = coin_owner_details_obj[i].trans_type;
				index+=TT_BYTES_CNT;
				snObj.val32 =coin_owner_details_obj[i].amt;
				for(j=0;j<AMT_BYTES_CNT;j++)
					response[index+j]=snObj.data[(AMT_BYTES_CNT-1-j)];
				index+=AMT_BYTES_CNT;
				snObj.val32 =coin_owner_details_obj[i].balance;
				for(j=0;j<AMT_BYTES_CNT;j++)
					response[index+j]=snObj.data[(AMT_BYTES_CNT-1-j)];
				index+=AMT_BYTES_CNT;
				for(j=0;j<TIME_STAMP_BYTES_CNT;j++)
					response[index+j]=coin_owner_details_obj[i].time_stamp[j];
				index+=TIME_STAMP_BYTES_CNT;
				memcpy(&response[index],coin_owner_details_obj[i].meta_data,META_DATA_BYTES_CNT);
				index+=META_DATA_BYTES_CNT;
				rows_cnt++;
				if(rows_cnt == no_of_rows){
					break;
				}
			}
		}
	}
	if(rows_cnt == 0){
		send_response(NO_STATEMENTS_FOUND,index);
	}else{
		send_response(SUCCESS,index);
	}
}
//---------------------------------------------------------------
//DELETE STATMENTS COMMAND  
//---------------------------------------------------------------
void execute_del_statements(unsigned int packet_len){
	int req_body = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,j=0,owner_id;
	int req_header_min;
	unsigned char status_code;
	printf("DEL STATEMENT COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(FAILED_TO_AUTHENTICATE,RES_HS+HS_BYTES_CNT);	
		return;
	}
	remove_coin_owner_details(owner_id,1);
	index = RES_HS+HS_BYTES_CNT;
	send_response(SUCCESS,index);
}
//---------------------------------------------------------------
//SHOW PAYMENT COMMAND  
//---------------------------------------------------------------
void execute_show_payment(unsigned int packet_len){
	int RAND_BYTES = 8;
	int req_body = CH_BYTES_CNT +  RAND_BYTES + GUID_BYTES_CNT + RAND_BYTES + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,j=0,SPACER_CNT=8;
	int req_header_min=0;
	unsigned char status_code,guid[GUID_BYTES_CNT];//,spacer[8],value=0;
	printf("SHOW PAYMENT COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT + RAND_BYTES;	
	memcpy(guid,&udp_buffer[index],GUID_BYTES_CNT);
	/*memcpy(spacer,&udp_buffer[index],8);
	printf("\n spacer \n");
	for(i=0;i<SPACER_CNT;i++){
		printf("%d,",spacer[i]);	
	}
	index+=SPACER_CNT;
	printf("\n");
	for(i=0,j=0;i<GUID_BYTES_CNT;i+=2,j++){
		value = spacer[j]>>4;
		index+= value;
		guid[i] = udp_buffer[index];
		index++;
		value = spacer[j] & 0x0F;
		index+=value;
		guid[i+1] = udp_buffer[index];
		index++;
	}
	for(i=0;i<GUID_BYTES_CNT;i++){
		printf("%d",guid[i]);
	}*/

	for(i=0;i<COIN_OWNER_MAX;i++){
		if(coin_owner_details_obj[i].owner_id==0){
			continue;
		}
		if(memcmp(&coin_owner_details_obj[i].guid,&guid,GUID_BYTES_CNT)==0){
			index = RES_HS+HS_BYTES_CNT;
			response[index] = coin_owner_details_obj[i].trans_type;
			index+=TT_BYTES_CNT;
			snObj.val32 =coin_owner_details_obj[i].amt;
			for(j=0;j<AMT_BYTES_CNT;j++)
				response[index+j]=snObj.data[(AMT_BYTES_CNT-1-j)];
			index+=AMT_BYTES_CNT;
			for(j=0;j<TIME_STAMP_BYTES_CNT;j++)
				response[index+j]=coin_owner_details_obj[i].time_stamp[j];
			index+=TIME_STAMP_BYTES_CNT;
			snObj.val32 =coin_owner_details_obj[i].owner_id;
			for(j=0;j<OWNER_ID_BYTES_CNT;j++)
				response[index+j]=snObj.data[(OWNER_ID_BYTES_CNT-1-j)];
			index+=OWNER_ID_BYTES_CNT;
			for(j=0;j<OWNER_ID_BYTES_CNT;j++)
				response[index+j]=0;
			index+=OWNER_ID_BYTES_CNT;
			memcpy(&response[index],coin_owner_details_obj[i].meta_data,META_DATA_BYTES_CNT);
			index+=META_DATA_BYTES_CNT;
			break;
		}
	}
	send_response(SUCCESS,index);
}
//---------------------------------------------------------------
//CHANGE COIN TYPE COMMAND  
//---------------------------------------------------------------
void execute_change_coin_type(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT + COIN_TYPE_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,j=0,no_of_coins=0,owner_id=0,size=0;
	int req_header_min,bytes_per_coin=SN_BYTES_CNT;
	unsigned char status_code, new_coin_type=0,pass_cnt=0,fail_cnt=0,pass_fail[COINS_MAX]={0};
	printf("CHANGE COIN TYPE COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(FAILED_TO_AUTHENTICATE,RES_HS+HS_BYTES_CNT);	
		return;
	}
	index += AN_BYTES_CNT;
	new_coin_type = udp_buffer[index];
	index+=COIN_TYPE_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		pass_fail[i]=0;
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == owner_id && coin_owner_obj[j].serial_no == snObj.val32){
				coin_owner_obj[j].coin_type = new_coin_type;
				pass_fail[i]=1;
				pass_cnt++;
				break;
			}
		}
		if(pass_fail[i]==0){
			fail_cnt++;
		}
		index +=SN_BYTES_CNT;
	}
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index =  RES_HS+HS_BYTES_CNT;
	size   =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//-------------------------------------------------------------------------------------------------
// sync response result gathering
//-------------------------------------------------------------------------------------------------
//
void* sync_collect_result(void *args){
  union coversion snObj;
	struct sync_resp *sync_resp_inp =  (struct sync_resp *)args;
	unsigned int i =0,len,index,no_of_srNos,j=0,k=0,n=0,frame_cnt,coin_type,rand_no;
	unsigned char buffer[UDP_BUFF_SIZE]={0,0,0,0,0,CMD_SYNC_TRANS_RESP,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,0,0,0,43,55,0x3E,0x3E};
	unsigned char recv_buffer[COINS_MAX],buffer_tmp[1024],encry[AN_BYTES_CNT],nounce[NOUNCE_BYTES_CNT];

	buffer[REQ_RI]=sync_resp_inp->dest_raida;
	index=REQ_HEAD_MIN_LEN + CH_BYTES_CNT;
	memset(snObj.data,0,4);
	snObj.val32 = sync_resp_inp->own_id;
	for(k=0;k<OWNER_ID_BYTES_CNT;k++)
		buffer[index+k]=snObj.data[(OWNER_ID_BYTES_CNT-1-k)];
	index+=OWNER_ID_BYTES_CNT;
	for(i=0;i<sync_resp_inp->sn_cnt;i++){
		memset(snObj.data,0,4);
		snObj.val32=sync_resp_inp->sn_nums[i];
		for(k=0;k<SN_BYTES_CNT;k++)
			buffer[index+k]=snObj.data[(SN_BYTES_CNT-1-k)];
		index+=SN_BYTES_CNT;
	}
	buffer[REQ_EN] = 1;// encryption 1 enabled
	buffer[index] = REQ_END;
	buffer[index+1] = REQ_END;
	len=0;
	for(i=0;i<UDP_BUFF_SIZE;i++){
		len++;
		if(buffer[i]==REQ_END && buffer[i-1]==REQ_END){
			break;
		}
	}
	frame_cnt =len/server_config_obj.bytes_per_frame;
	if((len % server_config_obj.bytes_per_frame)!=0)
		frame_cnt ++;		
	snObj.val32 = frame_cnt;
	buffer[REQ_FC] = snObj.data[1];
	buffer[REQ_FC+1] = snObj.data[0];
	
	srand (time(NULL));	
	rand_no=rand() % (server_config_obj.my_id_coins_cnt-1); 
	buffer[REQ_EN] = ENCRYP_128_AES_CTR_SN;
	snObj.val32 = my_id_coins_obj[rand_no].sr_no;;
	for(j=0;j<SN_BYTES_CNT;j++){
		buffer[REQ_SR_N0+j]=snObj.data[SN_BYTES_CNT-1-j];
	}

	//We take nouce 5 bytes
	for(i=0;i<5;i++){
		nounce[i] = buffer[REQ_NO_1+i];
	}
	j=0;
	//We take nouce 3 bytes 
	for(i=5;i<8;i++,j++){
		nounce[i] = buffer[REQ_NO_6+j];
	}

	printf("\nbfor encryption\n");
	for(i=0;i<len;i++){
		printf("%d,",buffer[i]);
  }

  printf("\nkey %d\n", sync_resp_inp->dest_raida);

	memcpy(buffer_tmp,&buffer[REQ_HEAD_MIN_LEN],len-REQ_HEAD_MIN_LEN);
	memcpy(encry,my_id_coins_obj[rand_no].AN[sync_resp_inp->dest_raida],AN_BYTES_CNT);//
  for (i=0; i < AN_BYTES_CNT; i++) {
    printf("%d,", encry[i]);
  }
  printf("\nnonce:\n");
  for (i=0; i < 16; i++) {
    printf("%d,", nounce[i]);
  }
	crypt_ctr(encry,buffer_tmp,len-REQ_HEAD_MIN_LEN,nounce);
	memcpy(&buffer[REQ_HEAD_MIN_LEN],buffer_tmp,len-REQ_HEAD_MIN_LEN);

	memcpy(dns_config_obj[sync_resp_inp->dest_raida].buffer,buffer,len);
	printf("\n");
	for(i=0;i<len;i++){
		printf("%d,",dns_config_obj[sync_resp_inp->dest_raida].buffer[i]);
	}
	send_to_dns(sync_resp_inp->dest_raida,len);
	n=listen_dns_socket(sync_resp_inp->dest_raida);
	
	if(n<=0){
		sync_resp_obj[sync_resp_inp->dest_raida].sn_cnt=0;	
	}else{

		memcpy(buffer_tmp,&dns_config_obj[sync_resp_inp->dest_raida].buffer[RES_HEAD_MIN_LEN],n-RES_HEAD_MIN_LEN);
		//memcpy(encry,my_id_coins_obj[rand_no].AN[sync_resp_inp->dest_raida],AN_BYTES_CNT);//
		crypt_ctr(encry,buffer_tmp,n-RES_HEAD_MIN_LEN,nounce);
		memcpy(&dns_config_obj[sync_resp_inp->dest_raida].buffer[RES_HEAD_MIN_LEN],buffer_tmp,n-RES_HEAD_MIN_LEN);		

		sync_resp_obj[sync_resp_inp->dest_raida].sn_cnt= (n-RESP_BUFF_MIN_CNT);
		index = RESP_BUFF_MIN_CNT;
		for(j=0;j<sync_resp_obj[sync_resp_inp->dest_raida].sn_cnt;j++){
			coin_type=dns_config_obj[sync_resp_inp->dest_raida].buffer[index];	
			sync_resp_obj[sync_resp_inp->dest_raida].coin_type[j] = coin_type;	
			index++;
			printf("\n%d %d",sync_resp_obj[sync_resp_inp->dest_raida].sn_nums[j], coin_type);
		}
	}

}
//---------------------------------------------------------------
//SYNC TRANS ADD COMMAND  
//---------------------------------------------------------------
void execute_sync_trans_add(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int index=0,j=0,k=0,no_of_coins=0,owner_id=0,bytes_per_coin = SN_BYTES_CNT,max_cnt=0,coin_type_max=0;
	int req_header_min,i=0;
	unsigned char status_code,upper=RAIDA_SERVER_MAX,lower=0,no;
	pthread_t ptid[RAIDA_SERVER_MAX]; 
	struct srno_coin_cnt{
		unsigned int sr_no;
		unsigned int coin_cnt[COIN_TYPE_MAX];
	}*srno_coin_cnt_obj;
	printf("SYNC TRANS ADD COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(FAILED_TO_AUTHENTICATE,RES_HS+HS_BYTES_CNT);	
		return;
	}
	for(i=0;i<RAIDA_SERVER_MAX;i++){
		sync_resp_obj[i].dest_raida = -1;
	}
	index += AN_BYTES_CNT;
	
  int add = 0;
  if (server_config_obj.raida_id < SYNC_RESP_RAIDA_CNT)
    add++;

	for(i=0;i<SYNC_RESP_RAIDA_CNT + add;i++){
		srand (time(NULL));	
		//no=rand() % RAIDA_SERVER_MAX;
    no=i;
    printf("i=%d\n",i);
		if(server_config_obj.raida_id == no){
//		if(server_config_obj.raida_id == no || sync_resp_obj[no].dest_raida == no){
//			i--;
			continue;
		}
    printf("i1=%d\n",i);
		sync_resp_obj[no].dest_raida = no;
		sync_resp_obj[no].own_id =owner_id;
		index = req_header_min+CH_BYTES_CNT+OWNER_ID_BYTES_CNT+ AN_BYTES_CNT;	
		printf("\n -----------------");
		printf("\nRaida no :- %d", no);
		for(j=0;j<no_of_coins;j++) {
      printf("j=%d\n",j);
			memset(snObj.data,0,4);
			for(k=0;k<SN_BYTES_CNT;k++)
				snObj.data[k]=udp_buffer[index+(SN_BYTES_CNT-1-k)];
      /*
			for(k=0;k<COIN_OWNER_MAX;k++){
				if(coin_owner_obj[k].owner_id == owner_id  && coin_owner_obj[k].serial_no == snObj.val32){
					send_response(SYNC_ADD_COIN_EXIST,RES_HS+HS_BYTES_CNT);
					return;
				}
			}
      */
			sync_resp_obj[no].sn_nums[j]=snObj.val32;
			index +=SN_BYTES_CNT;
			printf("\nSerial no :- %d",snObj.val32);
		}
		sync_resp_obj[no].sn_cnt = no_of_coins;	
		pthread_create(&ptid[no], NULL, &sync_collect_result, &sync_resp_obj[no]);
	}
	for(i=0;i<RAIDA_SERVER_MAX;i++){
		if(sync_resp_obj[i].dest_raida!=-1){
			pthread_join(ptid[i], NULL);
		}
	}
	srno_coin_cnt_obj = (struct srno_coin_cnt *) malloc(sizeof(struct srno_coin_cnt) * no_of_coins);
	memset(srno_coin_cnt_obj,0,sizeof(struct srno_coin_cnt) * no_of_coins);
	//analyze the results
	for(i=0;i<RAIDA_SERVER_MAX;i++){
		if(sync_resp_obj[i].dest_raida!=-1){
      printf("adding for raida %d\n", sync_resp_obj[i].dest_raida);
			for(j=0;j<no_of_coins;j++){
        printf("j=%d ctype=%d nums=%d\n",j, sync_resp_obj[i].coin_type[j], sync_resp_obj[i].sn_nums[j]);
				srno_coin_cnt_obj[j].coin_cnt[sync_resp_obj[i].coin_type[j]]++;
				srno_coin_cnt_obj[j].sr_no=sync_resp_obj[i].sn_nums[j];
			}
		}
	}
	for(i=0;i<no_of_coins;i++){
		max_cnt=0;
		for(j=0;j<COIN_TYPE_MAX;j++){
			if(srno_coin_cnt_obj[i].coin_cnt[j]!=0){
        printf("rrj=%d max=%d cnt=%d\n", j, max_cnt, srno_coin_cnt_obj[i].coin_cnt[j]);
				if(srno_coin_cnt_obj[i].coin_cnt[j]>max_cnt){
					max_cnt=srno_coin_cnt_obj[i].coin_cnt[j];
					coin_type_max =j;
				}
			}
		}
		printf("\n Serial Number %d ,%d",srno_coin_cnt_obj[i].sr_no, coin_type_max);
		if(coin_type_max != COIN_TYPE_MAX - 1){
			add_coin_owner(srno_coin_cnt_obj[i].sr_no,owner_id,coin_type_max,1);
		}
	}
	free(srno_coin_cnt_obj);
	//send response as SUCCESS 
	send_response(SUCCESS, RES_HS+HS_BYTES_CNT);
}
//---------------------------------------------------------------
//SYNC TRANS DEL COMMAND  
//---------------------------------------------------------------
void execute_sync_trans_del(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + AN_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int index=0,i=0,j=0,k=0,no_of_coins=0,owner_id=0,bytes_per_coin = SN_BYTES_CNT,size=0;
	int req_header_min;
	unsigned char status_code,upper=RAIDA_SERVER_MAX,lower=0,no;
	unsigned char pass_cnt=0,fail_cnt=0,pass_fail[COINS_MAX]={0};
	printf("SYNC TRANS DEL COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	if(memcmp(coin_id_obj[COIN_ID_INDEX].AN[owner_id],&udp_buffer[index],AN_BYTES_CNT)!=0){
		send_response(FAILED_TO_AUTHENTICATE,RES_HS+HS_BYTES_CNT);	
		return;
	}
	index += AN_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(k=0;k<SN_BYTES_CNT;k++)
			snObj.data[k]=udp_buffer[index+(SN_BYTES_CNT-1-k)];
		pass_fail[i]=0;	
		for(k=0;k<COIN_OWNER_MAX;k++){
			if(coin_owner_obj[k].owner_id == owner_id  && coin_owner_obj[k].serial_no == snObj.val32){
				pass_fail[i]=1;
				pass_cnt++;
				remove_coin_owner(snObj.val32,owner_id,1);
				break;
			}
		}
		printf("\nSerial no :- %d",snObj.val32);
		if(pass_fail[i]==0){
			fail_cnt++;
		}
		index +=SN_BYTES_CNT;
	}
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index =  RES_HS+HS_BYTES_CNT;
	size   =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//---------------------------------------------------------------
//SYNC TRANS RESP COMMAND  
//---------------------------------------------------------------
void execute_sync_trans_resp(unsigned int packet_len){
	int req_body_without_coins = CH_BYTES_CNT +  OWNER_ID_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,index1=0,j=0,no_of_coins=0,owner_id=0;
	int req_header_min,bytes_per_coin = SN_BYTES_CNT;
	unsigned char status_code;
	printf("SYNC TRANS RESP COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	memset(snObj.data,0,4);
	for(j=0;j<OWNER_ID_BYTES_CNT;j++)
		snObj.data[j]=udp_buffer[index+(OWNER_ID_BYTES_CNT-1-j)];
	owner_id=snObj.val32;
	if(snObj.val32 >=coin_id_obj[COIN_ID_INDEX].AN_CNT){
		send_err_resp_header(COIN_NO_NOT_FOUND);
		return;
	}
	index += OWNER_ID_BYTES_CNT;
	index1 = RES_HS+HS_BYTES_CNT;
	for(i=0;i<no_of_coins;i++) {
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];

    printf("sn %d %d\n", snObj.val32, owner_id);
		for(j=0;j<COIN_OWNER_MAX;j++){
			if(coin_owner_obj[j].owner_id == owner_id  && coin_owner_obj[j].serial_no == snObj.val32){
        printf("ctype %d\n", coin_owner_obj[j].coin_type);
				response[index1]=coin_owner_obj[j].coin_type;
				index1++;
				break;
			}
		}
		if(j>=COIN_OWNER_MAX){
			response[index1]=COIN_TYPE_INVALID;
			index1++;
		}
		index +=SN_BYTES_CNT;
	}	

  printf("senddd %d\n", index1);
	send_response(SUCCESS,index1);
}

//---------------------------------------------------------------
//UPGRADE COIN COMMAND  
//---------------------------------------------------------------
void execute_upgrade_coin_old(unsigned int packet_len){
	unsigned int req_body_without_coins = CH_BYTES_CNT +  LEGACY_RAIDA_TK_CNT + PG_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,index1=0,j=0,no_of_coins=0,size=0,no_of_srNos=0,len,n,k,frame_cnt;
	int req_header_min,bytes_per_coin=SN_BYTES_CNT;
	unsigned char buffer_coin_conv[UDP_BUFF_SIZE]={0,0,0,0,0,215,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,102,212,43,55,0x3E,0x3E};
	unsigned char buffer_tmp[UDP_BUFF_SIZE];
	unsigned char status_code, pass_cnt=0,fail_cnt=0,pass_fail[COIN_CONV_MAX_SR_NO]={0},legacy_tk[LEGACY_RAIDA_TK_CNT],pg[PG_BYTES_CNT];
	unsigned char md_input[64],md_output[64],tmp[64];
	unsigned int sr_nos_input[COIN_CONV_MAX_SR_NO],sr_nos_from_legacy[COIN_CONV_MAX_SR_NO];
	printf("UPGRADE COIN COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	index1 = req_header_min+CH_BYTES_CNT;	
	memcpy(legacy_tk,&udp_buffer[index],LEGACY_RAIDA_TK_CNT);
	memcpy(&buffer_coin_conv[index1],legacy_tk,LEGACY_RAIDA_TK_CNT);
	index+=LEGACY_RAIDA_TK_CNT;	
	index1+=LEGACY_RAIDA_TK_CNT;	
	memcpy(pg,&udp_buffer[index],PG_BYTES_CNT);
	index+=PG_BYTES_CNT;
	buffer_coin_conv[REQ_RI] = udp_buffer[REQ_RI];

	//copy all serial numbers	
	for(i=0;i<no_of_coins;i++){
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++){
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		sr_nos_input[i]=snObj.val32;
		printf("\n Inputs srn %d\n",sr_nos_input[i]);
		index+=SN_BYTES_CNT;
	}

	buffer_coin_conv[index1]=REQ_END;
	buffer_coin_conv[index1+1]=REQ_END;
	len=0;
	for(i=0;i<UDP_BUFF_SIZE;i++){
		len++;
		if(buffer_coin_conv[i]==REQ_END && buffer_coin_conv[i-1]==REQ_END){
			break;
		}
	}
	frame_cnt =len/server_config_obj.bytes_per_frame;
	if((len % server_config_obj.bytes_per_frame)!=0)
		frame_cnt ++;		
	snObj.val32 = frame_cnt;
	buffer_coin_conv[REQ_FC] = snObj.data[1];
	buffer_coin_conv[REQ_FC+1] = snObj.data[0];
	/*for(i=0;i<len;i++){
		//printf("%d,",buffer_coin_conv[i]);
		 //printf("%.2x",out[i]);
	}*/
	memcpy(buffer_tmp,&buffer_coin_conv[REQ_HEAD_MIN_LEN],len-REQ_HEAD_MIN_LEN);
	crypt_ctr(legacy_encrypt_key,buffer_tmp,len-REQ_HEAD_MIN_LEN,nounce);
	memcpy(&buffer_coin_conv[REQ_HEAD_MIN_LEN],buffer_tmp,len-REQ_HEAD_MIN_LEN);
	memcpy(raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer,buffer_coin_conv,len);
	/*printf("\n ctr --");
	for(i=0;i<len;i++){
		printf("%d,",raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[i]);
		 //printf("%.2x",out[i]);
	}
	printf("\n");*/
	send_to_legacy(udp_buffer[REQ_RI],len);
	n=listen_legacy_socket(udp_buffer[REQ_RI]);
	memcpy(buffer_tmp,&raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_HEAD_MIN_LEN],len-RES_HEAD_MIN_LEN);
	crypt_ctr(legacy_encrypt_key,buffer_tmp,len-RES_HEAD_MIN_LEN,nounce);
	memcpy(&raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_HEAD_MIN_LEN],buffer_tmp,len-RES_HEAD_MIN_LEN);
	/*printf("\n ctr --");
	for(i=0;i<n;i++){
		printf("%d,",raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[i]);
	}*/
	if(n>0){
		//Getting response from correct RAIDA and SUCEESS
		if(raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_RI]==udp_buffer[REQ_RI] && raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_SS]==SUCCESS){
			no_of_srNos = (n-RESP_BUFF_MIN_CNT)/SN_BYTES_CNT;
			printf("\nSerial Nos %d \n",no_of_srNos);
			index=RESP_BUFF_MIN_CNT;
			for(i=0;i<no_of_srNos;i++){
				memset(snObj.data,0,4);
				for(k=0;k<SN_BYTES_CNT;k++)
					snObj.data[k]=raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[index+(SN_BYTES_CNT-1-k)];
				printf(" %d",snObj.val32);
				//search the legacy serial number in list of input serial numbers
				for(j=0;j<no_of_coins;j++){
					if(snObj.val32  == sr_nos_input[j]){
						break;
					}
				}
				//Serial number was found	
				if(j<no_of_coins){
					printf("\n Sr no : %d",snObj.val32);	
					sprintf(md_input, "%d", udp_buffer[REQ_RI]);
					sprintf(tmp, "%d", snObj.val32);
					strcat(md_input,tmp);
					for(k=0;k<PG_BYTES_CNT;k++){		
						sprintf(tmp,"%02x", pg[k]);
						strcat(md_input,tmp);
					}
					printf("\n %s",md_input);
					md5(md_input,md_output);
					memcpy(coin_id_obj[COIN_CLOUD_INDEX].AN,md_output,AN_BYTES_CNT);
					coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
					pass_fail[i]=1;
					pass_cnt++;
				}else{
					pass_fail[i]=0;
					fail_cnt++;
				}
				index+=SN_BYTES_CNT;	
			}			
			if (fail_cnt == no_of_coins){
				status_code = ALL_FAIL;
			}else if(pass_cnt == no_of_coins){
				status_code = ALL_PASS;
			}else{
				status_code = MIX;
			}
		} else {
			status_code=LEGACY_RAIDA_TICKET_NOT_FOUND;	
		}
	}else{
		status_code=LEGACY_RAIDA_TIME_OUT;
	}
	index = RES_HS+HS_BYTES_CNT;
	size    =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	send_response(status_code,size);
}
//---------------------------------------------------------------
//UPGRADE COIN COMMAND  
//---------------------------------------------------------------
void execute_upgrade_coin(unsigned int packet_len){
	unsigned int req_body = CH_BYTES_CNT + LEGACY_RAIDA_TK_CNT + SN_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,index=0,index1=0,j=0,size=0,no_of_srNos=0,len,n,k,frame_cnt,ticket;
	int req_header_min;
	unsigned char buffer_coin_conv[UDP_BUFF_SIZE]={0,0,0,0,0,215,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,102,212,43,55,0x3E,0x3E};
	unsigned char buffer_tmp[UDP_BUFF_SIZE];
	unsigned char status_code,legacy_tk[LEGACY_RAIDA_TK_CNT];
	unsigned int sr_no,sr_nos_from_legacy[COIN_CONV_MAX_SR_NO],amt=0,deno_cnt[5]={0},value=0;
	float div_factor = 85.125;
	struct upgrade_coin_table {
		unsigned int coins_cnt;
		unsigned int deno;
		unsigned int value;
	}obj_upgrade_coin_table[5];
	obj_upgrade_coin_table[0].coins_cnt = 255;obj_upgrade_coin_table[0].deno = 1;obj_upgrade_coin_table[0].value = 3;
	obj_upgrade_coin_table[1].coins_cnt = 323;obj_upgrade_coin_table[1].deno = 5;obj_upgrade_coin_table[1].value = 19;
	obj_upgrade_coin_table[2].coins_cnt = 320;obj_upgrade_coin_table[2].deno = 25;obj_upgrade_coin_table[2].value = 94;
	obj_upgrade_coin_table[3].coins_cnt = 326;obj_upgrade_coin_table[3].deno = 100;obj_upgrade_coin_table[3].value = 383;
	obj_upgrade_coin_table[4].coins_cnt = 317;obj_upgrade_coin_table[4].deno = 250;obj_upgrade_coin_table[4].value = 931;
	
	printf("UPGRADE COIN COMMAND. \n");
	if (validate_request_body_general(packet_len,req_body,&req_header_min) ==0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;	
	index1 = req_header_min+CH_BYTES_CNT;	
	memcpy(legacy_tk,&udp_buffer[index],LEGACY_RAIDA_TK_CNT);
	memcpy(&buffer_coin_conv[index1],legacy_tk,LEGACY_RAIDA_TK_CNT);
	index+=LEGACY_RAIDA_TK_CNT;	
	index1+=LEGACY_RAIDA_TK_CNT;	
	buffer_coin_conv[REQ_RI] = udp_buffer[REQ_RI];
	memset(snObj.data,0,4);
	for(j=0;j<SN_BYTES_CNT;j++){
		snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
	}
	sr_no=snObj.val32;
	buffer_coin_conv[index1]=REQ_END;
	buffer_coin_conv[index1+1]=REQ_END;
	len=0;
	for(i=0;i<UDP_BUFF_SIZE;i++){
		len++;
		if(buffer_coin_conv[i]==REQ_END && buffer_coin_conv[i-1]==REQ_END){
			break;
		}
	}
	frame_cnt =len/server_config_obj.bytes_per_frame;
	if((len % server_config_obj.bytes_per_frame)!=0)
		frame_cnt ++;		
	snObj.val32 = frame_cnt;
	buffer_coin_conv[REQ_FC] = snObj.data[1];
	buffer_coin_conv[REQ_FC+1] = snObj.data[0];
	for(i=0;i<len;i++){
		printf("%d,",buffer_coin_conv[i]);
		 //printf("%.2x",out[i]);
	}
	memcpy(buffer_tmp,&buffer_coin_conv[REQ_HEAD_MIN_LEN],len-REQ_HEAD_MIN_LEN);
	crypt_ctr(legacy_encrypt_key,buffer_tmp,len-REQ_HEAD_MIN_LEN,nounce);
	memcpy(&buffer_coin_conv[REQ_HEAD_MIN_LEN],buffer_tmp,len-REQ_HEAD_MIN_LEN);
	memcpy(raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer,buffer_coin_conv,len);
	//printf("\n ctr --");
	//for(i=0;i<len;i++){
	//	printf("%d,",raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[i]);
		 //printf("%.2x",out[i]);
	//}
	//printf("\n");
	send_to_legacy(udp_buffer[REQ_RI],len);
	n=listen_legacy_socket(udp_buffer[REQ_RI]);
	memcpy(buffer_tmp,&raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_HEAD_MIN_LEN],len-RES_HEAD_MIN_LEN);
	crypt_ctr(legacy_encrypt_key,buffer_tmp,len-RES_HEAD_MIN_LEN,nounce);
	memcpy(&raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_HEAD_MIN_LEN],buffer_tmp,len-RES_HEAD_MIN_LEN);
	//printf("\n ctr --");
	//for(i=0;i<n;i++){
	//	printf("%d,",raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[i]);
	//}
	index1 = RES_HS+HS_BYTES_CNT;
	if(n>0){
		//Getting response from correct RAIDA and SUCEESS
		if(raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_RI]==udp_buffer[REQ_RI] && raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[RES_SS]==SUCCESS){
			unsigned char deno;
			no_of_srNos = (n-RESP_BUFF_MIN_CNT)/SN_BYTES_CNT;
			printf("\nSerial Nos %d \n",no_of_srNos);
			index=RESP_BUFF_MIN_CNT;
			amt = 0;
			value=0;
			for(i=0;i<no_of_srNos;i++){
				memset(snObj.data,0,4);
				for(k=0;k<SN_BYTES_CNT;k++)
					snObj.data[k]=raida_legacy_config_obj[udp_buffer[REQ_RI]].buffer[index+(SN_BYTES_CNT-1-k)];
				deno=get_legacy_denomination(snObj.val32);
				deno_cnt[get_legacy_denomination_index(deno)]++;
				amt+=deno;
				printf(" %d",snObj.val32);
				index+=SN_BYTES_CNT;	
			}
			for(i=0;i<5;i++){
				if(obj_upgrade_coin_table[i].coins_cnt == deno_cnt[i]){
					value = obj_upgrade_coin_table[i].value;			
					break;
				}
			}
			if(i>=5){
				value = floor(amt/div_factor);
			}

			index1 = RES_HS+HS_BYTES_CNT;
			index  = sr_no;
			printf("Input Serial number %d \n",sr_no);
			printf("The added serial numbers amount is %d conversion value is %d \n",amt,value);
			status_code = UPGRADE_COIN_SRNO_CONT;
			srand (time(NULL));	
			ticket = rand();
			printf("The ticket number is %d",ticket);
			add_upgrade_coin_ticket(ticket,value);
			memset(snObj.data,0,4);
			snObj.val32 = ticket;
			for(j=0;j<TK_BYTES_CNT;j++){
				response[index1+j]=snObj.data[(TK_BYTES_CNT-1-j)];
			}
			index1+=TK_BYTES_CNT;
			for(i=0;i<UPGRADE_COIN_SRNO_CNT;i++){
				
				if(coin_id_obj[COIN_CLOUD_INDEX].MFS[index]!=0){
					status_code = UPGRADE_COIN_SRNO_NON_CONT;
				}else{
					memset(snObj.data,0,4);
					snObj.val32 = index;
					for(j=0;j<SN_BYTES_CNT;j++){
						response[index1+j]=snObj.data[(SN_BYTES_CNT-1-j)];
					}
					index1+=SN_BYTES_CNT;
				}
				index++;
				if(index>=coin_id_obj[COIN_CLOUD_INDEX].AN_CNT) {
					index = 0;
				}

			}
			if(status_code == UPGRADE_COIN_SRNO_CONT){
				memset(snObj.data,0,4);
				snObj.val32 = sr_no;
				index1 = RES_HS+HS_BYTES_CNT+TK_BYTES_CNT;
				for(j=0;j<SN_BYTES_CNT;j++){
					response[index1+j]=snObj.data[(SN_BYTES_CNT-1-j)];
				}
				index1+=SN_BYTES_CNT;
			}
			
		} else {
			status_code=LEGACY_RAIDA_TICKET_NOT_FOUND;	
		}
	}else{
		status_code=LEGACY_RAIDA_TIME_OUT;
	}
	send_response(status_code,index1);
}
//---------------------------------------------------------------
//COIN CLAIM COMMAND  
//---------------------------------------------------------------
void execute_coin_claim(unsigned int packet_len){
	unsigned int req_body = CH_BYTES_CNT +  TK_BYTES_CNT	 + CT_BYTES_CNT + PG_BYTES_CNT + CMD_END_BYTES_CNT;
	unsigned int i=0,k,index=0,j=0,size=0,ticket,no_of_coins,bytes_per_coin=SN_BYTES_CNT,upgrade_ticket_index=0;
	unsigned int pass_cnt=0,fail_cnt=0;
	int req_header_min;
	unsigned char status_code,ct=0,pass_fail[UPGRADE_COIN_SRNO_CNT]={0},md_input[64],md_output[64],tmp[64],pg[PG_BYTES_CNT];
	time_t now = time(0);
	struct tm *t = gmtime(&now);
	printf("COIN CLAIM COMMAND. \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	index = req_header_min+CH_BYTES_CNT;
	memset(snObj.data,0,4);
	for(j=0;j<TK_BYTES_CNT;j++){
		snObj.data[j]=udp_buffer[index+(TK_BYTES_CNT-1-j)];
	}
	ticket = snObj.val32;
	for(i=0;i<COIN_UPGRADE_TICKETS_MAX;i++){
		if(upgrade_coin_tickets_obj[i].ticket == ticket){
			upgrade_ticket_index = i;
			break;
		}
	}
	if(i>=COIN_UPGRADE_TICKETS_MAX){
		send_response(RAIDA_TICKET_NOT_FOUND,RES_HS+HS_BYTES_CNT);
		return;
	}
	index+=TK_BYTES_CNT	;	
	ct=udp_buffer[index];
	index++;
	memcpy(pg,&udp_buffer[index],PG_BYTES_CNT);
	no_of_coins = upgrade_coin_tickets_obj[upgrade_ticket_index].value;
	index = req_header_min+CH_BYTES_CNT + TK_BYTES_CNT + CT_BYTES_CNT + PG_BYTES_CNT;
	for(i=0;i<no_of_coins;i++){
		if(ct == 0){
			memset(snObj.data,0,4);
			for(j=0;j<SN_BYTES_CNT;j++){
				snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
			}
			index+=SN_BYTES_CNT;
		}else{
			if(i == 0){
				memset(snObj.data,0,4);
				for(j=0;j<SN_BYTES_CNT;j++){
					snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];
				}
			}else{
				snObj.val32++;
			}
		}
		if((snObj.val32>coin_id_obj[COIN_CLOUD_INDEX].AN_CNT) || (coin_id_obj[COIN_CLOUD_INDEX].MFS[snObj.val32]!=0)) {
			pass_fail[i]=0;
			fail_cnt++;
		}else{
			pass_fail[i]=1;
			pass_cnt++;
			sprintf(md_input, "%d", udp_buffer[REQ_RI]);
			sprintf(tmp, "%d", snObj.val32);
			strcat(md_input,tmp);
			for(k=0;k<PG_BYTES_CNT;k++){		
				sprintf(tmp,"%02x", pg[k]);
				strcat(md_input,tmp);
			}
			md5(md_input,md_output);
			printf("\n Sr no, %d md :-",snObj.val32);
			for (j=0; j<AN_BYTES_CNT;j++) {
			        printf("%d, ", md_output[j]);
			}
			memcpy(coin_id_obj[COIN_CLOUD_INDEX].AN[snObj.val32],md_output,AN_BYTES_CNT);
			coin_id_obj[COIN_CLOUD_INDEX].MFS[snObj.val32]=t->tm_mon+1;
			coin_config_obj[COIN_CLOUD_INDEX].pages_changed[((snObj.val32)/coin_config_obj[COIN_CLOUD_INDEX].page_size)]=1;
		}
		
	}
	printf("Count %d %d %d\n",fail_cnt,pass_cnt,no_of_coins);
	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	} 
	index =  RES_HS+HS_BYTES_CNT;
	size   =  RES_HS+HS_BYTES_CNT;
	if(status_code == MIX){
		for(i=0;i<no_of_coins;i++){
			if( pass_fail[i]==1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size +=no_of_coins/8;
		if((no_of_coins % 8)!=0)
			size ++;		
	}
	remove_upgrade_coin_ticket(ticket);
	send_response(status_code,size);
}
