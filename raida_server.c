#include "raida_server.h"
#include "udp_socket.h"
char execpath[256];
struct server_config server_config_obj;
unsigned char shards_config[SHARDS_MAX];
struct dns_config dns_config_obj[RAIDA_SERVER_MAX];
struct raida_legacy_config raida_legacy_config_obj[RAIDA_SERVER_MAX];
struct coin_id coin_id_obj[255];
struct coin_config *coin_config_obj;
unsigned int coin_id_cnt;
uint8_t legacy_encrypt_key[ENCRYPTION_CONFIG_BYTES];
struct my_id_coins *my_id_coins_obj;

//----------------------------------------------------------
//Welcome message
//----------------------------------------------------------
void welcomeMsg() {
  printf("\nWelcome to Raida Server\n");
}
//----------------------------------------------------------
//load the keys (AN's) for Inter RAIDA Communication
//----------------------------------------------------------
int load_my_id_coins(){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,file_cnt=0,sr_no,k=0;
  unsigned char buff[COIN_KEY_FILE_SIZE];
  char path[256],file_path[1024];
  DIR *d;
  struct dirent *dir;
  printf("------------------------------\n");
  printf("My Id Coin's Details  \n");
  printf("------------------------------\n");
  my_id_coins_obj = (struct my_id_coins *)malloc(sizeof(struct my_id_coins)*server_config_obj.my_id_coins_cnt);
  strcpy(path,execpath);
  strcat(path,"/Data/my_id_coins/");
  d = opendir(path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if(dir->d_name[0]!='.'){
        char file_name[64];
        strcpy(file_name,dir->d_name);
        strtok(file_name, ".");
        sr_no=atoi(file_name);
        strcpy(file_path,path);
        strcat(file_path,dir->d_name);
        fp_inp = fopen(file_path, "rb");
        if(fp_inp!=NULL) {
          if(fread(buff,1,COIN_KEY_FILE_SIZE,fp_inp)!= COIN_KEY_FILE_SIZE){
            printf("Configuration parameters missing in %s \n",file_name);
            fclose(fp_inp);
            closedir(d);	
            return 1;		
          }
          my_id_coins_obj[file_cnt].sr_no = sr_no;
          for(i=3;i<COIN_KEY_FILE_ROWS;i++){
            memcpy(&my_id_coins_obj[file_cnt].AN[i-3],&buff[i*COIN_KEY_AN_SIZE],COIN_KEY_AN_SIZE);
          }	
          file_cnt++;
          fclose(fp_inp);
        }
        if(file_cnt == server_config_obj.my_id_coins_cnt){
          break;
        }

      }      				
    }
    closedir(d);	
  }
  if(file_cnt != server_config_obj.my_id_coins_cnt){
    printf("Files missing for coin keys, needed = %d  available = %d\n",server_config_obj.my_id_coins_cnt,file_cnt);
    return 1;
  }
  printf("%d ID coins loaded successfully..\n",file_cnt);
  /*for(k=0;k<10;k++){
    printf("%d \n",my_id_coins_obj[k].sr_no);
    for(i=0;i<25;i++){
    for(j=0;j<16;j++){
    printf("%d,",my_id_coins_obj[k].AN[i][j]);
    }	
    printf("\n");		
    }	

    }*/
  return 0;	
}
//----------------------------------------------------------
//Create dir's
//----------------------------------------------------------
void create_dirs(){
  char path[256],info[24],mkdir_path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/coin_owners/owners/");
  strcpy(mkdir_path,"mkdir -p ");
  strcat(mkdir_path,path);
  system(mkdir_path);

  strcpy(path,execpath);
  strcat(path,"/Data/coin_owners/statements/");
  strcpy(mkdir_path,"mkdir -p ");
  strcat(mkdir_path,path);
  system(mkdir_path);

  strcpy(path,execpath);
  strcat(path,"/Data/coin_owners/");
  strcpy(mkdir_path,"chmod -R 777 ");
  strcat(mkdir_path,path);
  system(mkdir_path);

}
//----------------------------------------------------------
//Loads raida no from raida_no.txt
//----------------------------------------------------------
int load_raida_no(){
  FILE *fp_inp=NULL;
  int size=0,ch;
  unsigned char buff[24];
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/raida_no.txt");
  if ((fp_inp = fopen(path, "r")) == NULL) {
    printf("raida_no.txt Cannot be opened , exiting \n");
    return 1;
  }
  while( ( ch = fgetc(fp_inp) ) != EOF ){
    size++;
  }
  fclose(fp_inp);
  fp_inp = fopen(path, "r");
  if(fread(buff, 1, size, fp_inp)<size){
    printf("Configuration parameters missing in raida_no.txt \n");
    return 1;
  }
  if(size == 2){
    server_config_obj.raida_id = (buff[0]-48)*10;
    server_config_obj.raida_id+= (buff[1]-48);
  }else{
    server_config_obj.raida_id=buff[0]-48;
  }
  printf("Raida Id  :-%d \n", server_config_obj.raida_id);
  fclose(fp_inp);
  return 0;
}
//----------------------------------------------------------
//Loads server configuation from server.bin
//----------------------------------------------------------
int load_server_config() {
  FILE *fp_inp = NULL;
  int cnt=0;
  unsigned char buff[SERVER_CONFIG_BYTES];
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/server.bin");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("server.bin Cannot be opened , exiting \n");
    return 1;
  }
  if(fread(buff, 1, SERVER_CONFIG_BYTES, fp_inp)<SERVER_CONFIG_BYTES){
    printf("Configuration parameters missing in server.bin \n");
    return 1;
  }
  //server_config_obj.raida_id=buff[0];
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
  server_config_obj.del_free_id_time_intervel = buff[15];
  server_config_obj.del_encryp2_time_intervel = buff[16];
  server_config_obj.deno_type = buff[17];
  server_config_obj.my_id_coins_cnt= buff[19];
  server_config_obj.my_id_coins_cnt |= (((uint16_t)buff[18])<<8);
  printf("------------------------------\n");
  printf("Server Configuration Details..\n");
  printf("------------------------------\n");
  printf("Port Number :- %d \n", server_config_obj.port_number);
  printf("Security check frequency :-%d\n", server_config_obj.security_check_frequency);
  printf("Backup frequency :- %d \n", server_config_obj.backup_frequency);
  printf("Refresh DNS :-%d \n", server_config_obj.refresh_dns);
  printf("Show register coins max :- %d \n", server_config_obj.show_regs_coins_max);
  printf("Show denom coins max :- %d \n", server_config_obj.show_denom_coins_max);
  printf("Show change coins max :- %d \n", server_config_obj.show_change_coins_max);
  printf("Bytes per UDP Request body :- %d \n",server_config_obj.bytes_per_frame);
  printf("Delete Ticket time intervel in secs :- %d \n",server_config_obj.del_ticket_time_intervel);
  printf("Free ID time intervel in days :- %d \n",server_config_obj.del_free_id_time_intervel);
  printf("Encryption 2 time intervel in secs :- %d \n",server_config_obj.del_encryp2_time_intervel);
  printf("Denomination type :- %d \n",server_config_obj.deno_type);
  printf("My Id coins count :- %d \n", server_config_obj.my_id_coins_cnt);
  fclose(fp_inp);
  return 0;
}
//----------------------------------------------------------
//Loads coins configuation from coins_config.bin
//---------------------------------------------------------
int load_coin_config(){
  FILE *fp_inp = NULL;
  unsigned int cnt=0,size=0,i=0,index=0,coin_id_max=0;
  unsigned char buff[SERVER_CONFIG_BYTES];
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/coin_config.bin");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("coin_config.bin Cannot be opened , exiting \n");
    return 1;
  }
  fseek(fp_inp, 0L, SEEK_END);
  size = ftell(fp_inp);
  fseek(fp_inp, 0L, SEEK_SET);
  printf("%u\n", size);
  if(fread(buff, 1, size, fp_inp)<size){
    printf("Configuration parameters missing in coin_config.bin \n");
    return 1;
  }

  coin_config_obj = (struct coin_config *) malloc(sizeof(struct coin_config)*(size/COIN_CONFIG_BYTES));
  coin_id_cnt = size/COIN_CONFIG_BYTES;
  if (coin_id_cnt != COIN_TABLES_CNT){
    printf("Configuration parameters  error missing in coin_config.bin \n");
    return 1;
  }
  index=0;
  printf("------------------------------\n");
  printf("Coin's Configuration Details..\n");
  printf("------------------------------\n");
  printf("Coin id count -: %d\n", coin_id_cnt);
  for(i=0;i<(size/COIN_CONFIG_BYTES);i++){
    if(coin_config_obj[i].coin_id>coin_id_max){
      coin_id_max = coin_config_obj[i].coin_id;
    }
    coin_config_obj[i].coin_id = buff[index+0];
    coin_config_obj[i].page_size = buff[index+2];
    coin_config_obj[i].page_size |= (((uint16_t)buff[index+1])<<8);
    coin_config_obj[i].no_of_pages = buff[index+4];
    coin_config_obj[i].no_of_pages |= (((uint16_t)buff[index+3])<<8);
    printf("Coin Id  -: %d ", coin_config_obj[i].coin_id);
    printf("\t AN's per page  -: %d", coin_config_obj[i].page_size);
    printf("\t No of Pages    -: %d\n", coin_config_obj[i].no_of_pages);
    index += COIN_CONFIG_BYTES;
  }
  //coin_id_obj = (struct coin_id *) malloc(sizeof(struct coin_id)*(coin_id_max+1));
  fclose(fp_inp);
  return 0;
}

