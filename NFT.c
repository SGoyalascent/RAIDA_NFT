#include "udp_socket.h"
#include "library.h"
#include "NFT.h"

unsigned char execpath_meta[256], execpath_data[256];
unsigned char tcp_response[TCP_BUFFER_MAX_SIZE], tcp_buffer[TCP_BUFFER_MAX_SIZE];
union coversion gdObj;
struct coin_id coin_id_obj[255];
struct NFT_TABLE nft_details[NFTS_CNT_MAX] = {0};
unsigned int tcp_port_no = 10;
int tcp_sockfd, tcp_connfd;
struct sockaddr_in tcp_servaddr, tcp_cliaddr;


//--------------------------------------------------
//READ CONFIG FILE and get NFT_Meta and NFT_DATA paths
//--------------------------------------------------
int Read_NFT_Configuration_File() {

    char path[256];
    strcpy(path, execpath);
    strcat(path, "/Data/NFT_config.txt");
    //printf("path: %s\n", path);
    FILE *myfile = fopen(path, "r");
    if(myfile == NULL) {
        perror("Error: agent_config.txt file not found\n");
		return 1;
    }
    fscanf(myfile, "NFT_Data_Path: %255s  NFT_Meta_Path = %255s", execpath_data, execpath_meta);
    fclose(myfile);
    printf("NFT_Data_Path = %s  NFT_Meta_Path = %s \n", execpath_data, execpath_meta);
    return 0;
}

//-----------------------------------------------------------
//Set time out for UDP frames
//-----------------------------------------------------------
void tcp_set_time_out(unsigned char secs){     
	FD_ZERO(&select_fds);            
	FD_SET(tcp_sockfd, &select_fds);           	                                  
	timeout.tv_sec = secs; 
	timeout.tv_usec = 0;
}
//-----------------------------------------------------------
//Initialize TCP Socket and bind to the port
//-----------------------------------------------------------
int init_tcp_socket() {

	//tcp_port_no = server_config_obj.port_number;
	// socket create and verification
	tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_sockfd == -1) {
		perror("TCP_Socket creation failed...\n");
		exit(0);
	}
	printf("TCP_Socket successfully created..\n");
	bzero(&tcp_servaddr, sizeof(tcp_servaddr));

	// assign IP, PORT
	tcp_servaddr.sin_family = AF_INET;
	tcp_servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcp_servaddr.sin_port = htons(tcp_port_no);

	// Binding newly created socket to given IP and verification
	if ((bind(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) != 0) {
		perror("TCP_Socket bind failed...\n");
		exit(0);
	}
	printf("TCP_Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(tcp_sockfd, 5)) != 0) {
		perror("Listen failed on TCP_socket...\n");
		exit(0);
	}
	printf("Server listening on TCP_socket..\n");
  
}
//-----------------------------------------------------------
// receives the TCP packet from the client
//-----------------------------------------------------------
int tcp_listen_request(){

	unsigned char *buffer, state = STATE_WAIT_START,status_code;
	uint16_t curr_frame_no = 0,n = 0,index = 0, frames_expected = 0;
	buffer = (unsigned char *) malloc(TCP_PACKET_SIZE);
	socklen_t len = sizeof(tcp_cliaddr);
	
	while(1){
		switch(state){
			case STATE_WAIT_START:
				printf("---------------------WAITING FOR REQ HEADER on TCP_socket----------------------\n");
				tcp_connfd = accept(tcp_sockfd, (struct sockaddr *)&tcp_cliaddr, &len);
				if (tcp_connfd < 0) {
					perror("Server accept failed on TCP_socket...\n");
					exit(0);
				}
				printf("Client accepted by the Server on TCP_socket...\n");
				index=0;
				//ssize_t recv(int tcp_sockfd, void *buf, size_t len, int flags);
				memset(buffer,0,TCP_PACKET_SIZE);
				n = recv(tcp_connfd, (unsigned char *)buffer, TCP_PACKET_SIZE, MSG_WAITALL);
				curr_frame_no++;
				printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
				state = STATE_START_RECVD;
			break;		
			case STATE_START_RECVD:
				printf("---------------------REQ HEADER RECEIVED on TCP_socket----------------------------\n");
				status_code = validate_request_header(buffer,n);
				if(status_code != NO_ERR_CODE){
					send_err_resp_header(status_code);			
					state = STATE_WAIT_START;
					continue;
				}
				else {
					frames_expected = buffer[REQ_FC+1];
					frames_expected |= (((uint16_t)buffer[REQ_FC])<<8);
					memcpy(tcp_buffer, buffer, n);
					index = n;
					if(frames_expected == 1) {
						state = STATE_END_RECVD;
					}
					else {
						state = STATE_WAIT_END;
					}
				}
			break;
			case STATE_WAIT_END:
				n = recv(tcp_connfd, (unsigned char *)buffer, TCP_PACKET_SIZE, MSG_WAITALL);
				curr_frame_no++;
				printf("--------RECVD  FRAME NO ------ %d \n", curr_frame_no);
				memcpy(&tcp_buffer[index],buffer,n);
				index += n;
				if(frames_expected == curr_frame_no) {
						state = STATE_END_RECVD;
					}
					else {
						state = STATE_WAIT_END;
					}
				
			break;			
			case STATE_END_RECVD:

					if(tcp_buffer[index-1] != REQ_END|| tcp_buffer[index-2] != REQ_END){
						send_err_resp_header(INVALID_END_OF_REQ);
						printf("Invalid end of packet  \n");
					}else{
						printf("---------------------END RECVD- on TCP_socket---------------------------------------------\n");
						printf("Total bytes received: %u bytes\n", index);
						printf("---------------------PROCESSING REQUEST on TCP_socket-----------------------------\n");
						tcp_process_request(index);
					}
					state = STATE_WAIT_START;
			break;
		}
	}
}

