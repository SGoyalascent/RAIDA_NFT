#include "udp_socket.h"
#include "library.h"
#include "NFT.h"

unsigned char path_an[256], path_meta[256], path_data[256];
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];

//-----------------------------------------------------------
//Set time out for UDP frames
//-----------------------------------------------------------
void set_time_out(unsigned char secs){     
	FD_ZERO(&select_fds);            
	FD_SET(sockfd, &select_fds);           	                                  
	timeout.tv_sec = secs; 
	timeout.tv_usec = 0;
}
//-----------------------------------------------------------
//Initialize UDP Socket and bind to the port
//-----------------------------------------------------------
int init_udp_socket() {

  setvbuf(stdout, NULL, _IONBF, 0);

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(server_config_obj.port_number);
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 ){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
}
//-----------------------------------------------------------
// receives the UDP packet from the client
//-----------------------------------------------------------
int listen_request(){
	unsigned char *buffer,state=STATE_WAIT_START,status_code,buffer_tmp[1024],err_flg=0;
	uint16_t frames_expected=0,curr_frame_no=0,n=0,i=0,index=0,j=0,coin_id;
	uint32_t client_s_addr=0;
  uint16_t csum = 0;
  unsigned int crc;
  uint8_t c0, c1;
		 	
	socklen_t len=sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);
	response_flg = UDP_RESPONSE;
	while(1){
		switch(state){
			case STATE_WAIT_START:
				printf("---------------------WAITING FOR REQ HEADER ----------------------\n");
				index=0;
				curr_frame_no=0;
				client_s_addr = 0;	
				memset(buffer,0,server_config_obj.bytes_per_frame);
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
				curr_frame_no=1;
				encrytion_type =0;
				index=0;
				err_flg=0;
				printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
				state = STATE_START_RECVD;
	
			break;		
			case STATE_START_RECVD:
				printf("---------------------REQ HEADER RECEIVED ----------------------------\n");
				 status_code=validate_request_header(buffer,n);
				if(status_code!=NO_ERR_CODE){
					send_err_resp_header(status_code);			
					state = STATE_WAIT_START;
					err_flg = 1;
				}else{
					frames_expected = buffer[REQ_FC+1];
					frames_expected|=(((uint16_t)buffer[REQ_FC])<<8);
          memset(nounce,0,NOUNCE_BYTES_CNT);
            //We take nouce 5 bytes
          for(i=0;i<5;i++){
            nounce[i] = buffer[REQ_NO_1+i];
          }
          j =0;

            //We take nouce 3 bytes 
          for(i=5;i<8;i++,j++){
            nounce[i] = buffer[REQ_NO_6+j];
          }
                printf("\n 111nounce \n");
                for(i=0;i<16;i++){
                  printf("%d,",nounce[i]);
                }
                printf("done\n");
          //check the type of encryption 
          if(buffer[REQ_EN] == ENCRYP_128_AES_CTR_SN){
            encrytion_type = buffer[REQ_EN];
            printf("\n ENCRYPTION TYPE := %d \n", encrytion_type);
            // 2bytes for 62,62 (signature)
            memcpy(buffer_tmp,&buffer[REQ_HEAD_MIN_LEN],n-REQ_HEAD_MIN_LEN-2);
            memset(snObj.data,0,4);
            for(j=0;j<SN_BYTES_CNT;j++){
              snObj.data[j]=buffer[REQ_SR_N0+(SN_BYTES_CNT-1-j)];
            }
            coin_id = buffer[REQ_COIN_ID+1];
            coin_id |= (((uint16_t)buffer[REQ_COIN_ID])<<8);
            memcpy(encrypt_key,coin_id_obj[coin_id].AN[snObj.val32],AN_BYTES_CNT);
            printf("%d\n",coin_id);
            printf("%d\n",snObj.val32);
            if(snObj.val32 >=coin_id_obj[coin_id].AN_CNT){
              send_err_resp_header(COIN_NO_NOT_FOUND);
              state = STATE_WAIT_START;
              err_flg = 1;
            } else{
              // 2bytes for 62,62 (signature)
      				if (frames_expected == 1){
                crypt_ctr(encrypt_key,buffer_tmp,n-REQ_HEAD_MIN_LEN-2,nounce);
                memcpy(&buffer[REQ_HEAD_MIN_LEN],buffer_tmp,n-REQ_HEAD_MIN_LEN-2);
                printf(" encry key\n");
                for(i=0;i<16;i++){
                  printf("%d,",encrypt_key[i]);
                }
                printf("\n nounce \n");
                for(i=0;i<16;i++){
                  printf("%d,",nounce[i]);
                }
  
                printf("\n After decry");
                for(i=0;i<n;i++){
                  printf("%d,",buffer[i]);
                }
                printf("\n");
              }
           }
          } else if (buffer[REQ_EN] == ENCRYP_128_AES_CTR_KEY_TABLE){
            int key_index;
            key_index = find_encry2_key(snObj.val32);
            printf("%d",	snObj.val32);
            if(key_index == -1){
              send_err_resp_header(KEY_NOT_CONFIG_ENCRY_2);
              state = STATE_WAIT_START;
              err_flg = 1;
            } else {
              // 2bytes for 62,62 (signature)
      				if (frames_expected == 1){
                memcpy(buffer_tmp,&buffer[REQ_HEAD_MIN_LEN],n-REQ_HEAD_MIN_LEN-2);
                memcpy(encrypt_key,key_table_obj[key_index].key,KEY_BYTES_CNT);
                crypt_ctr(encrypt_key,buffer_tmp,n-REQ_HEAD_MIN_LEN-2,nounce);
                memcpy(&buffer[REQ_HEAD_MIN_LEN],buffer_tmp,n-REQ_HEAD_MIN_LEN-2);
                for(i=0;i<16;i++){
                  printf("%d,",encrypt_key[i]);
                }
                printf("\n nounce \n");
                for(i=0;i<16;i++){
                  printf("%d,",nounce[i]);
                }
    
                printf("\n After decry");
                for(i=0;i<n;i++){
                  printf("%d,",buffer[i]);
                }
                printf("\n");
              }
            }
          } // end if frame_expected ==1
					if(err_flg == 0){
						memcpy(udp_buffer,buffer,n);
						index = n;
						client_s_addr = cliaddr.sin_addr.s_addr;
						if(frames_expected == 1){
							state = STATE_END_RECVD;
						}else{
							state = STATE_WAIT_END;
						}
					}
				}
			break;
			case STATE_WAIT_END:
				set_time_out(FRAME_TIME_OUT_SECS);
				if (select(32, &select_fds, NULL, NULL, &timeout) == 0 ){
					//send_err_resp_header(FRAME_TIME_OUT);
					send_err_resp_header(PACKET_ORDER_LOSS);
					state = STATE_WAIT_START;
					printf("Time out error \n");
				}else{
					n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
					if(client_s_addr==cliaddr.sin_addr.s_addr){
						curr_frame_no++;
						printf("--------RECVD  FRAME NO ------ %d \n", curr_frame_no);
						memcpy(&udp_buffer[index],buffer,n);
						index+=n;
						if(curr_frame_no==frames_expected){
              crc = crc32b(&udp_buffer[REQ_HEAD_MIN_LEN], index - REQ_HEAD_MIN_LEN);
              c1 = crc & 0xff;
              printf("csum %x , crc=%x (%x)\n",udp_buffer[6], crc,  c1);
              if (c1 != udp_buffer[6]) {
      					send_err_resp_header(PACKET_ORDER_LOSS);
			      		state = STATE_WAIT_START;
      					printf("Invalid checksum \n");
                break;
              }

							state = STATE_END_RECVD;

              if(udp_buffer[REQ_EN] == ENCRYP_128_AES_CTR_SN){
                printf("\n 2ENCRYPTION TYPE := %d \n", udp_buffer[REQ_EN]);
                memcpy(buffer_tmp, &udp_buffer[REQ_HEAD_MIN_LEN],index-REQ_HEAD_MIN_LEN-2);
                printf("cid=%d\n",coin_id);
                printf("csn=%d\n",snObj.val32);
//                printf("\n 2before decry n=%d \n", n);
  //              for(i=0;i<index;i++){
    //              printf("%d,",udp_buffer[i]);
      //          }
        //        printf("\n");
                crypt_ctr(encrypt_key,buffer_tmp,index-REQ_HEAD_MIN_LEN-2,nounce);
                memcpy(&udp_buffer[REQ_HEAD_MIN_LEN],buffer_tmp,index-REQ_HEAD_MIN_LEN-2);
                //memcpy(&udp_buffer[REQ_HEAD_MIN_LEN],buffer_tmp,index+n);
                printf("eccc\n");
                for(i=0;i<16;i++)
                  printf("%d,",encrypt_key[i]);
    
                printf("\n nounce \n");
                for(i=0;i<16;i++){
                  printf("%d,",nounce[i]);
                }

                printf("\n");
    
        //        printf("\n 2After decry n=%d \n", n);
          //      for(i=0;i<index;i++){
            //      printf("%d,",udp_buffer[i]);
              //  }
                printf("\n");
              }else if (udp_buffer[REQ_EN] == ENCRYP_128_AES_CTR_KEY_TABLE) {
                //crypt_ctr(encrypt_key,buffer,n,nounce);
              }

            } 
					}						
				}	
			break;			
			case STATE_END_RECVD:

					if(udp_buffer[index-1]!=REQ_END|| udp_buffer[index-2]!=REQ_END){
						send_err_resp_header(INVALID_END_OF_REQ);
						printf("Invalid end of packet  \n");
					}else{
						printf("---------------------END RECVD----------------------------------------------\n");
						printf("---------------------PROCESSING REQUEST-----------------------------\n");
						process_request(index);
					}
					state = STATE_WAIT_START;
			break;
		}
	}
}