//----------------------------------------------------------
//Loads shards configuation from shards.bin
//---------------------------------------------------------
int load_shards_config() {
  FILE *fp_inp = NULL;
  int i = 0;
  unsigned char buff[SHARDS_MAX];
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/shards.bin");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("shards.bin Cannot be opened , exiting \n");
    return 1;
  }
  if(fread(buff, 1, SHARDS_MAX, fp_inp)<SHARDS_MAX){
    printf("Configuration parameters missing in shards.bin \n");
    return 1;
  }
  printf("------------------------------\n");
  printf("Shards Configuration Details..\n");
  printf("------------------------------\n");
  for (i = 0;i < SHARDS_MAX;i++) {
    shards_config[i] = buff[i];
    if (shards_config[i] == 0xff) {
      printf("Shard no :- %d available\n", i+1);
    }else {
      printf("Shard no :- %d not-available\n", i+1);
    }
  }
  fclose(fp_inp);
  return 0;
}
//----------------------------------------------------------
//Loads dns configuation from dns.bin
//---------------------------------------------------------
int load_dns_config() {
  FILE *fp_inp = NULL;
  int i = 0,j=0,index=0;
  unsigned int dns_port;
  char dns_ip[64],tmp[16];
  unsigned char buff[RAIDA_SERVER_MAX*(DNS_LEN_MAX+DNS_PORT_MAX)];
  union coversion{
    unsigned char data[4];
    uint32_t val32;
  }convObj;
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/dns.bin");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("dns.bin Cannot be opened , exiting \n");
    return 1;
  }
  if(fread(buff, 1, RAIDA_SERVER_MAX*(DNS_LEN_MAX+DNS_PORT_MAX), fp_inp)<(RAIDA_SERVER_MAX*(DNS_LEN_MAX+DNS_PORT_MAX))){
    printf("Configuration parameters missing in dns.bin \n");
    return 1;
  }
  printf("------------------------------\n");
  printf("DNS Configuration Details..\n");
  printf("------------------------------\n");
  index =0;	
  for (i=0;i<RAIDA_SERVER_MAX;i++) {
    strcpy(dns_ip,"");
    for (j=0; j<DNS_LEN_MAX; j++) {
      sprintf(tmp, "%d", buff[index+j]);
      strcat(dns_ip,tmp);
      if(j<DNS_LEN_MAX-1)
        strcat(dns_ip,"."); 
    }
    index=index+DNS_LEN_MAX;
    convObj.val32 = 0;
    for (j=0; j<DNS_PORT_MAX; j++) {
      convObj.data[DNS_PORT_MAX-1-j] = buff[index+j];	
    }
    index=index+DNS_PORT_MAX;
    dns_port = convObj.val32;
    if(server_config_obj.raida_id!=i){
      init_dns_socket(i,dns_port,dns_ip);
    }
    printf("Raida :- %d \tdns :-%s   \t Port Number :-% d\n",i+1, dns_ip,dns_port);
  }
  fclose(fp_inp);
  return 0;
} 
//----------------------------------------------------------
//Loads legacy configuation from raida_legacy.bin
//---------------------------------------------------------
int load_raida_legacy_config() {
  FILE *fp_inp = NULL;
  int i = 0,j=0,index=0;
  unsigned int legacy_port;
  unsigned char legacy_ip[64],tmp[16];
  unsigned char buff[RAIDA_SERVER_MAX*(RAIDA_LEGACY_IP_MAX+RAIDA_LEGACY_PORT_MAX)];
  union coversion{
    unsigned char data[4];
    uint32_t val32;
  }convObj;
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/raida_legacy.bin");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("raida_legacy.bin Cannot be opened , exiting \n");
    return 1;
  }
  if(fread(buff, 1, RAIDA_SERVER_MAX*(RAIDA_LEGACY_IP_MAX+ RAIDA_LEGACY_PORT_MAX), fp_inp)<(RAIDA_SERVER_MAX*(RAIDA_LEGACY_IP_MAX+ RAIDA_LEGACY_PORT_MAX))){
    printf("Configuration parameters missing in raida_legacy.bin \n");
    return 1;
  }
  printf("------------------------------\n");
  printf("RAIDA LEGACYs Configuration Details..\n");
  printf("------------------------------\n");
  index =0;	
  for (i=0;i<RAIDA_SERVER_MAX;i++) {
    strcpy(legacy_ip,"");
    for (j=0; j<RAIDA_LEGACY_IP_MAX; j++) {
      sprintf(tmp, "%d", buff[index+j]);
      strcat(legacy_ip,tmp);
      if(j<RAIDA_LEGACY_IP_MAX-1)
        strcat(legacy_ip,"."); 
    }
    index=index+RAIDA_LEGACY_IP_MAX;
    convObj.val32 = 0;
    for (j=0; j<RAIDA_LEGACY_PORT_MAX; j++) {
      convObj.data[RAIDA_LEGACY_PORT_MAX-1-j] = buff[index+j];	
    }
    index=index+RAIDA_LEGACY_PORT_MAX;
    legacy_port = convObj.val32;
    //if(server_config_obj.raida_id!=i){
    init_legacy_socket(i,legacy_port,legacy_ip);
    //}
    printf("Raida :- %d \t legacy ip :-%s         \t Port Number :-% d\n",i+1, legacy_ip,legacy_port);
  }
  fclose(fp_inp);
  return 0;
}
//----------------------------------------------------------
//Loads encrypt key from raida_legacy.bin
//--------------------------------------------------------- 
int load_encrypt_key(){
  FILE *fp_inp = NULL;
  int i = 0;
  unsigned char buff[ENCRYPTION_CONFIG_BYTES];
  char path[256];
  strcpy(path,execpath);
  strcat(path,"/Data/encryption_key.bin");
  printf("------------------------------\n");
  printf("ENCRYPTION CONFIG KEY ..\n");
  printf("------------------------------\n");
  if ((fp_inp = fopen(path, "rb")) == NULL) {
    printf("encryption_key.bin Cannot be opened , exiting \n");
    return 1;
  }
  if(fread(buff, 1, ENCRYPTION_CONFIG_BYTES, fp_inp)<(ENCRYPTION_CONFIG_BYTES)){
    printf("Configuration parameters missing in encryption_key.bin \n");
    return 1;
  }
  memcpy(legacy_encrypt_key,buff,ENCRYPTION_CONFIG_BYTES);
  for(i=0;i<ENCRYPTION_CONFIG_BYTES;i++){
    printf("%02x,",legacy_encrypt_key[i]);
  }
  printf("\n");
  fclose(fp_inp);
  return 0;
}
//----------------------------------------------------------
//configure Authentication numbers 
//---------------------------------------------------------
int configure_an(unsigned int index, int alloc_only){

  FILE *fp_inp = NULL;
  unsigned int i = 0,j=0,an_cnt,no_of_bytes,size;
  unsigned char buff[AN_BYTES_CNT+MS_BYTES_CNT];
  char str_page_no[16],str_coin_id[16];
  char path[256], mkdir_path[256];
  sprintf((char*)str_coin_id,"%d",coin_config_obj[index].coin_id);
  coin_config_obj[index].pages_changed = (unsigned char *) malloc(sizeof(unsigned char) * coin_config_obj[index].no_of_pages);
  memset(coin_config_obj[index].pages_changed,sizeof(unsigned char) * coin_config_obj[index].no_of_pages,0);
  // We don't need to generate anything 
  
  if (alloc_only) {
    return 0;
  }
  for(i=0;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Creating AN's for coin  %d Please wait... \n",index);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcpy(mkdir_path,"mkdir -m 777 >>/dev/null 2>>/dev/null ");
    strcat(mkdir_path,path);
    system(mkdir_path);
    strcat(path,"/ANs");
    strcpy(mkdir_path,"mkdir -m 777 >>/dev/null 2>>/dev/null ");
    strcat(mkdir_path,path);
    system(mkdir_path);
    strcat(path,"/");
    sprintf((char*)str_page_no,"%d",i);		
    strcat(path,str_page_no);
    strcat(path,".bin");
    puts(path);
    if ((fp_inp = fopen(path, "ab+")) == NULL) {
      fp_inp = fopen(path, "wb+");
    }
    fseek(fp_inp, 0L, SEEK_END);
    an_cnt = ftell(fp_inp)/(AN_BYTES_CNT+MFS_BYTES_CNT);
    printf("AN's = %d",an_cnt);
    printf("coin = %d",coin_config_obj[index].page_size);
    if(an_cnt<coin_config_obj[index].page_size){
      unsigned char upper = 255, lower = 1, num[1];
      no_of_bytes = an_cnt*(AN_BYTES_CNT+MFS_BYTES_CNT);
      fseek(fp_inp,no_of_bytes,SEEK_SET);	
      for(j = 0;j < (coin_config_obj[index].page_size*(AN_BYTES_CNT+MFS_BYTES_CNT))-no_of_bytes; j++){
        //srand(j);
        num[0] = 0;// (rand() % (upper - lower + 1)) + lower;
        fwrite(num,1,1,fp_inp);
      }
      printf(".");
    }
    fclose(fp_inp);
  }
  return 0;
}
//----------------------------------------------------------
//Loads Authentication numbers from AN.bin
//---------------------------------------------------------
int load_an(unsigned int index,unsigned int coin_id){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,k=0,an_index=0, an_cnt;
  unsigned char buff[AN_BYTES_CNT+MS_BYTES_CNT];
  char str_page_no[16],str_coin_id[16],path[256];
  printf("\n------------------------------\n");
  printf("%d AN  Details.. \n", coin_id);
  printf("------------------------------\n");
  an_cnt=coin_config_obj[index].no_of_pages * coin_config_obj[index].page_size;
  coin_id_obj[coin_id].AN_CNT =  an_cnt;
  coin_id_obj[coin_id].AN = (unsigned char **) malloc(an_cnt*sizeof(unsigned char *));
  coin_id_obj[coin_id].MFS = (unsigned char *) malloc(an_cnt*MFS_BYTES_CNT);
  coin_id_obj[coin_id].free_id_days = (unsigned char *) malloc(an_cnt * FREE_ID_BYTES_CNT);
  coin_id_obj[coin_id].TICKETS =(struct master_ticket **) malloc(an_cnt * sizeof(struct master_ticket *));
  sprintf((char*)str_coin_id,"%d",coin_id);
  
  for(i=0;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Loading AN's for coin %d Please wait... \n",coin_id);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcat(path,"/ANs/");
    //strcat(path,str_coin_id);
    //strcat(path,"_");
    sprintf((char*)str_page_no,"%d",i);		
    strcat(path,str_page_no);
    strcat(path,".bin");
    //puts(path);
    if ((fp_inp = fopen(path, "rb")) == NULL) {
      printf("%s",path);
      printf("Cannot be opened , exiting \n");
      return 1;
    }
    for(j=0;j<coin_config_obj[index].page_size;j++){
      fread(buff, AN_BYTES_CNT+MFS_BYTES_CNT, 1, fp_inp) ;
      coin_id_obj[coin_id].AN[an_index]= (unsigned char *) malloc(AN_BYTES_CNT);
      memcpy(coin_id_obj[coin_id].AN[an_index],buff,AN_BYTES_CNT);
      memcpy(nft_details[an_index].AN,buff,AN_BYTES_CNT);
      coin_id_obj[coin_id].MFS[an_index]=buff[AN_BYTES_CNT];
      /*printf("AN  :- %d \t [",an_index );
        for(k=0;k<AN_BYTES_CNT;k++){		
        printf("%3d, ", coin_id_obj[coin_id].AN[an_index][k]);
        }
        printf("]");
        printf("\t MFS : -[%d]\n",coin_id_obj[index].MFS[an_index]);
        printf("%d,",an_index);*/
      coin_id_obj[coin_id].TICKETS[an_index]=NULL;
      coin_id_obj[coin_id].free_id_days[an_index] = 0;		
      an_index++;
    }
    printf(".");
    fclose(fp_inp);
  }
  printf("\n %d AN's loaded successfully \n",an_index);
  return 0;
}

