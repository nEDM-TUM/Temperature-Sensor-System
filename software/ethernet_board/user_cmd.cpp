#include "user_cmd.h"
#include "sock_stream.h"
#include "configuration.h"
#include "socket.h"
#include "packet.h"
#include <util/delay.h>

void handleIp();
void handlePort();
void handleMac();
void handleGw();
void handleReset();
void handleIpDB();
void handlePortDB();
void handleTwiaddr();
void handleDoMeasurement();
void handleScan();
void handleViewMeasurement();
void handleInterval();

// XXX cmdLen should be always the length of the registered cmd array below! 
#define DEFINED_CMD_COUNT 12
struct cmd cmds[]={
  {"ip", handleIp},
  {"port", handlePort},
  {"mac", handleMac},
  {"gw", handleGw},
  {"reset", handleReset},
  {"ip-db", handleIpDB},
  {"port-db", handlePortDB},
  {"twiaddr", handleTwiaddr},
  {"s", handleScan},
  {"m", handleDoMeasurement},
  {"v", handleViewMeasurement},
  {"i", handleInterval}
};
const char WillSet[] PROGMEM = " will be set to ";
const char IfUpdate[] PROGMEM = " (update option: reset)\n";
const char UpdateOption[] PROGMEM = ", update option: ";
const char WillReset[] PROGMEM = "The ethernet service will be reset, the future login is ";
const char Addr[] PROGMEM = " <addr>\n";

char cmdBuff[MAX_CMD_LEN];

uint8_t print4dotarr(FILE *stream, uint8_t * arr){
  return fprintf(stream, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
}

void handleViewMeasurement(){
	uint8_t socket = stream_get_sock();
	data_request[socket] = (!data_request[socket]);
}

void handleInterval(){
  int16_t paramsCount=0;
	uint32_t interval_tmp;
  paramsCount = fscanf(&sock_stream, "%lu", &interval_tmp);
	if(paramsCount == 1){
		measure_interval = interval_tmp*1000;
		fprintf(&sock_stream, "OK\n");
	}
  fprintf(&sock_stream, "interval: %lu\n", measure_interval/1000);
}

void handleIp(){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t ipTMP[4];
  fprintf(&sock_stream, "ip: ");
  paramsCount = fscanf(&sock_stream, "%u.%u.%u.%u/%u", ipTMP, ipTMP+1, ipTMP+2, ipTMP+3, &(cfg.subnet));
  if(paramsCount==5){
    for(index=0; index<4; index++){
      cfg.ip[index] = ipTMP[index];
    }
  }
  print4dotarr(&sock_stream, cfg.ip);
  fprintf(&sock_stream, "/%d", cfg.subnet);
  if(paramsCount!=5){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleGw(){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t gwTMP[4];
  fprintf(&sock_stream, "gw: ");
  paramsCount = fscanf(&sock_stream, "%u.%u.%u.%u", gwTMP, gwTMP+1, gwTMP+2, gwTMP+3);
  if(paramsCount == 4){
    for(index=0; index<4; index++){
      cfg.gw[index] = gwTMP[index];
    }
  }
  print4dotarr(&sock_stream, cfg.gw);
  if(paramsCount!=4){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleMac(){
  int8_t index;
  int16_t paramsCount=0;
  uint8_t macTMP[6];
  fprintf(&sock_stream, "mac: ");
  paramsCount = fscanf(&sock_stream, "%x:%x:%x:%x:%x:%x", macTMP, macTMP+1, macTMP+2, macTMP+3, macTMP+4, macTMP+5);
  if(paramsCount == 6){
    for(index=0; index<4; index++){
      cfg.mac[index] = macTMP[index];
    }
  }
  fprintf(&sock_stream, "%d:%d:%d:%d:%d:%d", cfg.mac[0], cfg.mac[1], cfg.mac[2], cfg.mac[3], cfg.mac[4], cfg.mac[5]);
  if(paramsCount!=6){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handlePort(){
  fprintf(&sock_stream, "port: ");
  if(fscanf(&sock_stream, "%u", &(cfg.port))==1){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handleReset(){
  uint8_t index;
  // broadcast
  stream_set_sock(MAX_SERVER_SOCK_NUM); 
  fprintf(&sock_stream, "%S", WillReset);
  print4dotarr(&sock_stream, cfg.ip);
  fprintf(&sock_stream, ":%d\n", cfg.port);
  sock_stream_flush();
#ifdef EEPROM
  // TODO update eeprom
#endif
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

void handleIpDB(){
  int8_t index;
  uint8_t ipTMP[4];
  int16_t paramsCount=0;
  fprintf(&sock_stream, "db ip: ");
  paramsCount = fscanf(&sock_stream, "%u.%u.%u.%u/%u", ipTMP, ipTMP+1, ipTMP+2, ipTMP+3);
  if(paramsCount==4){
    for(index=0; index<4; index++){
      cfg.ip_db[index] = ipTMP[index];
    }
  }
  print4dotarr(&sock_stream, cfg.ip_db);
  if(paramsCount!=4){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  }
}

void handlePortDB(){
  fprintf(&sock_stream, "db port: ");
  if(fscanf(&sock_stream, "%u", &(cfg.port_db))==1){
    fprintf(&sock_stream, "%S", UpdateOption);
  }else{
    fprintf(&sock_stream, "%S", IfUpdate);
  } 
}

void send_result(struct dummy_packet * packets){
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
  }
  fprintf(&sock_stream,  "\n");
}

void handleDoMeasurement(){
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
      send_result(received);
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

void handleScan(){
	//printf("sc\n\r");
	twi_access_fun = handleScan_access;
	ui_state = UI_TWILOCK;
}

void handleTwiaddr(){
  uint8_t synerr = 1;
  uint8_t old_addr, new_addr;
  if(fscanf(&sock_stream, "%u %u", &old_addr, &new_addr) ==2){
    printf("par %u, %u", old_addr, new_addr);
    //if(buff[4]=='g'){
    synerr = 0;
    if(twi_set_address(old_addr, new_addr)){
      fprintf(&sock_stream, "success\n");
    }else{
      fprintf(&sock_stream, "failed\n");
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
  int8_t index;
  struct cmd cmd;
  printf("Compare %s\n\r", buff);
  for(index= 0; index<DEFINED_CMD_COUNT ; index++){
    cmd = cmds[index];
    if(!(strcmp(buff, cmd.name))){
      cmd.handle();
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