//-----------------------------------------------------------
// Processes the UDP packet 
//-----------------------------------------------------------
void process_request(unsigned int packet_len){

    uint16_t cmd_no=0, coin_id,i=0,j=0;
	time_stamp_before = get_time_cs();
	memset(response,0,RESPONSE_HEADER_MAX-1);
	cmd_no = udp_buffer[REQ_CM+1];
	cmd_no |= (((uint16_t)udp_buffer[REQ_CM])<<8);
	coin_id = udp_buffer[REQ_CI+1];
	coin_id |= (((uint16_t)udp_buffer[REQ_CI])<<8);
	
	int b0,b1,b2,b3;
	uint32_t crc;

	if (cmd_no != CMD_VALIDATE && cmd_no != CMD_SYNC_TRANS_ADD && cmd_no != CMD_SYNC_TRANS_DEL && cmd_no != CMD_SYNC_TRANS_RESP && cmd_no != CMD_ECHO && cmd_no != CMD_VERSION) {
		printf("validating encryption/decryption hash\n");

		crc = crc32b(&udp_buffer[REQ_HEAD_MIN_LEN], 12);
		for (i=0; i<16; i++) {
		printf("%d ", udp_buffer[REQ_HEAD_MIN_LEN+i]);
		}

		b0 = (crc >> 24) & 0xff;
		b1 = (crc >> 16) & 0xff;
		b2 = (crc >> 8) & 0xff;
		b3 = (crc) & 0xff;

		printf("h=%x %d %d %d %d\n", crc, b0, b1, b2, b3);
		if (b0 != udp_buffer[REQ_HEAD_MIN_LEN+12] || b1 != udp_buffer[REQ_HEAD_MIN_LEN+13] || b2 != udp_buffer[REQ_HEAD_MIN_LEN+14] || b3 != udp_buffer[REQ_HEAD_MIN_LEN+15]) {
		printf("Hash mismatch\n");
			send_err_resp_header(ENCRYPTION_ERROR);
		return;
		}

	}

    switch(cmd_no) {

        case NFT_TEST_CREATE:                   execute_test_create(packet_len,coin_id); break;
        case NFT_CREATE:                        execute_create(packet_len,coin_id); break;
        case NFT_READ_DATA:                     execute_read_data(packet_len,coin_id); break;
        case NFT_READ_META:                     execute_read_meta(packet_len,coin_id); break;
        default:							    send_err_resp_header(INVALID_CMD);


    }

}