//----------------------------------------------------------
//configure GUIDs 
//---------------------------------------------------------
int configure_guid(unsigned int index, int alloc_only){

  FILE *fp_inp = NULL;
  unsigned int i = 0,j=0,an_cnt,no_of_bytes,size;
  unsigned char buff[AN_BYTES_CNT+MS_BYTES_CNT];
  char str_page_no[16],str_coin_id[16];
  char path[256], mkdir_path[256];
  sprintf((char*)str_coin_id,"%d",coin_config_obj[index].coin_id);
  coin_config_obj[index].pages_changed = (unsigned char *) malloc(sizeof(unsigned char) * coin_config_obj[index].no_of_pages);
  memset(coin_config_obj[index].pages_changed,sizeof(unsigned char) * coin_config_obj[index].no_of_pages,0);
  // We don't need to generate anything 
  
  if (alloc_only) {
    return 0;
  }
  for(i=0;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Creating GUID's for coin %d Please wait... \n",coin_config_obj[index].coin_id);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcpy(mkdir_path,"mkdir -m 777 >>/dev/null 2>>/dev/null ");
    strcat(mkdir_path,path);
    system(mkdir_path);
    strcat(path,"/GUIDs");
    strcpy(mkdir_path,"mkdir -m 777 >>/dev/null 2>>/dev/null ");
    strcat(mkdir_path,path);
    system(mkdir_path);
    strcat(path,"/");
    sprintf((char*)str_page_no,"%d",i);		
    strcat(path,str_page_no);
    strcat(path,".bin");
    puts(path);
    if ((fp_inp = fopen(path, "ab+")) == NULL) {
      fp_inp = fopen(path, "wb+");
    }
    fseek(fp_inp, 0L, SEEK_END);
    an_cnt = ftell(fp_inp)/(GUID_BYTES_CNT+MFS_BYTES_CNT);
    printf("guid's = %d",an_cnt);
    printf("coin = %d",coin_config_obj[index].page_size);
    if(an_cnt < coin_config_obj[index].page_size){
      unsigned char upper = 255, lower = 1, num[1];
      no_of_bytes = an_cnt*(GUID_BYTES_CNT+MFS_BYTES_CNT);
      fseek(fp_inp,no_of_bytes,SEEK_SET);	
      for(j = 0;j < (coin_config_obj[index].page_size*(AN_BYTES_CNT+MFS_BYTES_CNT))-no_of_bytes; j++){
        //srand(j);
        num[0] = 0;// (rand() % (upper - lower + 1)) + lower;
        fwrite(num,1,1,fp_inp);
      }
      printf(".");
    }
    fclose(fp_inp);
  }
  return 0;
}
//----------------------------------------------------------
//Loads GUIDs from GUIDs folder
//---------------------------------------------------------
int load_guid(unsigned int index,unsigned int coin_id){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,k=0,an_index=0, an_cnt;
  unsigned char buff[GUID_BYTES_CNT+MS_BYTES_CNT];
  char str_page_no[16],str_coin_id[16],path[256];
  printf("\n------------------------------\n");
  printf("%d GUID  Details.. \n", coin_id);
  printf("------------------------------\n");
  an_cnt=coin_config_obj[index].no_of_pages * coin_config_obj[index].page_size;
  //coin_id_obj[coin_id].AN_CNT =  an_cnt;
  //coin_id_obj[coin_id].AN = (unsigned char **) malloc(an_cnt*sizeof(unsigned char *));
  //coin_id_obj[coin_id].MFS = (unsigned char *) malloc(an_cnt*MFS_BYTES_CNT);
  //coin_id_obj[coin_id].free_id_days = (unsigned char *) malloc(an_cnt * FREE_ID_BYTES_CNT);
  //coin_id_obj[coin_id].TICKETS =(struct master_ticket **) malloc(an_cnt * sizeof(struct master_ticket *));

  coin_id_obj[coin_id].GUID = (unsigned char **) malloc(an_cnt*sizeof(unsigned char *));
  sprintf((char*)str_coin_id,"%d",coin_id);
  
  for(i=0;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Loading AN's for coin %d Please wait... \n",coin_id);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcat(path,"/GUIDs/");
    //strcat(path,str_coin_id);
    //strcat(path,"_");
    sprintf((char*)str_page_no,"%d",i);		
    strcat(path,str_page_no);
    strcat(path,".bin");
    //puts(path);
    if ((fp_inp = fopen(path, "rb")) == NULL) {
      printf("%s",path);
      printf("Cannot be opened , exiting \n");
      return 1;
    }
    for(j=0;j<coin_config_obj[index].page_size;j++){
      fread(buff, AN_BYTES_CNT+MFS_BYTES_CNT, 1, fp_inp) ;
      nft_details[an_index].SN = an_index;
      coin_id_obj[coin_id].GUID[an_index] = (unsigned char *) malloc(GUID_BYTES_CNT);
      memcpy(coin_id_obj[coin_id].GUID[an_index],buff,GUID_BYTES_CNT);
      memcpy(nft_details[an_index].GUID,buff,GUID_BYTES_CNT);
      nft_details[an_index].MFS = buff[GUID_BYTES_CNT];

      /*printf("GUID  :- %d \t [",an_index );
        for(k=0;k<GUID_BYTES_CNT;k++){		
        printf("%3d, ", coin_id_obj[coin_id].GUID[an_index][k]);
        }
        printf("]");
        printf("\t MFS : -[%d]\n",coin_id_obj[index].MFS[an_index]);
        printf("%d,",an_index);*/
      //coin_id_obj[coin_id].TICKETS[an_index]=NULL;
      //coin_id_obj[coin_id].free_id_days[an_index] = 0;		
      an_index++;
    }
    printf(".");
    fclose(fp_inp);
  }
  printf("\n %d AN's loaded successfully \n",an_index);
  return 0;
}

