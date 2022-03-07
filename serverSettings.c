#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include <time.h>
#include <math.h>

#define SERVER_CONFIG_BYTES 			20

char execpath[256];
void getexepath() {
 unsigned char buff[256];
  int count = readlink( "/proc/self/exe", buff, 256);
  int i=0,slash_pos;
  while(buff[i]!='\0'){
	if(buff[i]=='/'){
		slash_pos = i;
	}
	i++;
  }	
  strncpy(execpath,buff,slash_pos);
}

void main(){

	FILE *fp_inp = NULL;
	char *file_names[25]={"/deployment_data/raida0/data/server.bin","/deployment_data/raida1/data/server.bin","/deployment_data/raida2/data/server.bin","/deployment_data/raida3/data/server.bin",
						   "/deployment_data/raida4/data/server.bin","/deployment_data/raida5/data/server.bin","/deployment_data/raida6/data/server.bin","/deployment_data/raida7/data/server.bin",
						   "/deployment_data/raida8/data/server.bin","/deployment_data/raida9/data/server.bin","/deployment_data/raida10/data/server.bin","/deployment_data/raida11/data/server.bin",
						   "/deployment_data/raida12/data/server.bin","/deployment_data/raida13/data/server.bin","/deployment_data/raida14/data/server.bin","/deployment_data/raida15/data/server.bin",
						   "/deployment_data/raida16/data/server.bin","/deployment_data/raida17/data/server.bin","/deployment_data/raida18/data/server.bin","/deployment_data/raida19/data/server.bin"	,
						   "/deployment_data/raida20/data/server.bin","/deployment_data/raida21/data/server.bin","/deployment_data/raida22/data/server.bin","/deployment_data/raida23/data/server.bin",
						   "/deployment_data/raida24/data/server.bin"};	
	uint16_t port = 30000,i=0;
	int cnt=0;
	/*
	server_config_obj.raida_id=buff[0];
	server_config_obj.port_number = buff[2];
	server_config_obj.port_number|= (((uint16_t)buff[1])<<8);
	server_config_obj.security_check_frequency = buff[3] * SECURITY_CHECK_MUL_FACTOR;
	server_config_obj.backup_frequency = buff[4] * BACKUP_FREQ_MUL_FACTOR;
	server_config_obj.refresh_dns = buff[5] * REFRESH_DNS_MUL_FACTOR;
	server_config_obj.show_regs_coins_max = buff[7];
	server_config_obj.show_regs_coins_max |= (((uint16_t)buff[6])<<8);
	server_config_obj.show_denom_coins_max = buff[9];
	server_config_obj.show_denom_coins_max |= (((uint16_t)buff[8])<<8);
	server_config_obj.show_change_coins_max = buff[11];
	server_config_obj.show_change_coins_max |= (((uint16_t)buff[10])<<8);
	server_config_obj.bytes_per_frame = buff[13];
	server_config_obj.bytes_per_frame |= (((uint16_t)buff[12])<<8);
	server_config_obj.del_ticket_time_intervel = (uint16_t)buff[14];
	*/
	unsigned char path[256],buff[SERVER_CONFIG_BYTES]={0,0,10,10,10,10,0x7,0xD0,0x7,0xD0,0x7,0xD0,0x4,0x00,0x0A};
	getexepath();
	for(i=0;i<25;i++){
		getexepath();
		strcpy(path,"");
		strcpy(path,execpath);
		strcat(path,file_names[i]);
		printf("\n%s\n",path);
		if ((fp_inp = fopen(path, "rb+")) == NULL) {
			printf("\n server.bin Cannot be opened , exiting \n");
			return;
		}
		fread(buff, SERVER_CONFIG_BYTES,1, fp_inp);
		fseek(fp_inp,0,SEEK_SET);
		printf("%x\n",(unsigned char)(port >> 8));
		printf("%x\n",((unsigned char)port));
		buff[0]=(unsigned char)( (i));
		buff[1]=(unsigned char)(port >> 8);
		buff[2]=(unsigned char )port;
		fwrite(buff ,sizeof(buff),1,fp_inp);
		port++;
		fclose(fp_inp);
	}
	return;
}