//-----------------------------------------------------------
// Processes the UDP packet 
//-----------------------------------------------------------
void tcp_process_request(unsigned int packet_len){

    uint16_t cmd_no = 0, coin_id;
	time_stamp_before = get_time_cs();
	memset(response,0,RESPONSE_HEADER_MAX-1);
	cmd_no = tcp_buffer[REQ_CM+1];
	cmd_no |= (((uint16_t)tcp_buffer[REQ_CM])<<8);
	coin_id = tcp_buffer[REQ_CI+1];
	coin_id |= (((uint16_t)tcp_buffer[REQ_CI])<<8);
	
    switch(cmd_no) {

        case NFT_TEST_CREATE:					execute_test_create(packet_len,coin_id);break;
		case NFT_CREATE:                        execute_create(packet_len,coin_id); break;
		case NFT_UPDATE:                        execute_update(packet_len,coin_id); break;
        case NFT_READ_DATA:                     execute_read_data(packet_len,coin_id); break;
        case NFT_READ_META:                     execute_read_meta(packet_len,coin_id); break;
		case NFT_ADD:							execute_add_coins(packet_len,coin_id);break;		
        default:							    send_err_resp_header(INVALID_CMD);
    }

}

//-----------------------------------------------------------
// Prepare error response and send it.
//-----------------------------------------------------------
void tcp_send_err_resp_header(unsigned char status_code){
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
	response[RES_EC] = tcp_buffer[REQ_EC];
	response[RES_EC+1] = tcp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
	len=sizeof(tcp_cliaddr);
	
	send(tcp_connfd, (const char *)tcp_response, size, MSG_CONFIRM);
}

//-----------------------------------------------------------
// Prepare response and send it.
//-----------------------------------------------------------
void tcp_prepare_resp_header(unsigned char status_code){
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
	response[RES_EC] = tcp_buffer[REQ_EC];
	response[RES_EC+1] = tcp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
}

//---------------------------------------------------------------
//	SEND RESPONSE
//---------------------------------------------------------------
void tcp_send_response(unsigned char status_code,unsigned int size){
	int len=sizeof(tcp_cliaddr);
	tcp_prepare_resp_header(status_code);
	
	send(tcp_connfd, (const char *)tcp_response, size, MSG_CONFIRM);
}

