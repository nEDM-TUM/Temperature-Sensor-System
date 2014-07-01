#include "user_cmd.h"
#include "sock_stream.h"
#include "networking.h"
#include "config.h"
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
int8_t handleCookieDB();
int8_t handleNameDB();
int8_t handleDocDB();
int8_t handleFuncDB();
int8_t handleTestDB();
int8_t handleViewResponseDB();
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
const char Uint_2[] PROGMEM = "%u %u";
const char UintDot_4[] PROGMEM = "%u.%u.%u.%u";
const char UintDot_4Slash[] PROGMEM = "%u.%u.%u.%u/%u";
const char HexColon_6[] PROGMEM = "%x:%x:%x:%x:%x:%x";
const char LenLimit[] PROGMEM = "The command lenth is limited with ...";

// XXX cmdLen should be always the length of the registered cmd array below! 
#define DEFINED_CMD_COUNT 20
struct cmd cmds[]={
  {"ip", handleIp, UintDot_4Slash, NULL},
  {"port", handlePort, Uint, NULL},
  {"mac", handleMac, HexColon_6, NULL},
  {"gw", handleGw, UintDot_4, NULL},
  {"reset", handleReset, NULL, NULL},
  {"ip-db", handleIpDB, UintDot_4, NULL},
  {"port-db", handlePortDB, Uint, NULL},
  {"cookie-db", handleCookieDB, Uint, LenLimit},
  {"name-db", handleNameDB, Uint, LenLimit},
  {"doc-db", handleDocDB, Uint, LenLimit},
  {"func-db", handleFuncDB, Uint, LenLimit},
  {"test-db", handleTestDB, NULL, NULL},
  {"v-db", handleViewResponseDB, NULL, NULL},
  {"twiaddr", handleTwiaddr, Uint_2, NULL},
  {"s", handleScan, NULL, NULL},
  {"m", handleDoMeasurement, NULL, NULL},
  {"v", handleViewMeasurement, NULL, NULL},
  {"i", handleInterval, ULong, NULL},
  {"led", handleLED, Uint_2_Char, NULL},
  // XXX the help handler should be always at the end
  {"help", handleHelp, NULL, NULL}
};

const char Usage[] PROGMEM = "usage: ";
const char New[] PROGMEM = "(NEW)";
const char Colon[] PROGMEM = ": ";

const char UpdateOption[] PROGMEM = "\nupdate option:\n\treset";
const char CmdNotFound[] PROGMEM = "cmd not found! type 'help' to view options";

char cmdBuff[MAX_CMD_LEN];

uint8_t print4dotarr(FILE *stream, uint8_t * arr){
  return fprintf_P(stream, UintDot_4, arr[0], arr[1], arr[2], arr[3]);
}

int8_t handleViewMeasurement(){
	uint8_t serverSockIndex = stream_get_sock()-FIRST_SERVER_SOCK;
  if(serverSockIndex<MAX_SERVER_SOCK_NUM){
    data_request[serverSockIndex] = (!data_request[serverSockIndex]);
  }
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
  return result;
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
          fputs_P(PSTR("---?---"), &sock_stream);
          break;
      }
    }else{
      fputs_P(PSTR("---nc---"), &sock_stream);
    }
  }
  fputs_P(PSTR("\n"), &sock_stream);
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
  fputs_P(PSTR("found boards"), &sock_stream);
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
  fputs_P(PSTR("\n"), &sock_stream);
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
  return SUSPEND;
}

void accessLED(void){
	char onoff;
	uint16_t addr;
	uint16_t lednum;
  uint8_t paramsCount = fscanf_P(&sock_stream, Uint_2_Char, &addr, &lednum, &onoff);
	if(paramsCount == 3){
		if(twi_set_led(addr, onoff=='1', lednum)){
      fputs_P(PSTR("success\n"), &sock_stream);
		}else{
      fputs_P(PSTR("failed\n"), &sock_stream);
		}
	}else{
    fputs_P(PSTR("led <addr> <num> <1|0>\n"), &sock_stream);
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

	uint16_t old_addr=1;
	uint16_t new_addr=1;
  int16_t paramsCount=0;
  paramsCount = fscanf_P(&sock_stream, Uint_2, &old_addr, &new_addr);

	if(paramsCount == 2){
		if(new_addr < 8 || new_addr >=120 ){
      fputs_P(PSTR("7<addr<120\n"), &sock_stream);
			return;
		}
		if(twi_set_address(old_addr, new_addr)){
      fputs_P(PSTR("success\n"), &sock_stream);
		}else{
      fputs_P(PSTR("failed\n"), &sock_stream);
		}
	}else{
		fputs_P(PSTR("twiaddr <old> <new>\n"), &sock_stream);
	}
}

int8_t handleTwiaddr(){
	twi_access_fun = accessTwiaddr;
	ui_state = UI_TWILOCK;
  return SUSPEND;
}


int8_t handleIp(){
  int8_t index;
  int16_t paramsCount=0;
  uint16_t ipTMP[4];
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, UintDot_4Slash, ipTMP, ipTMP+1, ipTMP+2, ipTMP+3, &(cfg.subnet));
  if(paramsCount==5){
    for(index=0; index<4; index++){
      cfg.ip[index] = (uint8_t)ipTMP[index];
    }
    fputs_P(New, &sock_stream);
    result = SUCCESS_PARAMS_PARSE;
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, UintDot_4Slash, cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.subnet);
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(UpdateOption, &sock_stream);
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
    result = SUCCESS_PARAMS_PARSE;
  }
  fputs_P(Colon, &sock_stream);
  print4dotarr(&sock_stream, cfg.gw);
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(UpdateOption, &sock_stream);
  }
  return result;
}