//---------------------------------------------------------
// Adds ticket at the serial_no index
//--------------------------------------------------------
void add_ticket(unsigned int coin_id,uint32_t ticket_no,uint32_t sr_no){
  if(coin_id_obj[coin_id].TICKETS[sr_no]==NULL){
    coin_id_obj[coin_id].TICKETS[sr_no] = (struct master_ticket  *) malloc(sizeof(struct master_ticket));
  }
  coin_id_obj[coin_id].TICKETS[sr_no]->ticket_no = ticket_no; 
  coin_id_obj[coin_id].TICKETS[sr_no]->time_stamp = time(NULL);
  coin_id_obj[coin_id].TICKETS[sr_no]->raida_claim =0xffffffff;
  printf("Ticket no %d added successfully at serial no %d with time stamp %d \n",coin_id_obj[coin_id].TICKETS[sr_no]->ticket_no, sr_no, coin_id_obj[coin_id].TICKETS[sr_no]->time_stamp);
}
//----------------------------------------------------------
// Returns time in centi seconds
//----------------------------------------------------------
long get_time_cs()
{
  long            ms,cs; // Milliseconds
  time_t          s;  // Seconds
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  s  = spec.tv_sec;
  ms = round(spec.tv_nsec / 1.0e3); // Convert nanoseconds to milliseconds
  cs = ms /100;	
  //    printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",(intmax_t)s, ms);
  return ms;	
}
//---------------------------------------------------------
// Get the current directory path starting from home dir
//---------------------------------------------------------
void getexepath()
{
  char buff[256];
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
//----------------------------------------------------------
//Compares 2 dates
//----------------------------------------------------------
char compare_date(struct date d1,struct date d2){
  if (d2.year < d1.year)
    return -1;
  else if (d2.year > d1.year)
    return 1;
  if (d1.year == d2.year)
  {
    if (d2.month<d1.month)
      return -1;
    else if (d2.month>d1.month)
      return 1;
    if (d1.month == d2.month){
      if (d2.day<d1.day)
        return -1;
      else if(d2.day>d1.day)
        return 1;
      if(d1.day==d2.day){
        if (d2.hh<d1.hh)
          return -1;
        else if(d2.hh>d1.hh)
          return 1;	
        if(d1.hh==d2.hh){
          if (d2.mm<d1.mm)
            return -1;
          else if(d2.mm>d1.mm)
            return 1;	
          if(d1.mm==d2.mm){
            if (d2.ss<d1.ss)
              return -1;
            else if(d2.ss>d1.ss)
              return 1;	
            else if(d1.ss==d2.ss)
              return 1;
          }

        }

      }

    }

  }

}
//-------------------------------------------------------------
// update AN pages
//-------------------------------------------------------------
void update_an_pages(unsigned int coin_id){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,start_index=0,end_index=0;
  char str_page_no[16],str_coin_id[16],tmp[256];
  char path[256];
  sprintf((char*)str_coin_id,"%d",coin_id);
  for(i=0;i<coin_config_obj[coin_id].no_of_pages;i++){
    if(coin_config_obj[coin_id].pages_changed[i]==1){
      printf("%d page updated",i+1);
      /*strcpy(path,execpath);
        strcat(path,"/Data/coin_");
        strcat(path,str_coin_id);
        strcat(path,"/coin_");
        strcat(path,str_coin_id);
        strcat(path,"_");
        sprintf((char*)str_page_no,"%d",i);		
        strcat(path,str_page_no);
        strcpy(tmp,path);
        strcat(path,"_an.bin");
        strcat(tmp,"_an_tmp.bin");*/

      strcpy(path,execpath);
      strcat(path,"/Data/coin_");
      strcat(path,str_coin_id);
      strcat(path,"/ANs/");
      sprintf((char*)str_page_no,"%d",i);		
      strcat(path,str_page_no);
      strcat(path,".bin");
      strcpy(tmp,path);
      remove(path);
      fp_inp = fopen(tmp, "wb+");
      start_index = i * coin_config_obj[coin_id].page_size;
      end_index = start_index + coin_config_obj[coin_id].page_size;
      //printf("%d - %d", start_index, end_index);
      fseek(fp_inp,0,SEEK_SET);	
      for(j=start_index;j<end_index;j++){
        fwrite(coin_id_obj[coin_id].AN[j],AN_BYTES_CNT,1,fp_inp);
        fwrite(&coin_id_obj[coin_id].MFS[j],MFS_BYTES_CNT,1,fp_inp);
      }
      coin_config_obj[coin_id].pages_changed[i]=0;
      fclose(fp_inp);
      rename(tmp,path);
    }
  }		
}

//-------------------------------------------------------------
// update GUID pages
//-------------------------------------------------------------
void update_guid_pages(unsigned int coin_id){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,start_index=0,end_index=0;
  char str_page_no[16],str_coin_id[16],tmp[256];
  char path[256];
  sprintf((char*)str_coin_id,"%d",coin_id);
  for(i=0;i<coin_config_obj[coin_id].no_of_pages;i++){
    if(coin_config_obj[coin_id].pages_changed[i]==1){
      printf("%d page updated",i+1);
      /*strcpy(path,execpath);
        strcat(path,"/Data/coin_");
        strcat(path,str_coin_id);
        strcat(path,"/coin_");
        strcat(path,str_coin_id);
        strcat(path,"_");
        sprintf((char*)str_page_no,"%d",i);		
        strcat(path,str_page_no);
        strcpy(tmp,path);
        strcat(path,"_an.bin");
        strcat(tmp,"_an_tmp.bin");*/

      strcpy(path,execpath);
      strcat(path,"/Data/coin_");
      strcat(path,str_coin_id);
      strcat(path,"/GUIDs/");
      sprintf((char*)str_page_no,"%d",i);		
      strcat(path,str_page_no);
      strcat(path,".bin");
      strcpy(tmp,path);
      remove(path);
      fp_inp = fopen(tmp, "wb+");
      start_index = i * coin_config_obj[coin_id].page_size;
      end_index = start_index + coin_config_obj[coin_id].page_size;
      //printf("%d - %d", start_index, end_index);
      fseek(fp_inp,0,SEEK_SET);	
      for(j=start_index;j<end_index;j++){
        fwrite(nft_details[j].GUID,GUID_BYTES_CNT,1,fp_inp);
        fwrite(&nft_details[j].MFS,MFS_BYTES_CNT,1,fp_inp);
      }
      coin_config_obj[coin_id].pages_changed[i]=0;
      fclose(fp_inp);
      rename(tmp,path);
    }
  }		
}

//-------------------------------------------------------------
// load coin owner and coin details
//-------------------------------------------------------------
int load_coin_owner(){
  char path[256],tmp_path[256];
  unsigned char buff[64],j,index=0,str_owner_id[24];
  unsigned int owner_id;
  DIR *d;
  struct dirent *dir;
  FILE *fp_inp = NULL;
  struct coin_owner			coin_owner_tmp_obj;	
  struct coin_owner_details		coin_owner_details_tmp_obj;	
  strcpy(path,execpath);
  strcat(path,"/Data/coin_owners/owners/");
  d = opendir(path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if(dir->d_name[0]!='.'){
        char file_name[64];
        strcpy(file_name,dir->d_name);
        strtok(file_name, ".");
        owner_id=atoi(file_name);
        strcpy(tmp_path,path);
        strcat(tmp_path,dir->d_name);
        fp_inp = fopen(tmp_path, "rb");
        if(fp_inp!=NULL) {
          while(fread(buff, 1,SN_BYTES_CNT+COIN_TYPE_BYTES_CNT, fp_inp) > 0){
            memset(snObj.data,0,4);
            for(j=0;j<SN_BYTES_CNT;j++)
              snObj.data[j]=buff[(SN_BYTES_CNT-1-j)];
            printf("%d",snObj.val32);
            add_coin_owner(snObj.val32,owner_id,buff[3],0);
          }
          fclose(fp_inp);
        }
      }      				
    }
    closedir(d);	
  }
  //Load coin statements
  strcpy(path,execpath);
  strcat(path,"/Data/coin_owners/statements/");
  d = opendir(path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if(dir->d_name[0]!='.'){
        char file_name[64];
        strcpy(file_name,dir->d_name);
        strtok(file_name, ".");
        owner_id=atoi(file_name);
        strcpy(tmp_path,path);
        strcat(tmp_path,dir->d_name);
        fp_inp = fopen(tmp_path, "rb");
        if(fp_inp!=NULL){
          while(fread(buff,1,AMT_BYTES_CNT+AMT_BYTES_CNT+1+TIME_STAMP_BYTES_CNT+GUID_BYTES_CNT+META_DATA_BYTES_CNT,fp_inp) !=0){					
            index=0;
            for(j=0;j<AMT_BYTES_CNT;j++){
              snObj.data[j]=buff[index+(AMT_BYTES_CNT-1-j)];
            }
            coin_owner_details_tmp_obj.balance = snObj.val32 ;
            printf("Bal %d\n",coin_owner_details_tmp_obj.balance);
            index+=AMT_BYTES_CNT;
            for(j=0;j<AMT_BYTES_CNT;j++){
              snObj.data[j]=buff[index+(AMT_BYTES_CNT-1-j)];
            }
            coin_owner_details_tmp_obj.amt = snObj.val32 ;
            printf("Amt %d\n",coin_owner_details_tmp_obj.amt);
            index+=AMT_BYTES_CNT;
            coin_owner_details_tmp_obj.trans_type = buff[index];
            printf("Transl %d\n",coin_owner_details_tmp_obj.trans_type);
            index++;
            for(j=0;j<TIME_STAMP_BYTES_CNT;j++){
              coin_owner_details_tmp_obj.time_stamp[j]=buff[index];
            }

            index+=TIME_STAMP_BYTES_CNT;	
            for(j=0;j<GUID_BYTES_CNT;j++){
              coin_owner_details_tmp_obj.guid[j]=buff[j+index];
            }
            index+=GUID_BYTES_CNT;
            for(j=0;j<META_DATA_BYTES_CNT;j++){
              coin_owner_details_tmp_obj.meta_data[j]= buff[j+index];
            }
            add_coin_owner_details(owner_id,coin_owner_details_tmp_obj.guid,coin_owner_details_tmp_obj.time_stamp
                ,coin_owner_details_tmp_obj.trans_type,coin_owner_details_tmp_obj.meta_data
                ,coin_owner_details_tmp_obj.amt,coin_owner_details_tmp_obj.balance,0);
          }
          fclose(fp_inp);
        }
      }      				
    }
    closedir(d);
  }
  return 0;
}
//-------------------------------------------------------------
// update coin owner 
//-------------------------------------------------------------
void update_coin_owner(){
  unsigned int i=0,j=0,k=0;
  char str_coin_own[16],tmp[256],path[256],info[24];
  FILE *fp_inp = NULL;
  for(i=0;i<COIN_OWNER_MAX;i++){
    if(coin_owner_obj[i].changed == 1){
      sprintf((char*)str_coin_own,"%d",coin_owner_obj[i].owner_id);		
      strcpy(path,execpath);
      strcat(path,"/Data/coin_owners/owners/");
      strcat(path,str_coin_own);
      strcpy(tmp,path);
      strcat(path,".bin");
      strcat(tmp,"_");
      strcat(tmp,".bin");
      rename(path,tmp);
      fp_inp = fopen(path, "wb+");
      if(fp_inp!=NULL){
        for(j=0;j<COIN_OWNER_MAX;j++){
          if(coin_owner_obj[i].owner_id == coin_owner_obj[j].owner_id ){
            coin_owner_obj[j].changed = 0;	
            snObj.val32 = coin_owner_obj[j].serial_no;
            for(k=0;k<SN_BYTES_CNT;k++){
              fwrite(&snObj.data[(SN_BYTES_CNT-1-k)],1,1,fp_inp);
            }	
            fwrite(&coin_owner_obj[j].coin_type,1,1,fp_inp);
          }
        }
        fclose(fp_inp);
      }
      remove(tmp);
    }
  }		
}
//-------------------------------------------------------------
// update coin owner 
//-------------------------------------------------------------
void* update_coin_owner_details(void *arg){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,k=0;
  char str_coin_own[16],tmp[256],path[256],info[24],mkdir_path[256];
  for(i=0;i<COIN_OWNER_MAX;i++){
    if(coin_owner_details_obj[i].changed == 1){
      coin_owner_details_obj[i].changed = 0;	
      sprintf((char*)str_coin_own,"%d",coin_owner_details_obj[i].owner_id);		
      strcpy(path,execpath);
      strcat(path,"/Data/coin_owners/statements/");
      strcat(path,str_coin_own);
      strcat(path,".bin");
      fp_inp = fopen(path, "ab+");
      snObj.val32 = coin_owner_details_obj[i].amt;
      for(k=0;k<AMT_BYTES_CNT;k++){
        fwrite(&snObj.data[(AMT_BYTES_CNT-1-k)],1,1,fp_inp);
      }
      snObj.val32 = coin_owner_details_obj[i].balance;
      for(k=0;k<AMT_BYTES_CNT;k++){
        fwrite(&snObj.data[(AMT_BYTES_CNT-1-k)],1,1,fp_inp);
      }	
      fwrite(&coin_owner_details_obj[i].trans_type,1,1,fp_inp);
      for(k=0;k<TIME_STAMP_BYTES_CNT;k++){
        fwrite(&coin_owner_details_obj[i].time_stamp[k],1,1,fp_inp);
      }	
      for(k=0;k<GUID_BYTES_CNT;k++){
        fwrite(&coin_owner_details_obj[i].guid[k],1,1,fp_inp);
      }
      for(k=0;k<META_DATA_BYTES_CNT;k++){
        fwrite(&coin_owner_details_obj[i].meta_data[k],1,1,fp_inp);
      }
      fclose(fp_inp);		
    }
  }		
}

