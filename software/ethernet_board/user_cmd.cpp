#include "user_cmd.h"

uint8_t print4dotarr(FILE *stream, uint8_t * arr){
  return fprintf(stream, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
}

void handleIp(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t ipTMP[4];
  uint8_t subnetTMP; 
  fprintf(&sock_stream, "ip: ");
  paramsCount = fscanf(&sock_stream, "%u.%u.%u.%u/%u", ipTMP, ipTMP+1, ipTMP+2, ipTMP+3, &subnet);
  if(paramsCount==5){
    for(index=0; index<4; index++){
      ip[index] = ipTMP[index];
    }
  }
  print4dotarr(&sock_stream, ip);
  fprintf(&sock_stream, "/%d", subnet);
  if(paramsCount!=5){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleGw(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t gwTMP[4];
  fprintf(&sock_stream, "gw: ");
  paramsCount = sscanf(paramsStr, "%u.%u.%u.%u", gwTMP, gwTMP+1, gwTMP+2, gwTMP+3);
  if(paramsCount == 4){
    for(index=0; index<4; index++){
      gw[index] = gwTMP[index];
    }
  }
  print4dotarr(&sock_stream, gw);
  if(paramsCount!=4){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleMac(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t macTMP[6];
  fprintf(&sock_stream, "mac: ");
  paramsCount = sscanf(paramsStr, "%x:%x:%x:%x:%x:%x", macTMP, macTMP+1, macTMP+2, macTMP+3, macTMP+4, macTMP+5);
  if(paramsCount == 6){
    for(index=0; index<4; index++){
      mac[index] = macTMP[index];
    }
  }
  fprintf(&sock_stream, "%d:%d:%d:%d:%d:%d", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  if(paramsCount!=6){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handlePort(uint8_t sock, char* paramsStr){
  fprintf(&sock_stream, "port: ");
  if(sscanf(paramsStr, "%u", &port)==1){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleReset(uint8_t sock, char* paramsStr){
  uint8_t index;
  // broadcast
  stream_set_sock(MAX_SERVER_SOCK_NUM); 
  fprintf(&sock_stream, "%S", WillReset);
  print4dotarr(&sock_stream, ip);
  fprintf(&sock_stream, ":%d\n", port);
  sock_stream_flush();
  for(index= 0; index<MAX_SERVER_SOCK_NUM; index++){
    disconnect(index);
  }
  // Wait a second to close all sockets
  _delay_ms(1000);
  // TODO LEDs action
  for(index= 0; index<MAX_SERVER_SOCK_NUM; index++){
    if(W5100.readSnSR(index) != SnSR::CLOSED){
      // If a socket is still not closed, force it
      close(index);
    }
  }
  beginService();
}

void handleIpDB(uint8_t sock, char* paramsStr){
  int8_t index;
  uint8_t ipTMP[4];
  int16_t paramsCount=0;
  fprintf(&sock_stream, "db ip: ");
  paramsCount = sscanf(paramsStr, "%u.%u.%u.%u/%u", ipTMP, ipTMP+1, ipTMP+2, ipTMP+3);
  if(paramsCount==4){
    for(index=0; index<4; index++){
      ip_db[index] = ipTMP[index];
    }
  }
  print4dotarr(&sock_stream, ip_db);
  if(paramsCount!=4){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handlePortDB(uint8_t sock, char* paramsStr){
  fprintf(&sock_stream, "db port: ");
  if(sscanf(paramsStr, "%u", &port)==1){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  } 
}

void send_result(struct dummy_packet * packets, uint8_t sock){
  char buffer[10];
	uint8_t s;
	int16_t temp;
  for (s=0;s<8;s++){
    fprintf(&sock_stream, " | P%u: ", s+1);
    if(packets[s].header.error && packets[s].header.connected){
      fprintf(&sock_stream, " ERROR ");
    }
    if(packets[s].header.connected){
      switch(packets[s].header.type){
        case PACKET_TYPE_TSIC:
          temp =  ((struct tsic_packet *)(packets) )[s].temperature;
          fprintf(&sock_stream, "T = %d.%02d", temp/100, temp%100);
          //printf(" T = %d", ( (struct tsic_packet *)(packets) )[s].temperature);
          break;
        case PACKET_TYPE_HYT:
          fprintf(&sock_stream, "T = %d", ( (struct hyt_packet *)(packets) )[s].temperature);
          fprintf(&sock_stream, " H = %d", ( (struct hyt_packet *)(packets) )[s].humidity);
          break;
        default:
          fprintf(&sock_stream, "---?---");
          break;
      }
    }else{
      fprintf(&sock_stream,  "---nc---");
    }
    fprintf(&sock_stream,  "\n");
  }
}

void handleDoMeasurement(uint8_t sock, char* paramsStr){
	printf("handler do measure\n\r");
  twi_start_measurement(0);
	uint8_t iaddr;
	uint8_t addr;
  uint8_t state;
  struct dummy_packet received[8];
	for (iaddr=0;iaddr<num_boards;iaddr++){
		addr = scanresults[iaddr];
		//printf("# %u # ", addr);
		state = twi_receive_data(addr, ((uint8_t*)received),8*sizeof(struct dummy_packet));
    if (state){
      send_result(received, sock);
    }
  }


}

void handleScan_access(void){
	// FIXME: socket number is hardcoded here
	uint8_t sock = 0;
	uint8_t i;
	num_boards = twi_scan(scanresults, 20);
  fprintf(&sock_stream, "found boards");
	printf("found boards: ");
	for (i=0;i<num_boards;i++){
		printf("%u", scanresults[i]);
		uint8_t t;
    fprintf(&sock_stream, " %u", scanresults[i]);
	}
	printf("\n\r");
  fprintf(&sock_stream,  "\n");
}

void handleScan(uint8_t sock, char * paramsStr){
	//printf("sc\n\r");
	twi_access_fun = handleScan_access;
	ui_state = UI_TWILOCK;
}

void handleTwiaddr(uint8_t sock, char * paramsStr){
  uint8_t synerr = 1;
  if(paramsStr[0]!='\0'){
    uint8_t old_addr, new_addr;
    if(sscanf(paramsStr, "%u %u", &old_addr, &new_addr) ==2){
			printf("par %u, %u", old_addr, new_addr);
			printf("|%s|", paramsStr);
      //if(buff[4]=='g'){
      synerr = 0;
      if(twi_set_address(old_addr, new_addr)){
        fprintf(&sock_stream, "success\n");
      }else{
        fprintf(&sock_stream, "failed\n");
      }
    }
  }
  if (synerr){
    fprintf(&sock_stream, "Usage: twiaddr <old>%S", Addr);
  }
}

inline void sendError(uint8_t sock){
  fprintf(&sock_stream, "Error!\n");
}

char* cmpCMD(char* cmdstr, const char * cmd){
  while(*cmd!='\0' && cmdstr!='\0' && *cmd == *cmdstr){
    cmd++;
    cmdstr++;
  }
  if(*cmd=='\0'){
    if(*cmdstr=='\0'){
      return cmdstr; // equal, no param
    }else if(*cmdstr == ' '){
      cmdstr ++;
      return cmdstr; // equal, contains param
    }
  }
  return NULL; // not equal
}

void execCMD(uint8_t sock, char * buff){
  uint8_t params[MAX_PARAM_COUNT];
  int8_t paramCounter =0;
  int8_t index;
  char * cmpedBuff;
  struct cmd cmd;
  printf("Compare %s\n\r", buff);
  for(index= 0; index<cmdLen; index++){
    cmd = cmds[index];
    cmpedBuff = cmpCMD(buff, cmd.name);
    if(cmpedBuff!=NULL){
      cmd.handle(sock, cmpedBuff);
      sock_stream_flush();
      return;
    }
  }
  fprintf(&sock_stream, "Usage: TODO\n");
  sock_stream_flush();
}

void handleCMD(uint8_t sock){
  uint8_t pointer=0;
  int16_t b;
  int8_t cmd_flag=1;
  stream_set_sock(sock); 
  while((b=fgetc(&sock_stream)) != EOF){
    if(b == ' ' || b == '\n' || b == ';'){
      if(pointer>0){
        cmdBuff[pointer++] = '\0';
        execCMD(sock, cmdBuff);
        cmd_flag = 0;
      }
      if(b == '\n' || b == ';'){
        cmd_flag=1;
      }
      pointer = 0;
      continue;
    }
    if(pointer >= MAX_CMD_LEN){
      pointer = 0;
    }
    // XXX The first char should be alphabet in low case 
    if(pointer == 0 && (b<97 || b>122)){
      printf("first char %c %u\n\r", b,b);
      continue;
    }
    if(cmd_flag){
      cmdBuff[pointer++] = b;
    }
  }
}