//-----------------------------------------------------------
//  Validate request header
//-----------------------------------------------------------
unsigned char tcp_validate_request_header(unsigned char * buff,int packet_size) {
	uint16_t frames_expected,i=0,request_header_exp_len= REQ_HEAD_MIN_LEN, coin_id=0;

	if(buff[REQ_EN] ==0 || buff[REQ_EN] ==1 || buff[REQ_EN] ==2){
		
	}
	else{
		return INVALID_EN_CODE;
	}
	if(packet_size< request_header_exp_len) {
		printf("Invalid request header  \n");
		return INVALID_PACKET_LEN;
	}
	frames_expected = buff[REQ_FC+1];
	frames_expected |= (((uint16_t)buff[REQ_FC])<<8);
	printf("Frames expected :- %d  , 2KB/frame\n", frames_expected);  

	if(frames_expected <=0  || frames_expected > TCP_FRAMES_MAX) {
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
unsigned int validate_request_body_only_coins(unsigned int packet_len,unsigned char bytes_per_coin,unsigned int req_body_without_coins,int *req_header_min){
	unsigned int no_of_coins = 0;
	*req_header_min = REQ_HEAD_MIN_LEN;   // + EN_CODES[tcp_buffer[REQ_EN]];	
	no_of_coins = (packet_len-(*req_header_min+req_body_without_coins))/bytes_per_coin;
	printf("Number of coins = :  %d \n", no_of_coins);	

	if((packet_len - (*req_header_min + req_body_without_coins))%bytes_per_coin !=0 ){
		tcp_send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins == 0){
		tcp_send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins > 1){
		tcp_send_err_resp_header(COIN_LIMIT_EXCEED);
		return 0;
	}
	return no_of_coins;
}

//------------------------------------------------------------------------------------------
//  Validate the request body for the TEST CREATE, CREATE, UPDATE SERVICE and Return the no. of coins
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body_nfts(unsigned int packet_len,unsigned int index){
	unsigned int coin_bytes = 0, no_of_coins = 0, bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;

	printf("--------Validating Request body-------\n");
	while(index < packet_len) {
		if((tcp_buffer[index] == 0xAB) && (tcp_buffer[index+1] == 0xCD) && (tcp_buffer[index+2] == 0xEF)) {
			break;
		}
		if( (tcp_buffer[index] == 0x3E) && (tcp_buffer[index+1] == 0x3E) ) {
			break;
		}
		coin_bytes++;
		index++;
	}
	no_of_coins = coin_bytes/bytes_per_coin;

	if( (coin_bytes%bytes_per_coin != 0) || no_of_coins == 0) {
		tcp_send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	printf("-----Request Body Validated------\n");

	return no_of_coins;
}

//-----------------------------------------------------------
// CHECK if GUID IS UNIQUE
//-----------------------------------------------------------
unsigned char isGuidUnique(unsigned char *guid) {
	struct dirent *dir; 
    struct stat statbuf;
	DIR *d_data, *d_meta;
	unsigned int files_count = 0;

	//Check in NFT_Data directory
    d_data = opendir(execpath_data); 
	files_count = 0;
    if(d_data == NULL) {
        printf("Error: %s\n", strerror(errno));
		return 2;  
    }
    while ((dir = readdir(d_data)) != NULL) 
    {
		char df_path[512], df_name[256];
        sprintf(df_name, "%s",dir->d_name);
        sprintf(df_path, "%s/%s", execpath_data, dir->d_name);
		printf("\nfilename: %s  filepath: %s\n", df_name, df_path);

		if(dir->d_type == DT_REG) {
            char *token;
            if(stat(df_path, &statbuf) == -1) {
                printf("Error: %s\n", strerror(errno));
                continue;
            }
			token = strtok(df_name, ".");
			if(strcmp(token, guid) == 0) {   // guid already exists
                return 0;
            }
		}
	}
	closedir(d_data);

	//Check in NFT_Meta directory
	d_meta = opendir(execpath_meta); 
	files_count = 0;
	if(d_meta == NULL) {
        printf("Error: %s\n", strerror(errno));
		return 2;  
    }
	while ((dir = readdir(d_meta)) != NULL) 
    {
		char df_path[512], df_name[256];
        sprintf(df_name, "%s",dir->d_name);
        sprintf(df_path, "%s/%s", execpath_data, dir->d_name);
		printf("\nfilename: %s  filepath: %s\n", df_name, df_path);

		if(dir->d_type == DT_REG) {
            char *token;
            if(stat(df_path, &statbuf) == -1) {
                printf("Error: %s\n", strerror(errno));
                continue;
            }
			token = strtok(df_name, ".");
			if(strcmp(token, guid) == 0) {   // guid already exists
                return 0;
            }
		}
	}
	closedir(d_meta);
	return 1;
}

//----------------------------------------------------------------
//AUTHENTICATE THE NO. OF COINS
//-----------------------------------------------------------------
unsigned char authenticate_coins(unsigned int no_of_coins, unsigned int index, unsigned int coin_id) {

	printf("------Authenticating the Coins-----\n");
	unsigned int fail_cnt = 0;
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		index += SN_BYTES_CNT;
		printf("Serial number: %d \n", gdObj.val32);
		if(gdObj.val32 >= coin_id_obj[coin_id].AN_CNT){
			tcp_send_err_resp_header(COIN_NO_NOT_FOUND);
			return 2;
		}
		if(memcmp(coin_id_obj[coin_id].AN[gdObj.val32],&tcp_buffer[index],AN_BYTES_CNT) !=0 ){
			fail_cnt++;
			break;
		}
		index += AN_BYTES_CNT;
	}

	return fail_cnt;
}
//--------------------------------------------------------------------
//TEST SERVICE TO CREATE NEW NFT's
//-------------------------------------------------------------------
void execute_test_create(unsigned int packet_len, int coin_id) {

    int bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT, req_header_min = REQ_HEAD_MIN_LEN, no_of_coins = 0, size = 0;
	unsigned int index = 0, fail_cnt = 0, SN_no, meta_index, meta_bytes_cnt = 0, data_index, data_bytes_cnt = 0;
	unsigned char status_code, guid_bytes[GUID_BYTES_CNT] = {0};
	time_t now = time(0);
	struct tm *t = gmtime(&now);

	printf("--------------------------TEST CREATE SERVICE--------------------------\n");

	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	//-------Validate the Request body and get the no. of coins-----------
	
	no_of_coins = validate_request_body_nfts(packet_len, index);
	if(no_of_coins == 0) {
		printf("Error: Request Body not Validated\n");
		return;
	}
	//--------------------get the guid no.---------------------------
	index = req_header_min + CH_BYTES_CNT;
	printf("GUID_received: ");
	for(int i = 0; i < GUID_BYTES_CNT; i++) {
		guid_bytes[i] = tcp_buffer[index+i];
		printf("%d ", guid_bytes[i]);
	}
	printf("\n");
	index += GUID_BYTES_CNT;

	unsigned char guid_hex_bytes[2*GUID_BYTES_CNT + 1];
	char* ptr =  &guid_hex_bytes[0];
	for(int j=0;j < GUID_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", guid_bytes[j]);
	}
	printf("GUID_hex.:- %s\n", guid_hex_bytes); 

//------------check if guid is unique/exists or not by cross-checking with the nft_table-------------------
	unsigned char flag_guid = 1;
	for(int i = 0; i < NFTS_CNT_MAX; i++) {
		if(memcmp(nft_details[i].GUID, guid_bytes, GUID_BYTES_CNT) == 0) {
			flag_guid = 0;
			break;
		}
	}
	if(flag_guid == 0) {
		printf("Error: GUID already exits in the NFT Table\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_GUID_NOT_UNIQUE;
		tcp_send_response(status_code,size);
		return;
	}
	printf("GUID is unique in the NFT Table\n");

//---------------check if the guid is unique/exists by looking in the NFT files directory-------------
	flag_guid = isGuidUnique(guid_bytes);
	if(flag_guid  == 0) {
		printf("Error: GUID already exits in the NFTs Directory\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_GUID_NOT_UNIQUE;
		tcp_send_response(status_code,size);
		return;
	}
	printf("GUID is unique in the NFTs directory\n");

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id);

	//send response if coins(all or few) counterfiet
	if(fail_cnt == 0) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_CREATE_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");
	//index += no_of_coins*bytes_per_coin;
//-----------------------Check if Coins already have NFTs--------------------------------------
	printf("------Checking if NFTs already exists for the Coins-----\n");
	fail_cnt = 0;
	unsigned char temp[GUID_BYTES_CNT] = {0};
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		SN_no = gdObj.val32;
		printf("Serial number: %d \n", SN_no);
		if(memcmp(nft_details[SN_no].GUID,temp,GUID_BYTES_CNT) != 0) {
			fail_cnt++;
			break;
		}
		index += SN_BYTES_CNT +AN_BYTES_CNT;
	}
	if(fail_cnt > 0) {
		printf("Error: NFTs already Exists for the Coins\n");
		status_code = NFT_CREATE_WARNING_NFT_ALREADY_EXISTS_FOR_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}

	printf("------NFTs not exists for all the Coins--------\n");
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT + (no_of_coins*bytes_per_coin);

//---------------------------------get the nft meta bytes-------------------------------------
	printf("-----Reading NFT Meta bytes from the Request Body-----\n");
	index += REQ_BODY_SEPERATOR_BYTES_CNT;
	meta_index = index, meta_bytes_cnt = 0;
	unsigned char error_flag = 1;

	while(meta_index <= packet_len) {
		if( (tcp_buffer[meta_index] == 0xAB) && (tcp_buffer[meta_index+1] == 0xCD) && (tcp_buffer[meta_index+2] == 0xEF) ) {
			break;
		}
		if( (tcp_buffer[meta_index] == 0x3E) && (tcp_buffer[meta_index+1] == 0x3E) ) {
			error_flag = 0;
			break;
		}
		meta_index++;
	}
	meta_bytes_cnt = meta_index - index;
	//----------if meta or data is not in the request body---------
	if(meta_bytes_cnt == 0 || error_flag == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	//unsigned char meta_bytes[meta_bytes_cnt];
	//memcpy(meta_bytes, &tcp_buffer[index], meta_bytes_cnt);
	//index += (meta_bytes_cnt + REQ_BODY_SEPERATOR_BYTES_CNT);
	index = meta_index + REQ_BODY_SEPERATOR_BYTES_CNT;
	printf("-----NFT Meta bytes read successfully from the Request Body-----\n");

//---------------------------------get the nft data bytes---------------------------------------------
	printf("-----Reading NFT Data bytes from the Request Body-----\n");
	data_index = index, data_bytes_cnt = 0;
	while(data_index < packet_len) {
		if( (tcp_buffer[data_index] == 0x3E) && (tcp_buffer[data_index+1] == 0x3E) ) {
			break;
		}
		data_index++;
	}
	data_bytes_cnt = data_index - index;
	//---------if meta or data is not in the request body---------
	if(data_bytes_cnt == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	//unsigned char data_bytes[data_bytes_cnt];
	//memcpy(data_bytes, &tcp_buffer[index], data_bytes_cnt);
	//index += data_bytes_cnt;
	index = data_index + CMD_END_BYTES_CNT;
	printf("-----NFT Data bytes read successfully from the Request Body-----\n");

//-----------------if nft is greater than 8MB(per coin) for all coins combined-----------------
	unsigned int nft_bytes_cnt = data_bytes_cnt + meta_bytes_cnt;
	if( nft_bytes_cnt > (NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE*no_of_coins) ) {
		printf("Error: Total NFT bytes greater than 8MB\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_EXCESS_DATA_FOR_COINS;
		tcp_send_response(status_code,size);
		return;
	}
	printf("NFT is less than/equal to 8MB\n");

//-------------------SINCE no Error is present, sent Success Response------------------------------

	printf("NFTs can be created successfully\n");
	size = RES_HS + HS_BYTES_CNT;
	status_code = SUCCESS;
	tcp_send_response(status_code,size);
}

//----------------------------------------------------------------------
//SERVICE TO CREATE NEW NFT's
//----------------------------------------------------------------------
void execute_create(unsigned int packet_len, unsigned int coin_id) {
	
	int bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT, req_header_min = REQ_HEAD_MIN_LEN, no_of_coins = 0, size = 0;
	unsigned int index = 0, fail_cnt = 0, SN_no, meta_index, meta_bytes_cnt = 0, data_index, data_bytes_cnt = 0;
	unsigned char status_code, guid_bytes[GUID_BYTES_CNT] = {0};
	time_t now = time(0);
	struct tm *t = gmtime(&now);

	printf("--------------------------CREATE SERVICE--------------------------\n");

	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	//-------Validate the Request body and get the no. of coins-----------
	
	no_of_coins = validate_request_body_nfts(packet_len, index);
	if(no_of_coins == 0) {
		printf("Error: Request Body not Validated\n");
		return;
	}
	//--------------------get the guid no.---------------------------
	index = req_header_min + CH_BYTES_CNT;
	printf("GUID_received: ");
	for(int i = 0; i < GUID_BYTES_CNT; i++) {
		guid_bytes[i] = tcp_buffer[index+i];
		printf("%d ", guid_bytes[i]);
	}
	printf("\n");
	index += GUID_BYTES_CNT;

	unsigned char guid_hex_bytes[2*GUID_BYTES_CNT + 1];
	char* ptr =  &guid_hex_bytes[0];
	for(int j=0;j < GUID_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", guid_bytes[j]);
	}
	printf("GUID_hex.:- %s\n", guid_hex_bytes); 

//------------check if guid is unique/exists or not by cross-checking with the nft_table-------------------
	unsigned char flag_guid = 1;
	for(int i = 0; i < NFTS_CNT_MAX; i++) {
		if(memcmp(nft_details[i].GUID, guid_bytes, GUID_BYTES_CNT) == 0) {
			flag_guid = 0;
			break;
		}
	}
	if(flag_guid == 0) {
		printf("Error: GUID already exits in the NFT Table\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_GUID_NOT_UNIQUE;
		tcp_send_response(status_code,size);
		return;
	}
	printf("GUID is unique in the NFT Table\n");

//---------------check if the guid is unique/exists by looking in the NFT files directory-------------
	flag_guid = isGuidUnique(guid_hex_bytes);
	if(flag_guid  == 0) {
		printf("Error: GUID already exits in the NFT files Directory\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_GUID_NOT_UNIQUE;
		tcp_send_response(status_code,size);
		return;
	}
	printf("GUID is unique in the NFTs Directory\n");

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id);

	//send response if coins(all or few) counterfiet
	if(fail_cnt == 1) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_CREATE_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");
	//index += no_of_coins*bytes_per_coin;
//-----------------------Check if Coins already have NFTs--------------------------------------
	
	printf("------Checking if NFTs already exists for the Coins-----\n");
	fail_cnt = 0;
	unsigned char temp[GUID_BYTES_CNT] = {0};
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		SN_no = gdObj.val32;
		printf("Serial number: %d \n", SN_no);
		if(memcmp(nft_details[SN_no].GUID,temp,GUID_BYTES_CNT) != 0) {
			fail_cnt++;
			break;
		}
		index += bytes_per_coin;
	}

	if(fail_cnt > 0) {
		printf("Error: NFTs already Exists for the Coins\n");
		status_code = NFT_CREATE_WARNING_NFT_ALREADY_EXISTS_FOR_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}

	printf("------NFTs not exists for all the Coins--------\n");
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT + (no_of_coins*bytes_per_coin);

//---------------------------------get the nft meta bytes-------------------------------------
	printf("-----Reading NFT Meta bytes from the Request Body-----\n");
	index += REQ_BODY_SEPERATOR_BYTES_CNT;
	meta_index = index, meta_bytes_cnt = 0;
	unsigned char error_flag = 1;

	while(meta_index <= packet_len) {
		if( (tcp_buffer[meta_index] == 0xAB) && (tcp_buffer[meta_index+1] == 0xCD) && (tcp_buffer[meta_index+2] == 0xEF) ) {
			break;
		}
		if( (tcp_buffer[meta_index] == 0x3E) && (tcp_buffer[meta_index+1] == 0x3E) ) {
			error_flag = 0;
			break;
		}
		meta_index++;
	}
	meta_bytes_cnt = meta_index - index;
	//----------if meta or data is not in the request body---------
	if(meta_bytes_cnt == 0 || error_flag == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	unsigned char meta_bytes[meta_bytes_cnt];
	memcpy(meta_bytes, &tcp_buffer[index], meta_bytes_cnt);
	index = meta_index + REQ_BODY_SEPERATOR_BYTES_CNT;
	printf("-----NFT Meta bytes read successfully from the Request Body-----\n");

//---------------------------------get the nft data bytes---------------------------------------------
	printf("-----Reading NFT Data bytes from the Request Body-----\n");
	data_index = index, data_bytes_cnt = 0;
	while(data_index < packet_len) {
		if( (tcp_buffer[data_index] == 0x3E) && (tcp_buffer[data_index+1] == 0x3E) ) {
			break;
		}
		data_index++;
	}
	data_bytes_cnt = data_index - index;
	//---------if meta or data is not in the request body---------
	if(data_bytes_cnt == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	unsigned char data_bytes[data_bytes_cnt];
	memcpy(data_bytes, &tcp_buffer[index], data_bytes_cnt);
	index = data_index + CMD_END_BYTES_CNT;
	printf("-----NFT Data bytes read successfully from the Request Body-----\n");

//-----------------if nft data is greater than 8MB(per coin) for all coins combined-----------------
	unsigned int nft_bytes_cnt = data_bytes_cnt + meta_bytes_cnt;
	if( nft_bytes_cnt > (NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE*no_of_coins) ) {
		printf("Error: Total NFT bytes greater than 8MB\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_EXCESS_DATA_FOR_COINS;
		tcp_send_response(status_code,size);
		return;
	}
	printf("NFT is less than/equal to 8MB\n");	

//-----------------------------------------------------------------------------------------

	unsigned char nft_file_name[256], file_path_data[256], file_path_meta[256];
	
	strcpy(nft_file_name, guid_hex_bytes);
	strcat(nft_file_name, ".bin");

	strcpy(file_path_data, execpath_data);
	strcat(file_path_data, "/");
	strcat(file_path_data, nft_file_name);

	strcpy(file_path_meta, execpath_meta);
	strcat(file_path_meta, "/");
	strcat(file_path_meta, nft_file_name);

	FILE *fp_inp = NULL;
//-------------------------------------create new nft data file-----------------------------------
	if ((fp_inp = fopen(file_path_data, "wb")) == NULL) {
      	printf("%s\n",file_path_data);
     	perror("Cannot be opened , exiting \n");
      	size = RES_HS + HS_BYTES_CNT;
		status_code = FAIL;
		tcp_send_response(status_code,size);
	  	return;
    } 
	fwrite(data_bytes, data_bytes_cnt, 1, fp_inp);
	fclose(fp_inp);	
	printf("NFT Data file created\n");

//--------------------------------create new nft meta file------------------------------------
	fp_inp = NULL;
	if ((fp_inp = fopen(file_path_meta, "wb")) == NULL) {
		printf("%s\n",file_path_meta);
		perror("Cannot be opened , exiting \n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = FAIL;
		tcp_send_response(status_code,size);
	  	return;
    } 
	fwrite(meta_bytes, meta_bytes_cnt, 1, fp_inp);
	fclose(fp_inp);
	printf("NFT Meta file created\n");

//------------------------store the guid, mfs, sn and an's in the the nft table------------------------------
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		index += SN_BYTES_CNT;
		SN_no = gdObj.val32;
		
		nft_details[SN_no].SN = SN_no;									//add SN to the NFT table
		memcpy(nft_details[SN_no].AN,&tcp_buffer[index],AN_BYTES_CNT);  //add AN to the NFT table
		memcpy(nft_details[SN_no].GUID,guid_bytes,GUID_BYTES_CNT);		//add GUID to the NFT table
		nft_details[SN_no].MFS = t->tm_mon+1;
		index += AN_BYTES_CNT;
		
		printf("SN: %u\n", nft_details[SN_no].SN);
		printf("AN: ");
		for(int j = 0; j < AN_BYTES_CNT; j++) {
			printf("%d ",nft_details[SN_no].AN[j]);
		}
		printf("\n");
		printf("GUID: ");
		for(int j = 0; j < GUID_BYTES_CNT; j++) {
			printf("%d ",nft_details[SN_no].GUID[j]);
		}
		printf("\n");
	}
	printf("GUID, Coins stored in the NFT Table\n");
	
//---------------------------------------------------------------------------------------
	printf("---- NFTs created Successfully for all the Coins----\n");
	size = RES_HS + HS_BYTES_CNT;
	status_code = NFT_CREATE_SUCCESS;
	tcp_send_response(status_code,size);
}

//----------------------------------------------------------------------
//SERVICE TO UPDATE OLD NFT's
//----------------------------------------------------------------------
void execute_update(unsigned int packet_len, unsigned int coin_id) {
	
	int bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT, req_header_min = REQ_HEAD_MIN_LEN, no_of_coins = 0, size = 0;
	unsigned int index = 0, fail_cnt = 0, SN_no, meta_index, meta_bytes_cnt = 0, data_index, data_bytes_cnt = 0;
	unsigned char status_code, guid_bytes[GUID_BYTES_CNT] = {0};
	time_t now = time(0);
	struct tm *t = gmtime(&now);

	printf("--------------------------UPDATE SERVICE--------------------------\n");

	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	//-------Validate the Request body and get the no. of coins-----------
	
	no_of_coins = validate_request_body_nfts(packet_len, index);
	if(no_of_coins == 0) {
		printf("Error: Request Body not Validated\n");
		return;
	}
	//--------------------get the guid no.---------------------------
	index = req_header_min + CH_BYTES_CNT;
	printf("GUID_received: ");
	for(int i = 0; i < GUID_BYTES_CNT; i++) {
		guid_bytes[i] = tcp_buffer[index+i];
		printf("%d ", guid_bytes[i]);
	}
	printf("\n");
	index += GUID_BYTES_CNT;

	unsigned char guid_hex_bytes[2*GUID_BYTES_CNT + 1];
	char* ptr =  &guid_hex_bytes[0];
	for(int j=0;j < GUID_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", guid_bytes[j]);
	}
	printf("GUID_hex.:- %s\n", guid_hex_bytes); 

//------------check if guid is exists or not by cross-checking with the nft_table-------------------
	unsigned char flag_guid = 0;
	for(int i = 0; i < NFTS_CNT_MAX; i++) {
		if(memcmp(nft_details[i].GUID, guid_bytes, GUID_BYTES_CNT) == 0) {
			flag_guid = 1;
			break;
		}
	}
	if(flag_guid == 0) {
		printf("Error: GUID not exits in the NFT Table\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_UPDATE_GUID_NOT_FOUND;
		tcp_send_response(status_code,size);
		return;
	}
	printf("GUID exists in the NFT Table\n");

//---------------check if the guid is unique/exists by looking in the NFT files directory-------------
	flag_guid = isGuidUnique(guid_hex_bytes);
	if(flag_guid == 1) {
		printf("Error: GUID not exits in the NFT files Directory\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_UPDATE_GUID_NOT_FOUND;
		tcp_send_response(status_code,size);
		return;
	}
	else if(flag_guid == 0) {
		printf("GUID exists in the NFTs Directory\n");
	}

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id);

	//send response if coins(all or few) counterfiet
	if(fail_cnt == 0) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_CREATE_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");
	index += no_of_coins*bytes_per_coin;
/*
//-----------------------Check if Coins already have NFTs--------------------------------------
	
	printf("------Checking if NFTs already exists for the Coins-----\n");
	fail_cnt = 0;
	unsigned char temp[GUID_BYTES_CNT] = {0};
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		SN_no = gdObj.val32;
		printf("Serial number: %d \n", SN_no);
		if(memcmp(nft_details[SN_no].GUID,temp,GUID_BYTES_CNT) != 0) {
			fail_cnt++;
			break;
		}
		index += bytes_per_coin;
	}
	if(fail_cnt > 0) {
		printf("Error: NFTs already Exists for the Coins\n");
		status_code = NFT_CREATE_WARNING_NFT_ALREADY_EXISTS_FOR_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}

	printf("------NFTs not exists for all the Coins--------\n");
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT + (no_of_coins*bytes_per_coin);
*/

//---------------------------------get the nft meta bytes-------------------------------------
	printf("-----Reading NFT Meta bytes from the Request Body-----\n");
	index += REQ_BODY_SEPERATOR_BYTES_CNT;
	meta_index = index, meta_bytes_cnt = 0;
	unsigned char error_flag = 1;

	while(meta_index <= packet_len) {
		if( (tcp_buffer[meta_index] == 0xAB) && (tcp_buffer[meta_index+1] == 0xCD) && (tcp_buffer[meta_index+2] == 0xEF) ) {
			break;
		}
		if( (tcp_buffer[meta_index] == 0x3E) && (tcp_buffer[meta_index+1] == 0x3E) ) {
			error_flag = 0;
			break;
		}
		meta_index++;
	}
	meta_bytes_cnt = meta_index - index;
	//----------if meta or data is not in the request body---------
	if(meta_bytes_cnt == 0 || error_flag == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	unsigned char meta_bytes[meta_bytes_cnt];
	memcpy(meta_bytes, &tcp_buffer[index], meta_bytes_cnt);
	index = meta_index + REQ_BODY_SEPERATOR_BYTES_CNT;
	printf("-----NFT Meta bytes read successfully from the Request Body-----\n");

//---------------------------------get the nft data bytes---------------------------------------------
	printf("-----Reading NFT Data bytes from the Request Body-----\n");
	data_index = index, data_bytes_cnt = 0;
	while(data_index < packet_len) {
		if( (tcp_buffer[data_index] == 0x3E) && (tcp_buffer[data_index+1] == 0x3E) ) {
			break;
		}
		data_index++;
	}
	data_bytes_cnt = data_index - index;
	//---------if meta or data is not in the request body---------
	if(data_bytes_cnt == 0) {
		printf("Error: Data or Meta bytes not exists in the Request Body\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_WARNING_NO_DATA_OR_META_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

	unsigned char data_bytes[data_bytes_cnt];
	memcpy(data_bytes, &tcp_buffer[index], data_bytes_cnt);
	index = data_index + CMD_END_BYTES_CNT;
	printf("-----NFT Data bytes read successfully from the Request Body-----\n");

//-----------------if nft data is greater than 8MB(per coin) for all coins combined-----------------
	unsigned int nft_bytes_cnt = data_bytes_cnt + meta_bytes_cnt;
	if( nft_bytes_cnt > (NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE*no_of_coins) ) {
		printf("Error: Total NFT bytes greater than 8MB\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_CREATE_EXCESS_DATA_FOR_COINS;
		tcp_send_response(status_code,size);
		return;
	}
	printf("NFT is less than/equal to 8MB\n");

//----------------------------------------------------------------------------------------------	

	unsigned char nft_file_name[256], file_path_data[256], file_path_meta[256];

	strcpy(nft_file_name, guid_bytes);
	strcat(nft_file_name, ".bin");

	strcpy(file_path_data, execpath_data);
	strcat(file_path_data, "/");
	strcat(file_path_data, nft_file_name);

	FILE *fp_inp = NULL;
//-------------------------------------update nft data file-----------------------------------
	if ((fp_inp = fopen(file_path_data, "wb+")) == NULL) {
		printf("%s\n",file_path_data);
		printf("Cannot be opened , exiting \n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = FAIL;
		tcp_send_response(status_code,size);
		return;
    } 
	fwrite(data_bytes, data_bytes_cnt, 1, fp_inp);
	fclose(fp_inp);	

//--------------------------------update nft meta file------------------------------------
	fp_inp = NULL;
	if ((fp_inp = fopen(file_path_meta, "wb+")) == NULL) {
		printf("%s\n",file_path_meta);
		printf("Cannot be opened , exiting \n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = FAIL;
		tcp_send_response(status_code,size);
		return;
    } 
	fwrite(meta_bytes, meta_bytes_cnt, 1, fp_inp);
	fclose(fp_inp);

//------------------------store the guid, mfs, sn and an's in the the nft table------------------------------
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		index += SN_BYTES_CNT;
		SN_no = gdObj.val32;
		
		nft_details[SN_no].SN = gdObj.val32;
		memcpy(nft_details[SN_no].AN,&tcp_buffer[index],AN_BYTES_CNT);
		memcpy(nft_details[SN_no].GUID,guid_bytes,GUID_BYTES_CNT);
		nft_details[SN_no].MFS = t->tm_mon+1;
		index += AN_BYTES_CNT;
		
		printf("SN: %d\n", nft_details[SN_no].SN);
		printf("AN: ");
		for(int j = 0; j < AN_BYTES_CNT; j++) {
			printf("%c ",nft_details[SN_no].AN[j]);
		}
		printf("\n");
		printf("GUID: ");
		for(int j = 0; j < GUID_BYTES_CNT; j++) {
			printf("%c ",nft_details[SN_no].GUID[j]);
		}
		printf("\n");
	}
	printf("GUID, Coins stored in the NFT Table\n");
//--------------------------------------------------------------------------------------
	
	printf("NFTs Updated Successfully\n");
	size = RES_HS + HS_BYTES_CNT;
	status_code = SUCCESS;
	tcp_send_response(status_code,size);

}

//-------------------------------------------------------------------------
//READ NFT DATA FILE
//------------------------------------------------------------------------
void execute_read_data(unsigned int packet_len, unsigned int coin_id) {
	
	int req_body_without_coins = CH_BYTES_CNT + CMD_END_BYTES_CNT, bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;
	unsigned int index = 0, SN_no, req_header_min, no_of_coins, size = 0, fail_cnt = 0;
	unsigned char status_code, nft_file_name[256], file_path_data[256], guid_bytes[GUID_BYTES_CNT] = {0};
	
	printf("-------------------------READ DATA SERVICE-----------------\n");
	no_of_coins = validate_request_body_only_coins(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins == 0){
		printf("Error: Request Body not Validated\n");
		return;
	}

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id); 
	//send response if coins(all or few) counterfiet
	if(fail_cnt == 1) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_CREATE_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");

//----------------Get the GUID for the coin from the NFT Table----------------------
	index = req_header_min + CH_BYTES_CNT;
	for(int j = 0; j < SN_BYTES_CNT; j++) {
		gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
	}
	SN_no = gdObj.val32;
	index += bytes_per_coin;
	memcpy(guid_bytes, nft_details[SN_no].GUID, GUID_BYTES_CNT);
	
	unsigned char guid_hex_bytes[2*GUID_BYTES_CNT + 1];
	char* ptr =  &guid_hex_bytes[0];
	for(int j=0;j < GUID_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", guid_bytes[j]);
	}
	printf("GUID_hex.:- %s\n", guid_hex_bytes); 

//------------------Update the NFT Data File-------------------------------------

	strcpy(nft_file_name, guid_hex_bytes);
	strcat(nft_file_name, ".bin");

	strcpy(file_path_data, execpath_data);
	strcat(file_path_data, "/");
	strcat(file_path_data, nft_file_name);

	FILE *fp_inp = NULL;
	unsigned int no_of_bytes = 0;
	unsigned char data_bytes[NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE];
	if ((fp_inp = fopen(file_path_data, "rb")) == NULL) {
		printf("%s\n",file_path_data);
		printf("Cannot be opened , exiting \n");
		status_code = FAIL;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
	  	return;
    } 
	
	fseek(fp_inp, 0L, SEEK_END);
    no_of_bytes = ftell(fp_inp);
    fseek(fp_inp,no_of_bytes,SEEK_SET);	
	fread(data_bytes, no_of_bytes, 1, fp_inp);
	fclose(fp_inp);	

	index = RES_HS + HS_BYTES_CNT;
	memcpy(&tcp_response[index], data_bytes, no_of_bytes);
	index += no_of_bytes;
	tcp_response[index] = 0x3E;
	tcp_response[index+1] = 0x3E;

	status_code = SUCCESS;
	size = index +  CMD_END_BYTES_CNT;
	tcp_send_response(status_code,size);

}

//-----------------------------------------------------------------------------
//READ NFT META FILE
//----------------------------------------------------------------------------
void execute_read_meta(unsigned int packet_len, unsigned int coin_id) {
	int req_body_without_coins = CH_BYTES_CNT + CMD_END_BYTES_CNT, bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;
	unsigned int index = 0, SN_no, req_header_min, no_of_coins, size = 0, fail_cnt = 0;
	unsigned char status_code, nft_file_name[256], file_path_meta[256], guid_bytes[GUID_BYTES_CNT] = {0};
	
	printf("-------------------------READ META SERVICE-----------------\n");
	no_of_coins = validate_request_body_only_coins(packet_len,bytes_per_coin,req_body_without_coins,&req_header_min);
	if(no_of_coins == 0){
		printf("Error: Request Body not Validated\n");
		return;
	}

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id); 
	//send response if coins(all or few) counterfiet
	if(fail_cnt == 1) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_CREATE_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");

//----------------Get the GUID for the coin from the NFT Table----------------------
	index = req_header_min + CH_BYTES_CNT;
	for(int j = 0; j < SN_BYTES_CNT; j++) {
		gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
	}
	SN_no = gdObj.val32;
	index += bytes_per_coin;
	memcpy(guid_bytes, nft_details[SN_no].GUID, GUID_BYTES_CNT);
	
	unsigned char guid_hex_bytes[2*GUID_BYTES_CNT + 1];
	char* ptr =  &guid_hex_bytes[0];
	for(int j=0;j < GUID_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", guid_bytes[j]);
	}
	printf("GUID_hex.:- %s\n", guid_hex_bytes);

//------------------Update the NFT Data File-------------------------------------
	strcpy(nft_file_name, guid_bytes);
	strcat(nft_file_name, ".bin");

	strcpy(file_path_meta, execpath_meta);
	strcat(file_path_meta, "/");
	strcat(file_path_meta, nft_file_name);

	FILE *fp_inp = NULL;
	unsigned int no_of_bytes = 0;
	unsigned char meta_bytes[NFT_DATA_PLUS_META_PER_COIN_MAX_BYTES_SIZE];
	if ((fp_inp = fopen(file_path_meta, "rb")) == NULL) {
		printf("%s\n",file_path_meta);
		printf("Cannot be opened , exiting \n");
		status_code = FAIL;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
    } 
	
	fseek(fp_inp, 0L, SEEK_END);
    no_of_bytes = ftell(fp_inp);
    fseek(fp_inp,no_of_bytes,SEEK_SET);	
	fread(meta_bytes, no_of_bytes, 1, fp_inp);
	fclose(fp_inp);	

	index = RES_HS + HS_BYTES_CNT;
	memcpy(&tcp_response[index], meta_bytes, no_of_bytes);
	index += no_of_bytes;
	tcp_response[index] = 0x3E;
	tcp_response[index+1] = 0x3E;

	status_code = SUCCESS;
	size = index +  CMD_END_BYTES_CNT;
	tcp_send_response(status_code,size);

}

//-----------------------------------------------------------------------------------------
//ADD COINS in the NFT Table
//-----------------------------------------------------------------------------------------
void execute_add_coins(unsigned int packet_len, unsigned int coin_id) {

    int req_body_without_coins = CH_BYTES_CNT + CMD_END_BYTES_CNT, bytes_per_coin = SN_BYTES_CNT + AN_BYTES_CNT;
	unsigned int req_header_min = REQ_HEAD_MIN_LEN, no_of_coins = 0, size = 0, index = 0, fail_cnt = 0, SN_no;
	unsigned char status_code, guid_bytes[GUID_BYTES_CNT] = {0};

	printf("--------------------------ADD COINS SERVICE--------------------------\n");

	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	//-------Validate the Request body and get the no. of coins-----------
	
	no_of_coins = validate_request_body_nfts(packet_len, index);
	if(no_of_coins == 0) {
		printf("Error: Request Body not Validated\n");
		return;
	}
	//--------------------get the guid no.---------------------------
	index = req_header_min + CH_BYTES_CNT;
	printf("GUID_received: ");
	for(int i = 0; i < GUID_BYTES_CNT; i++) {
		guid_bytes[i] = tcp_buffer[index+i];
		printf("%c", guid_bytes[i]);
	}
	printf("\n");
	index += GUID_BYTES_CNT;
	//printf("GUID: %s\n", guid_bytes);

//--------------------------authenticate the coins-------------------------------------------------
	fail_cnt = authenticate_coins(no_of_coins, index, coin_id);

	//send response if coins(all or few) counterfiet
	if(fail_cnt == 1) {
		printf("Error: Coins Not Authenticated\n");
		status_code = NFT_ADD_COUNTERFIET_COINS;
		size = RES_HS + HS_BYTES_CNT;
		tcp_send_response(status_code,size);
		return;
	}
	else if(fail_cnt == 2) {
		return;
	}
	printf("--------All Coins Authenticated--------\n");

	//------------------check if guid already exists or not by cross-checking with the nft_table----------------------------------------
	unsigned char flag_guid = 0;
	for(int i = 0; i < NFTS_CNT_MAX; i++) {
		if(memcmp(nft_details[i].GUID, guid_bytes, GUID_BYTES_CNT) == 0) {
			flag_guid = 1;    			//GUID exists
			break;
		}
	}
	if(flag_guid == 0) {
		printf("Error: GUID not exits in the NFT Table\n");
		size = RES_HS + HS_BYTES_CNT;
		status_code = NFT_ADD_GUID_NOT_FOUND;
		tcp_send_response(status_code,size);
		return;
	}

//------------------------store the guid, mfs, sn and an's in the the nft table------------------------------
	index = req_header_min + CH_BYTES_CNT + GUID_BYTES_CNT;
	for(int i = 0;i < no_of_coins;i++) {
		memset(gdObj.data,0,4);
		for(int j = 0;j < SN_BYTES_CNT;j++) {
			gdObj.data[j] = tcp_buffer[index+(SN_BYTES_CNT-1-j)];
		}
		index += SN_BYTES_CNT;
		SN_no = gdObj.val32;
		
		nft_details[SN_no].SN = SN_no;									//add SN to the NFT table
		memcpy(nft_details[SN_no].AN,&tcp_buffer[index],AN_BYTES_CNT);  //add AN to the NFT table
		memcpy(nft_details[SN_no].GUID,guid_bytes,GUID_BYTES_CNT);		//add GUID to the NFT table
		index += AN_BYTES_CNT;
		
		printf("SN: %u\n", nft_details[SN_no].SN);
		printf("AN: ");
		for(int j = 0; j < AN_BYTES_CNT; j++) {
			printf("%d ",nft_details[SN_no].AN[j]);
		}
		printf("\n");
		printf("GUID: ");
		for(int j = 0; j < GUID_BYTES_CNT; j++) {
			printf("%d ",nft_details[SN_no].GUID[j]);
		}
		printf("\n");
	}
	printf("GUID, Coins stored in the NFT Table\n");	

//---------------------------------------------------------------------------------------
	printf("---- Coins Successfully added in the NFT Table----\n");
	size = RES_HS + HS_BYTES_CNT;
	status_code = SUCCESS;
	tcp_send_response(status_code,size);
}