int8_t handleMac(){
  int8_t index;
  int16_t paramsCount=0;
  uint16_t macTMP[6];
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, HexColon_6, macTMP, macTMP+1, macTMP+2, macTMP+3, macTMP+4, macTMP+5);
  if(paramsCount == 6){
    for(index=0; index<6; index++){
      cfg.mac[index] = (uint8_t)macTMP[index];
    }
    fputs_P(New, &sock_stream);
    result = SUCCESS_PARAMS_PARSE;
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, HexColon_6, cfg.mac[0], cfg.mac[1], cfg.mac[2], cfg.mac[3], cfg.mac[4], cfg.mac[5]);
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(UpdateOption, &sock_stream);
  }
  return result;
}

int8_t handlePort(){
  int16_t paramsCount=fscanf_P(&sock_stream, Uint, &(cfg.port));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleReset(){
  uint8_t index;
  // broadcast
  fputs_P(PSTR("\n"), &sock_stream);
  stream_set_sock(MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK); 
  fputs_P(PSTR("The ethernet service will be reset, the future login is "), &sock_stream);
  print4dotarr(&sock_stream, cfg.ip);
  fputs_P(PSTR(" "), &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port);
  fputs_P(PSTR("\n"), &sock_stream);
  sock_stream_flush();
  config_write(&cfg);
  for(index= FIRST_SERVER_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
    disconnect(index);
  }
  // Wait a second to close all sockets
  _delay_ms(1000);
  // TODO LEDs action
  for(index= FIRST_SERVER_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
    if(W5100.readSnSR(index) != SnSR::CLOSED){
      // If a socket is still not closed, force it
      close(index);
    }
  }
  net_beginService();
  return NO_PARAMS_PARSE;
}

int8_t handleIpDB(){
  int8_t index;
  uint16_t ipTMP[4];
  int16_t paramsCount=0;
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, UintDot_4, ipTMP, ipTMP+1, ipTMP+2, ipTMP+3);
  if(paramsCount==4){
    for(index=0; index<4; index++){
      cfg.ip_db[index] = (uint8_t)ipTMP[index];
    }
    fputs_P(New, &sock_stream);
    result = SUCCESS_PARAMS_PARSE;
  }
  fputs_P(Colon, &sock_stream);
  print4dotarr(&sock_stream, cfg.ip_db);
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(UpdateOption, &sock_stream);
  }
  return result;
}

