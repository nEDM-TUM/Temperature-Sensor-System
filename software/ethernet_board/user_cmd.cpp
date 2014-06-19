#include "user_cmd.h"
#include "sock_stream.h"
#include "configuration.h"
#include "socket.h"
#include "packet.h"
#include <util/delay.h>

// set debug mode
// #define DEBUG

int8_t handleIp();
int8_t handlePort();
int8_t handleMac();
int8_t handleGw();
int8_t handleReset();
int8_t handleIpDB();
int8_t handlePortDB();
int8_t handleTwiaddr();
int8_t handleDoMeasurement();
int8_t handleScan();
int8_t handleViewMeasurement();
int8_t handleInterval();
int8_t handleLED();
int8_t handleHelp();

// parameter format string in progmem
const char Uint[] PROGMEM = "%u";
const char ULong[] PROGMEM = "%lu";
const char Uint_2_Char[] PROGMEM = "%u %u %c";
const char UintArrow_2[] PROGMEM = "%u > %u";
const char UintDot_4[] PROGMEM = "%u.%u.%u.%u";
const char UintDot_4Slash[] PROGMEM = "%u.%u.%u.%u/%u";
const char HexColon_6[] PROGMEM = "%x:%x:%x:%x:%x:%x";

// XXX cmdLen should be always the length of the registered cmd array below! 
#define DEFINED_CMD_COUNT 14
struct cmd cmds[]={
  {"ip", handleIp, UintDot_4Slash},
  {"port", handlePort, Uint},
  {"mac", handleMac, HexColon_6},
  {"gw", handleGw, UintDot_4},
  {"reset", handleReset, NULL},
  {"ip-db", handleIpDB, UintDot_4},
  {"port-db", handlePortDB, Uint},
  {"twiaddr", handleTwiaddr, UintArrow_2},
  {"s", handleScan, NULL},
  {"m", handleDoMeasurement, NULL},
  {"v", handleViewMeasurement, NULL},
  {"i", handleInterval, ULong},
  {"led", handleLED, Uint_2_Char},
  // XXX the help handler should be always at the end
  {"help", handleHelp, NULL}
};

const char Usage[] PROGMEM = "usage: ";
const char New[] PROGMEM = "(NEW)";
const char Colon[] PROGMEM = ": ";

const char UpdateOption[] PROGMEM = "\nupdate option:\n\treset";
const char WillReset[] PROGMEM = "The ethernet service will be reset, the future login is ";
const char CmdNotFound[] PROGMEM = "cmd not found! type 'help' to view options";

char cmdBuff[MAX_CMD_LEN];

uint8_t print4dotarr(FILE *stream, uint8_t * arr){
  return fprintf_P(stream, UintDot_4, arr[0], arr[1], arr[2], arr[3]);
}

int8_t handleViewMeasurement(){
	uint8_t socket = stream_get_sock();
	data_request[socket] = (!data_request[socket]);
  return 1;
}

int8_t handleInterval(){
  int16_t paramsCount=0;
	uint32_t interval_tmp;
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, ULong, &interval_tmp);
	if(paramsCount == 1){
		measure_interval = interval_tmp*1000;
    fputs_P(New, &sock_stream);
    result = SUCCESS_PARAMS_PARSE;
	}
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, ULong, measure_interval/1000);
  return paramsCount;
}