//-------------------------------------------------------------
// backup an's
//-------------------------------------------------------------
void* backup_an_thread(void *arg){
  uint32_t time1,time2,i=0;
  time1=time(NULL);
  while(1){
    time2 = time(NULL);
    if((time2 - time1) > server_config_obj.backup_frequency){
      time1 = time2;
      //printf("An Back up time \n");
      /*
      for(i=0;i<coin_id_cnt;i++){
        update_an_pages(i);
      } */
      update_an_pages(2);    //coin_id for NFT = 2
    }
  }
}

//-------------------------------------------------------------
// backup guid's
//-------------------------------------------------------------
void* backup_guid_thread(void *arg){
  uint32_t time1,time2,i=0;
  time1=time(NULL);
  while(1){
    time2 = time(NULL);
    if((time2 - time1) > server_config_obj.backup_frequency){
      time1 = time2;
      //printf("An Back up time \n");
      /*
      for(i=0;i<coin_id_cnt;i++){
        update_an_pages(i);
      } */
      update_guid_pages(2);    //coin_id for NFT = 2
    }
  }
}
//-------------------------------------------------------------
// backup coin owner
//-------------------------------------------------------------
void* backup_coin_owner_thread(void *arg){
  uint32_t time1,time2,i=0;
  time1=time(NULL);
  while(1){
    time2 = time(NULL);
    if((time2 - time1) > server_config_obj.backup_frequency){
      time1 = time2;
      //printf("coin owner back up time \n");
      update_coin_owner();
      update_coin_owner_details(NULL);
    }
  }
}
//-------------------------------------------------------------
// free id days_left thread
//-------------------------------------------------------------
void* free_id_days_left_thread(void *arg){
  uint32_t time1,time2,i=0,j=0,coin_id=0;
  time1=time(NULL);
  while(1){
    time2 = time(NULL);
    if((time2 - time1) > SECS_IN_DAY){
      time1 = time2;
      for(i=0;i<coin_id_obj[coin_id].AN_CNT;i++){
        for(j=0;j<COIN_OWNER_MAX;j++){
          if(coin_owner_obj[j].owner_id != 0){
            if(coin_owner_obj[j].serial_no == i){
              break;
            }
          }
        }
        if(j>=COIN_OWNER_MAX){
          if(coin_id_obj[coin_id].free_id_days[i]==0){
            memset(coin_id_obj[coin_id].AN[i],0,AN_BYTES_CNT);
          }else{
            coin_id_obj[coin_id].free_id_days[i]--;
          }
        }
      }
    }				
  }
}
//-------------------------------------------------------------
// del encry2 keys thread
//-------------------------------------------------------------
void* del_encryp2_keys_thread(void *arg){
  uint32_t time1,time2,i=0;
  time1=time(NULL);
  while(1){
    time2 = time(NULL);
    if((time2 - time1) > server_config_obj.del_encryp2_time_intervel){
      unsigned int i=0;	
      for(i=0;i<ENCRY2_KEYS_MAX;i++){
        if(key_table_obj[i].key_id !=0){
          if((time2-key_table_obj[i].time_stamp) > server_config_obj.del_encryp2_time_intervel){
            key_table_obj[i].key_id = 0;
            key_table_obj[i].time_stamp = 0;
            break;
          }
        }
      }
    }
  }
}
//-------------------------------------------------------------
//
//-------------------------------------------------------------