//-----------------------------------------------------------
// Prepare error response and send it.
//-----------------------------------------------------------
void send_err_resp_header(int status_code){
	int len,size=12;
	unsigned char ex_time;
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}
	printf("Error Status code %d  \n",status_code);
	response[RES_RI] = server_config_obj.raida_id;
	response[RES_SH] = 0;
	response[RES_SS] = status_code;
	response[RES_EX] = 0;
	response[RES_RE] = 0;
	response[RES_RE+1] = 0;
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
	len=sizeof(cliaddr);
	sendto(sockfd, (const char *)response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		len);
}
//-----------------------------------------------------------
// Prepare response and send it.
//-----------------------------------------------------------
void prepare_resp_header(unsigned char status_code){
	unsigned char ex_time;
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}
	response[RES_RI] = server_config_obj.raida_id;
	response[RES_SH] = 0;
	response[RES_SS] = status_code;
	response[RES_EX] = ex_time;
	response[RES_RE] = 0;
	response[RES_RE+1] = 0;
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
}
//-----------------------------------------------------------
//  Validate request header
//-----------------------------------------------------------
unsigned char validate_request_header(unsigned char * buff,int packet_size){
	uint16_t frames_expected,i=0,request_header_exp_len= REQ_HEAD_MIN_LEN, coin_id=0;
	/*if(buff[REQ_EN]!=0){
		for(i=1;i<EN_CODES_MAX+1;i++){
			if(EN_CODES[buff[REQ_EN]]>0)
				break;
		}
	}*/
	if(buff[REQ_EN] ==0 || buff[REQ_EN] ==1 || buff[REQ_EN] ==2){
		
	}else{
		return INVALID_EN_CODE;
	}
	request_header_exp_len = REQ_HEAD_MIN_LEN;
	if(packet_size< request_header_exp_len){
		printf("Invalid request header  \n");
		return INVALID_PACKET_LEN;
	}
	frames_expected = buff[REQ_FC+1];
	frames_expected|=(((uint16_t)buff[REQ_FC])<<8);
	printf("No of frames expected :- %d\n", frames_expected);
	if(frames_expected <=0  || frames_expected > FRAMES_MAX){
		printf("Invalid frame count  \n");
		return INVALID_FRAME_CNT;
	}	
	if(buff[REQ_CL]!=0){
		printf("Invalid cloud id \n");
		return INVALID_CLOUD_ID;
	}
	if(buff[REQ_SP]!=0){
		printf("Invalid split id \n");
		return INVALID_SPLIT_ID;
	}
	if(buff[REQ_RI]!=server_config_obj.raida_id){
		printf("Invalid Raida id \n");
		return WRONG_RAIDA;
	}
	if(shards_config[buff[REQ_SH]] ==SHARD_NOT_AVAILABLE_VALUE){
		printf("Shard %d not available \n", buff[REQ_SH] );
		return SHARD_NOT_AVAILABLE;
	}
	coin_id = buff[REQ_CI+1];
	coin_id |= (((uint16_t)buff[REQ_CI])<<8);
	for(i=0;i<coin_id_cnt;i++){
		if(coin_config_obj[i].coin_id==coin_id){
			break;
		}
	}
	if(i>=coin_id_cnt){
		return COIN_ID_NOT_FOUND; 
	}
	return NO_ERR_CODE;
}