void send_result(struct dummy_packet * packets){
  char buffer[10];
	uint8_t s;
	int16_t temp;
  for (s=0;s<8;s++){
    fprintf(&sock_stream, " | P%u: ", s+1);
    if(packets[s].header.error && packets[s].header.connected){
      fprintf(&sock_stream, " ERROR %u ", packets[s].header.error);
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

int8_t handleDoMeasurement(){
#ifdef DEBUG
	printf("handle do measure\n\r");
#endif
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
  return NO_PARAMS_PARSE;
}

void accessScan(void){
	// FIXME: socket number is hardcoded here
	uint8_t sock = 0;
	uint8_t i;
	num_boards = twi_scan(scanresults, 20);
  fprintf(&sock_stream, "found boards");
#ifdef DEBUG
	printf("found boards: ");
#endif
	for (i=0;i<num_boards;i++){
		uint8_t t;
    fprintf(&sock_stream, " %u", scanresults[i]);
#ifdef DEBUG
		printf(" %u", scanresults[i]);
#endif
	}
  fprintf(&sock_stream,  "\n");
#ifdef DEBUG
	printf("\n\r");
#endif
  sock_stream_flush();
}

int8_t handleScan(){
#ifdef DEBUG
	printf("handle scan\n\r");
#endif
	twi_access_fun = accessScan;
	ui_state = UI_TWILOCK;
  return 1;
}

void accessLED(void){
	char onoff;
	uint16_t addr;
	uint16_t lednum;
  uint8_t paramsCount = fscanf_P(&sock_stream, Uint_2_Char, &addr, &lednum, &onoff);
	if(paramsCount == 3){
		if(twi_set_led(addr, onoff=='1', lednum)){
			fprintf(&sock_stream, "success\n");
		}else{
			fprintf(&sock_stream, "failed\n");
		}
	}else{
		fprintf(&sock_stream, "led <addr> <num> <1|0>\n");
	}
}

int8_t handleLED(){
#ifdef DEBUG
	printf("handle scan\n\r");
#endif
	twi_access_fun = accessLED;
	ui_state = UI_TWILOCK;
	// FIXME: replace 2 with macro
  return SUSPEND;
}

void accessTwiaddr(void){
  int8_t success = 0;
	uint16_t old_addr, new_addr;
	if(fscanf(&sock_stream, "%u>%u", &old_addr, &new_addr) ==2){
		printf("par %u, %u", old_addr, new_addr);
		//if(buff[4]=='g'){
		success = 1;
		if(twi_set_address(old_addr, new_addr)){
			fprintf(&sock_stream, "success\n");
		}else{
			fprintf(&sock_stream, "failed\n");
		}
	}
}

int8_t handleTwiaddr(){
	//twi_access_fun = handleTwiaddr_access;
	//ui_state = UI_TWILOCK;
  int8_t result = FAILED_PARAMS_PARSE;
	uint16_t old_addr=1;
	uint16_t new_addr=1;
  int16_t paramsCount=0;
  paramsCount = fscanf_P(&sock_stream, UintArrow_2, &old_addr, &new_addr);
	if(paramsCount == 2){
#ifdef DEBUG
		printf("change twi addr %u to %u\n\r", old_addr, new_addr);
#endif
		//if(buff[4]=='g'){
		result = SUCCESS_PARAMS_PARSE;
		//if(twi_set_address(old_addr, new_addr)){
		//	fprintf(&sock_stream, "success\n");
		//}else{
		//	fprintf(&sock_stream, "failed\n");
		//}
	}
  return result;
  //return SUSPEND;
}


int8_t handleIp(){
  int8_t index;
  int16_t paramsCount=0;
  uint16_t ipTMP[4];
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, UintDot_4Slash, ipTMP, ipTMP+1, ipTMP+2, ipTMP+3, &(cfg.subnet));
  fprintf_P(&sock_stream, PSTR("TEST %d TEST\n"), paramsCount);
  if(paramsCount==5){
    for(index=0; index<4; index++){
      cfg.ip[index] = (uint8_t)ipTMP[index];
    }
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, UintDot_4Slash, cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.subnet);
  if(paramsCount==5){
    fprintf_P(&sock_stream, UpdateOption);
    result = SUCCESS_PARAMS_PARSE;
  }
  return result;
}

int8_t handleGw(){
  int8_t index;
  int16_t paramsCount=0;
  uint16_t gwTMP[4];
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, UintDot_4, gwTMP, gwTMP+1, gwTMP+2, gwTMP+3);
  if(paramsCount == 4){
    for(index=0; index<4; index++){
      cfg.gw[index] = (uint8_t)gwTMP[index];
    } 
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  print4dotarr(&sock_stream, cfg.gw);
  if(paramsCount==4){
    fprintf_P(&sock_stream, UpdateOption);
    result = SUCCESS_PARAMS_PARSE;
  }
  return result;
}

int8_t handleMac(){
  int8_t index;
  int16_t paramsCount=0;
  uint16_t macTMP[6];
  paramsCount = fscanf_P(&sock_stream, HexColon_6, macTMP, macTMP+1, macTMP+2, macTMP+3, macTMP+4, macTMP+5);
  if(paramsCount == 6){
    for(index=0; index<6; index++){
      cfg.mac[index] = (uint8_t)macTMP[index];
    }
    fputs_P(New, &sock_stream);
  }else if(paramsCount > 0){
    // not success by parsing
    return 0;
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, HexColon_6, cfg.mac[0], cfg.mac[1], cfg.mac[2], cfg.mac[3], cfg.mac[4], cfg.mac[5]);
  if(paramsCount==6){
    fprintf_P(&sock_stream, UpdateOption);
  }
  return 1;
}

int8_t handlePort(){
  int16_t paramsCount=fscanf_P(&sock_stream, Uint, &(cfg.port));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
    fputs_P(Colon, &sock_stream);
    fprintf_P(&sock_stream, Uint, cfg.port);
    fputs_P(UpdateOption, &sock_stream);
    return 1;
  }
  if(paramsCount < 0){
    fputs_P(Colon, &sock_stream);
    fprintf(&sock_stream, Uint, cfg.port);
    return 1;
  }
  return 0;
}

int8_t handleReset(){
  uint8_t index;
  // broadcast
  fprintf_P(&sock_stream, PSTR("\n"));
  stream_set_sock(MAX_SERVER_SOCK_NUM); 
  fprintf(&sock_stream, "%S", WillReset);
  print4dotarr(&sock_stream, cfg.ip);
  fprintf_P(&sock_stream, PSTR(" "));
  fprintf_P(&sock_stream, Uint, cfg.port);
  fprintf_P(&sock_stream, PSTR("\n"));
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

int8_t handleIpDB(){
  int8_t index;
  uint16_t ipTMP[4];
  int16_t paramsCount=0;
  paramsCount = fscanf_P(&sock_stream, UintDot_4, ipTMP, ipTMP+1, ipTMP+2, ipTMP+3);
  if(paramsCount==4){
    for(index=0; index<4; index++){
      cfg.ip_db[index] = (uint8_t)ipTMP[index];
    }
    fputs_P(New, &sock_stream);
  }else if(paramsCount >= 0){
    // not success by parsing
    return 0;
  }
  fputs_P(Colon, &sock_stream);
  print4dotarr(&sock_stream, cfg.ip_db);
  if(paramsCount==4){
    fprintf(&sock_stream, "%S", UpdateOption);
  }
  return 1;
}

int8_t handlePortDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, Uint, &(cfg.port_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
    fputs_P(Colon, &sock_stream);
    fprintf_P(&sock_stream, Uint, cfg.port);
    fputs_P(UpdateOption, &sock_stream);
    return 1;
  }
  if(paramsCount < 0){
    fputs_P(Colon, &sock_stream);
    fprintf(&sock_stream, Uint, cfg.port);
    return 1;
  }
  return 0;
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

void printOption(struct cmd cmd){
  fputs_P(PSTR("\n\t"), &sock_stream);
  fputs(cmd.name, &sock_stream);
  fputs_P(PSTR(" "), &sock_stream);
  fputs_P(cmd.param_format, &sock_stream);
}

int8_t handleHelp(){
  int8_t index;
  for(index= 0; index<DEFINED_CMD_COUNT - 1; index++){
    fputs_P(Usage, &sock_stream);
    printOption(cmds[index]);
  }
  return 1;
}

uint8_t execCMD(uint8_t sock, char * buff){
  int8_t index;
  // TODO some prefix
  struct cmd cmd;
  printf("Compare %s\n\r", buff);
  for(index= 0; index<DEFINED_CMD_COUNT ; index++){
    cmd = cmds[index];
    if(!(strcmp(buff, cmd.name))){
      fputs(cmd.name, &sock_stream);
			uint8_t handle_state = cmd.handle();
      if(!handle_state && cmd.param_format!=NULL){
        fputs_P(PSTR(" "), &sock_stream);
        fputs_P(Usage, &sock_stream);
        printOption(cmd);
      }
      fputs_P(PSTR("\n"), &sock_stream);
      sock_stream_flush();
			if(handle_state == 2){
				// FIXME: replace 2 with macro
				return 2;
			}else{
				return 1;
			}
    }
  }
  fputs_P(CmdNotFound, &sock_stream);
  sock_stream_flush();
}

uint8_t handleCMD(uint8_t sock){
  uint8_t pointer=0;
  int16_t b;
  int8_t cmd_flag=1;
  stream_set_sock(sock); 
  while((b=fgetc(&sock_stream)) != EOF){
    if(b == ' ' || b == '\n' || b == ';'){
			uint8_t cmd_success = 0;
      if(pointer>0){
        cmdBuff[pointer++] = '\0';
        cmd_success = execCMD(sock, cmdBuff);
        cmd_flag = 0;
      }
      if(b == '\n' || b == ';'){
        cmd_flag=1;
      }
      pointer = 0;
			if (cmd_success == 2){
				// FIXME: replace 2 with macro
				// we have to wait, and not eat the socket contents
				// as we are waiting for the twi bus to become free, to
				// continue processing the current command
				//
				// with the return value, we also notify server, to not
				// handle any more sockets:
				return 2;
			}else{
				continue;
			}
    }
    if(pointer >= MAX_CMD_LEN){
      pointer = 0;
    }
    if(cmd_flag){
      cmdBuff[pointer++] = b;
    }
  }
	return 1;
}