int rename_an_files(unsigned int index,unsigned int coin_id){
  FILE *fp_inp = NULL;
  unsigned int i=0,j=0,k=0,an_index=0, an_cnt;
  char str_page_no[16],str_page_no_tmp[16],str_coin_id[16],path[256],tmp[256];
  printf("\n------------------------------\n");
  printf("%d AN  Details.. \n", coin_id);
  printf("------------------------------\n");
  //	coin_config_obj[index].no_of_pages = 4;
  sprintf((char*)str_coin_id,"%d",coin_id);
  for(i=0;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Renaming coin  %d Please wait... \n",i);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcat(path,"/ANs/");
    sprintf((char*)str_page_no,"%d",i);		
    strcat(path,str_page_no);
    strcpy(tmp,path);
    strcat(path,".bin");
    strcat(tmp,"_.bin");
    puts(path);
    puts(tmp);
    rename(path,tmp);
    printf(".");
  }
  for(i=1;i<coin_config_obj[index].no_of_pages;i++){
    printf("\n Renaming coin  %d Please wait... \n",i);
    strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcat(path,"/ANs/");
    strcpy(tmp,path);
    sprintf((char*)str_page_no,"%d",i);		
    sprintf((char*)str_page_no_tmp,"%d",i-1);		
    strcat(path,str_page_no);
    strcat(tmp,str_page_no_tmp);
    strcat(path,"_.bin");
    strcat(tmp,".bin");
    rename(path,tmp);
  }
  strcpy(path,execpath);
  strcat(path,"/Data/coin_");
  strcat(path,str_coin_id);
  strcat(path,"/ANs/");
  strcpy(tmp,path);
  sprintf((char*)str_page_no,"%d",0);		
  sprintf((char*)str_page_no_tmp,"%d",coin_config_obj[index].no_of_pages-1);		
  strcat(path,str_page_no);
  strcat(path,"_.bin");
  strcat(tmp,str_page_no_tmp);
  strcat(tmp,".bin");
  rename(path,tmp);	

  /*strcpy(path,execpath);
    strcat(path,"/Data/coin_");
    strcat(path,str_coin_id);
    strcat(path,"/ANs/");
    strcpy(tmp,path);
    sprintf((char*)str_page_no,"%d",0);		
    sprintf((char*)str_page_no_tmp,"%d",0);		
    strcat(path,str_page_no);
    strcat(path,"_.bin");
    strcat(tmp,".bin");
    rename(tmp,path);	*/

  printf("\n");
  return 0;
}
//------------------------------------------------------------
//MAIN PROGRAM TO RUN RAIDA SERVER
//------------------------------------------------------------