//------------------------------------------------------------------------------------------
//  Validate coins and request body and return number of coins 
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body(unsigned int packet_len,unsigned char bytes_per_coin,unsigned int req_body_without_coins,int *req_header_min){
	unsigned int no_of_coins=0;
	*req_header_min = REQ_HEAD_MIN_LEN;// + EN_CODES[udp_buffer[REQ_EN]];	
	no_of_coins = (packet_len-(*req_header_min+req_body_without_coins))/bytes_per_coin;
	if((packet_len-(*req_header_min+req_body_without_coins))%bytes_per_coin!=0){
		send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins==0){
		send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins>COINS_MAX){
		send_err_resp_header(COIN_LIMIT_EXCEED);
		return 0;
	}
	printf("Number of coins = :  %d \n", no_of_coins);	
	return no_of_coins;
}

//------------------------------------------------------------------------------------------
//  Validate coins and request body and return number of coins 
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body_general(unsigned int packet_len,unsigned int req_body,int *req_header_min){
	*req_header_min = REQ_HEAD_MIN_LEN;// + EN_CODES[udp_buffer[REQ_EN]];
	if(packet_len != (*req_header_min) + req_body){
		send_err_resp_header(INVALID_PACKET_LEN);
		return 0;
	}
	return 1;
}
//--------------------------------------------------------------------
//
//-------------------------------------------------------------------
void execute_test_create(unsigned int packet_len, unsigned int coin_id) {

    int req_body_without_coins = CH_BYTES_CNT + CMD_END_BYTES_CNT,bytes_per_coin = SN_BYTES_CNT+AN_BYTES_CNT;
	int req_header_min, no_of_coins,size = 0;
	unsigned int index=0,j=0,pass_cnt=0,fail_cnt=0;
	unsigned char status_code,pass_fail = 0;
	printf("DETECT Command \n");
	no_of_coins = validate_request_body(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins <=0){
		return;
	}
	
		index = req_header_min + CH_BYTES_CNT;
		memset(snObj.data,0,4);
		for(j=0;j<SN_BYTES_CNT;j++)
			snObj.data[j]=udp_buffer[index+(SN_BYTES_CNT-1-j)];

		index +=SN_BYTES_CNT;
		printf("Serial number %d \n", snObj.val32);
		if(snObj.val32 >=coin_id_obj[coin_id].AN_CNT){
			send_err_resp_header(COIN_NO_NOT_FOUND);
			return;
		}
		pass_fail = 1;
		if(memcmp(coin_id_obj[coin_id].AN[snObj.val32],&udp_buffer[index],AN_BYTES_CNT)!=0){
			pass_fail = 0;
			fail_cnt++;
		}
		if(pass_fail == 1){
			pass_cnt++;	
		}			

	status_code = MIX;
	if (fail_cnt == no_of_coins){
		status_code = ALL_FAIL;
	}
    else if(pass_cnt == no_of_coins){
		status_code = ALL_PASS;
	}
	index = RES_HS + HS_BYTES_CNT;
	size = RES_HS + HS_BYTES_CNT;
	if(status_code == MIX){
		for(int i = 0;i < no_of_coins;i++){
			if( pass_fail == 1)
				response[index + (i/8)] |= 1<<(i%8);
		}
		size += no_of_coins/8;
		if((no_of_coins % 8) != 0)
			size++;		
	}
	send_response(status_code,size);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

void execute_create(unsigned int packet_len, unsigned int coin_id) {


}
//-------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void execute_read_data(unsigned int packet_len, unsigned int coin_id) {


}

//-----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void execute_read_meta(unsigned int packet_len, unsigned int coin_id) {


}