int8_t handlePortDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, Uint, &(cfg.port_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port_db);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleCookieDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, PSTR("60%s"), &(cfg.cookie_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs_P(cfg.cookie_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleNameDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, PSTR("15%s"), &(cfg.name_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs_P(cfg.name_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleDocDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, PSTR("20%s"), &(cfg.doc_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs_P(cfg.doc_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleFuncDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, PSTR("25%s"), &(cfg.func_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs_P(cfg.func_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(UpdateOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleTestDB(){
  net_sendTestToDB();
  uint8_t serverSockIndex = stream_get_sock()-FIRST_SERVER_SOCK;
  if(serverSockIndex<MAX_SERVER_SOCK_NUM){
    db_response_request[serverSockIndex]= 1;
  }
  return NO_PARAMS_PARSE;
}

int8_t handleViewResponseDB(){
   uint8_t serverSockIndex = stream_get_sock()-FIRST_SERVER_SOCK;
  if(serverSockIndex<MAX_SERVER_SOCK_NUM){
    db_response_request[serverSockIndex]= !db_response_request[serverSockIndex];
  }
  return NO_PARAMS_PARSE;
}

inline void sendError(uint8_t sock){
  fputs_P(PSTR("Error!\n"), &sock_stream);
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
  if(cmd.param_format!=NULL){
  fputs_P(PSTR(" "), &sock_stream);
  fputs_P(cmd.param_format, &sock_stream);
  }
  if(cmd.comment!=NULL){
    fputs_P(PSTR(" ("), &sock_stream);
    fputs_P(cmd.comment, &sock_stream);
    fputs_P(PSTR(")"), &sock_stream);
  }
}

int8_t handleHelp(){
  int8_t index;
  for(index= 0; index<DEFINED_CMD_COUNT - 1; index++){
    fputs_P(Usage, &sock_stream);
    printOption(cmds[index]);
  }
  return 1;
}

uint8_t execCMD(uint8_t sock, char * buff, int8_t hasParams){
  int8_t index;
  struct cmd cmd;
  uint8_t handle_state;
#ifdef DEBUG
  printf("Compare %s\n\r", buff);
#endif
  for(index= 0; index<DEFINED_CMD_COUNT; index++){
    cmd = cmds[index];
    if(!(strcmp(buff, cmd.name))){
      fputs(cmd.name, &sock_stream);
			handle_state= cmd.handle();
      if(hasParams && handle_state==FAILED_PARAMS_PARSE && cmd.param_format!=NULL){
        fputs_P(PSTR(" "), &sock_stream);
        fputs_P(Usage, &sock_stream);
        printOption(cmd);
      }
      fputs_P(PSTR("\n"), &sock_stream);
			//if(handle_state != SUSPEND){
			//	fputs_P(PSTR("\n% "), &sock_stream);
			//}
      sock_stream_flush();
      return handle_state;
    }
  }
	//fputs_P(PSTR("\n% "), &sock_stream);
  fputs_P(CmdNotFound, &sock_stream);
  sock_stream_flush();
	// FIXME: what is return value here????
}

uint8_t ui_handleCMD(uint8_t sock){
	uint8_t pointer=0;
	int16_t b;
	int8_t flag_return=0;
	int8_t new_cmd_flag=1;
	uint8_t cmd_result = NO_PARAMS_PARSE;
	stream_set_sock(sock); 
	while((b=fgetc(&sock_stream)) != EOF){
		if(b == ' ' || b == '\n' || b == ';'){
			flag_return = 1;
			if(pointer>0){
				cmdBuff[pointer++] = '\0';
				if(b == '\n' || b == ';'){
					// if the read char is new line or ';', that means, from now on, a new cmd will be expected 
					new_cmd_flag=1;
				} else {
					new_cmd_flag = 0;
				}
				cmd_result = execCMD(sock, cmdBuff, !new_cmd_flag);
			}
			if(b == '\n' || b == ';'){
				// if the read char is new line or ';', that means, from now on, a new cmd will be expected 
				new_cmd_flag=1;
			}
			pointer = 0;
			if (cmd_result == SUSPEND){
				// we have to wait, and not eat the socket contents
				// as we are waiting for the twi bus to become free, to
				// continue processing the current command
				//
				// with the return value, we also notify server, to not
				// handle any more sockets:
				return cmd_result;
			}else{
				continue;
			}
		}
		if(pointer >= MAX_CMD_LEN){
			pointer = 0;
		}
		// if new cmd is expected, the read char will be stored into buff
		// if not, the read char will be ignored
		if(new_cmd_flag){
			cmdBuff[pointer++] = b;
		}
	}
	if (flag_return && (cmd_result != SUSPEND)){
		// FIXME: why is this not printed, when just return is pressed (without entering a cmd)
		fputs_P(PSTR("% "), &sock_stream);
		sock_stream_flush();
	}
	return cmd_result;
}

uint8_t ui_recvResponseDB(){
  int16_t b;
  uint8_t i;
  // FIXME improve
  stream_set_sock(DB_CLIENT_SOCK); 
  if((b=fgetc(&sock_stream)) == EOF){
    return 0;
  }
  for(i = 0;  i<MAX_SERVER_SOCK_NUM; i++){
    if(db_response_request[i]){
      stream_set_sock(i+FIRST_SERVER_SOCK); 
      fputs_P(PSTR("Response from DB:\n\n"), &sock_stream);
    }
  }
  while(true){
    stream_set_sock(DB_CLIENT_SOCK); 
    if((b=fgetc(&sock_stream)) == EOF){
      return 1;
    }
    for(i = 0;  i<MAX_SERVER_SOCK_NUM; i++){
      if(db_response_request[i]){
        stream_set_sock(i+FIRST_SERVER_SOCK); 
        fputc(b, &sock_stream);
      }
    }
  }
  return 1;
}