void* main_NFT_server(void *arg) {

  printf("NFT Server THREAD\n");
  //srand(time(NULL));
  init_tcp_socket();
  pthread_t ptid;
  pthread_create(&ptid, NULL, &backup_guid_thread, NULL);
 
  while(1) {
    tcp_listen_request();
  }
  //rename_an_files(1,1);

}

//------------------------------------------------------------
//MAIN PROGRAM TO RUN NFT SERVER
//------------------------------------------------------------
void* main_raida_server(void *arg) {

  printf("RAIDA_server thread\n");
  //srand(time(NULL));
  init_udp_socket();
  
  pthread_t ptid[4];
  pthread_create(&ptid[0], NULL, &backup_an_thread, NULL);
  pthread_create(&ptid[1], NULL, &backup_coin_owner_thread, NULL);
 // pthread_create(&ptid[2], NULL, &free_id_days_left_thread, NULL);
  pthread_create(&ptid[3], NULL, &del_encryp2_keys_thread, NULL);
  
  while(1) {
    listen_request();
  } 
  //rename_an_files(1,1);

}

//----------------------------------------------------------
// main function
//---------------------------------------------------------
int main(int argc, char *argv[]) {
  uint32_t packet_size;	
  int alloc_only = 1;
  welcomeMsg();
  getexepath();
  create_dirs();

  if(load_raida_no() || load_server_config()  || load_coin_config() || load_shards_config()  || load_dns_config() || load_raida_legacy_config() ||
      load_encrypt_key() || load_coin_owner() || load_my_id_coins() || Read_NFT_Configuration_File()) {
    exit(0);
  }

  if (argc > 1) {
    alloc_only = atoi(argv[1]);
    printf("Alloc only %d", alloc_only);
  }
  
  int index = 0;
  if(configure_an(index, alloc_only)){
    exit(0);
  }
  if(load_an(index,coin_config_obj[index].coin_id)){
    exit(0);
  }
  if(configure_guid(index, alloc_only)){
    exit(0);
  }
  if(load_guid(index,coin_config_obj[index].coin_id)){
    exit(0);
  }
  
  //srand(time(NULL));
  //pthread_t ptid_main[2];
  //pthread_create(&ptid_main[0], NULL, &main_raida_server, NULL);
  //pthread_create(&ptid_main[1], NULL, &main_NFT_server, NULL);

  init_tcp_socket();
  pthread_t ptid;
  pthread_create(&ptid, NULL, &backup_guid_thread, NULL);
 
  while(1) {
    tcp_listen_request();
  }
 
  return 0;
}
