#include "user_cmd.h"
#include "sock_stream.h"
#include "networking.h"
#include "config.h"
#include "socket.h"
#include "packet.h"
#include "wdt_delay.h"
#include <util/delay.h>

// set debug mode
// #define DEBUG

int8_t handleIp();
int8_t handlePort();
int8_t handleMac();
int8_t handleGw();
int8_t handleSendDB();
int8_t handleIpDB();
int8_t handlePortDB();
int8_t handleCookieDB();
int8_t handleNameDB();
int8_t handleDocDB();
int8_t handleFuncDB();
int8_t handleReset();
int8_t handleResetDB();
int8_t handleStore();
int8_t handleViewResponseDB();
int8_t handleTwiaddr();
int8_t handleScan();
int8_t handleViewMeasurement();
int8_t handleInterval();
int8_t handleLED();
int8_t handleClose();
int8_t handleHelp();

// parameter format string in progmem
const char Int[] PROGMEM = "%d";
const char Uint[] PROGMEM = "%u";
const char ULong[] PROGMEM = "%lu";
const char Uint_2_Char[] PROGMEM = "%u %u %c";
const char Uint_2[] PROGMEM = "%u %u";
const char UintDot_4[] PROGMEM = "%u.%u.%u.%u";
const char UintDot_4Slash[] PROGMEM = "%u.%u.%u.%u/%u";
const char HexColon_6[] PROGMEM = "%x:%x:%x:%x:%x:%x";
const char FormatCookieDB[] PROGMEM = "%" STR(DB_COOKIE_SIZE) "s";
const char FormatNameDB[] PROGMEM = "%" STR(DB_NAME_SIZE) "s";
const char FormatDocDB[] PROGMEM = "%" STR(DB_DOC_SIZE) "s";
const char FormatFuncDB[] PROGMEM = "%" STR(DB_FUNC_SIZE) "s";

const char SendDBComment[] PROGMEM = "<1|0>\n\t\t1: Enable sending data to database; 2: Disable sending data to database";
const char ViewResponseDBComment[] PROGMEM = "Toggle display of database reponse";
const char ResetComment[] PROGMEM = "Store changes and reset all network services"; 
const char ResetDBComment[] PROGMEM = "Store changes and reset connection to database"; 
const char StoreComment[] PROGMEM = "Permanently store changes"; 

const char TwiaddrComment[] PROGMEM = "<old> <new>\n\t\tChange board I2C address. The address should be between 7 and 120.";
const char ScanComment[] PROGMEM = "Scan for connected collection boards";
const char ViewMeasurementComment[] PROGMEM = "Toggle displaying of current measurement results";
const char IntervalComment[] PROGMEM = "<seconds>\n\t\tChange the measurement interval";
const char LEDComment[] PROGMEM = "<board addr> <led num> <1|0>\n\t\tControl the leds of collector boards, 1: led on; 0: led off";
const char IpComment[] PROGMEM = "<ip/subnet>\n\t\tChange the IP address and subnet";
const char PortComment[] PROGMEM = "<port>\n\t\tChange the TCP port of this user interface";
const char MacComment[] PROGMEM = "<mac>\n\t\tChange the ethernet mac";
const char GwComment[] PROGMEM = "<gw>\n\t\tChange the gateway";
const char IpDBComment[] PROGMEM = "<ip>\n\t\tChange the IP address of database";
const char PortDBComment[] PROGMEM = "<port>\n\t\tChange port of database";
const char CookieDBComment[] PROGMEM = "<cookie>\n\t\tChange cookie for communication with database (max " STR(DB_COOKIE_SIZE) " chars)";
const char NameDBComment[] PROGMEM = "<name>\n\t\tChange database name (max " STR(DB_NAME_SIZE) " chars)";
const char DocDBComment[] PROGMEM = "<name>\n\t\tChange json document name (max " STR(DB_DOC_SIZE) " chars)";
const char FuncDBComment[] PROGMEM = "<name>\n\t\tChange insert into database function name (max " STR(DB_FUNC_SIZE) " chars)";
const char CloseComment[] PROGMEM = "Close user interface savely";

struct cmd baCMD = {"ba", handleTwiaddr, Uint_2, TwiaddrComment};
struct cmd ledCMD = {"led", handleLED, Uint_2_Char, LEDComment};
// XXX this should always be the length of the registered cmd array below! 
#define DEFINED_CMD_COUNT 22

struct cmd cmds[]={
  // Network cmd
  {"ip", handleIp, UintDot_4Slash, IpComment},
  {"p", handlePort, Uint, PortComment},
  {"mac", handleMac, HexColon_6, MacComment},
  {"gw", handleGw, UintDot_4, GwComment},
  // DB cmd
  {"d.s", handleSendDB, Int, SendDBComment},
  {"d.ip", handleIpDB, UintDot_4, IpDBComment},
  {"d.p", handlePortDB, Uint, PortDBComment},
  {"d.ck", handleCookieDB, FormatCookieDB, CookieDBComment},
  {"d.n", handleNameDB, FormatNameDB, NameDBComment},
  {"d.d", handleDocDB, FormatDocDB, DocDBComment},
  {"d.f", handleFuncDB, FormatFuncDB, FuncDBComment},
  {"d.v", handleViewResponseDB, NULL, ViewResponseDBComment},
  // eeprom cmd
  {"res", handleReset, NULL, ResetComment},
  {"d.re", handleResetDB, NULL, ResetDBComment},
  {"sto", handleStore, NULL, StoreComment},
  //boards cmd
  baCMD, //{"ba", handleTwiaddr, Uint_2, TwiaddrComment}
  {"s", handleScan, NULL, ScanComment},
  {"v", handleViewMeasurement, NULL, ViewMeasurementComment},
  {"i", handleInterval, ULong, IntervalComment},
  ledCMD, //{"led", handleLED, Uint_2_Char, LEDComment}
  {"c", handleClose, NULL, CloseComment},
  // XXX the help handler should always be at the end
  {"help", handleHelp, NULL, NULL}
};

const char Usage[] PROGMEM = "Available commands:";
const char New[] PROGMEM = "(NEW)";
const char Colon[] PROGMEM = ": ";

const char ResetOption[] PROGMEM = "\nto store changes and reset network services for changes to become effective, type\n\tres";
const char ResetDBOption[] PROGMEM = "\nto store changes and reset the connection to database for changes to become effective, type\n\td.re";
const char StoreOption[] PROGMEM = "\nto store changes type\n\tsto";
const char CmdNotFound[] PROGMEM = "cmd not found! type 'help' to view options\n";

char cmdBuff[MAX_CMD_LEN];

void printOption(struct cmd);

uint8_t print4dotarr(FILE *stream, uint8_t * arr){
  return fprintf_P(stream, UintDot_4, arr[0], arr[1], arr[2], arr[3]);
}

int8_t handleViewMeasurement(){
	uint8_t serverSockIndex = stream_get_sock()-FIRST_SERVER_SOCK;
  if(serverSockIndex<MAX_SERVER_SOCK_NUM){
    data_request[serverSockIndex] = (!data_request[serverSockIndex]);
  }
  return NO_PARAMS_PARSE;
}

int8_t handleInterval(){
  int16_t paramsCount=0;
	uint32_t interval_tmp;
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, ULong, &interval_tmp);
	if(paramsCount == 1){
		cfg.measure_interval = interval_tmp*1000;
    fputs_P(New, &sock_stream);
    result = SUCCESS_PARAMS_PARSE;
	}
  fputs_P(Colon, &sock_stream);
  fprintf_P(&sock_stream, ULong, cfg.measure_interval/1000);
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(StoreOption, &sock_stream);
  }
  return result;
}

void send_result(struct dummy_packet * packets){
  char buffer[10];
	uint8_t s;
	int16_t value;
  for (s=0;s<8;s++){
    fprintf_P(&sock_stream, PSTR(" | S%u: "), s);
    if(packets[s].header.error && packets[s].header.connected){
      fprintf_P(&sock_stream, PSTR("ERROR %4u"), packets[s].header.error);
    }
    if(packets[s].header.connected){
      switch(packets[s].header.type){
        case PACKET_TYPE_TSIC:
          value =  ((struct tsic_packet *)(packets) )[s].temperature;
          fprintf_P(&sock_stream, PSTR("T = %3d.%02d"), value/100, value%100);
          break;
        case PACKET_TYPE_HYT:
          value =  ((struct hyt_packet *)(packets) )[s].temperature;
          fprintf_P(&sock_stream, PSTR("T = %3d.%02d | "), value/100, value%100);
          value =  ((struct hyt_packet *)(packets) )[s].humidity;
          fprintf_P(&sock_stream, PSTR("H = %3d.%02d"), value/100, value%100);
          break;
        default:
          fputs_P(PSTR("----xx----"), &sock_stream);
          break;
      }
    }else{
      fputs_P(PSTR("----nc----"), &sock_stream);
    }
  }
}

void accessScan(void){
	uint8_t i;
	num_boards = twi_scan(scanresults, 20);
  if(num_boards == 0){
    fputs_P(PSTR("No boards found!"), &sock_stream);
  } else {
    fputs_P(PSTR("found boards:"), &sock_stream);
#ifdef DEBUG
    puts_P(PSTR("found boards:"));
#endif
    for (i=0;i<num_boards;i++){
      uint8_t t;
      fprintf(&sock_stream, " %u", scanresults[i]);
#ifdef DEBUG
      printf(" %u", scanresults[i]);
#endif
    }
  }
  fputc('\n', &sock_stream);
#ifdef DEBUG
	printf("\n\r");
#endif
  sock_stream_flush();
}

int8_t handleScan(){
#ifdef DEBUG
	printf_P(PSTR("handle scan\n\r"));
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
      fputs_P(PSTR("OK :)\n"), &sock_stream);
		}else{
      fputs_P(PSTR("Failed :(\n"), &sock_stream);
		}
	}else{
		fputs_P(Usage, &sock_stream);
    printOption(ledCMD);
    fputc('\n', &sock_stream);
	}
}

int8_t handleLED(){
#ifdef DEBUG
	printf_P(PSTR("handle scan\n\r"));
#endif
	twi_access_fun = accessLED;
	ui_state = UI_TWILOCK;
  return SUSPEND;
}

void accessTwiaddr(void){
	uint16_t old_addr=1;
	uint16_t new_addr=1;
  int16_t paramsCount=0;
  paramsCount = fscanf_P(&sock_stream, Uint_2, &old_addr, &new_addr);

	if(paramsCount == 2){
		if(new_addr < 8 || new_addr >=120 ){
      fputs_P(PSTR("7 < addr < 120\n"), &sock_stream);
			return;
		}
		if(twi_set_address(old_addr, new_addr)){
      fputs_P(PSTR("OK :)\n"), &sock_stream);
		}else{
      fputs_P(PSTR("Failed :(\n"), &sock_stream);
		}
	}else{
		fputs_P(Usage, &sock_stream);
    printOption(baCMD);
    fputc('\n', &sock_stream);
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
    fputs_P(ResetOption, &sock_stream);
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
    fputs_P(ResetOption, &sock_stream);
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
    fputs_P(ResetOption, &sock_stream);
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
    fputs_P(ResetOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleStore(){
  config_write(&cfg);
  fputs_P(PSTR(" done!"), &sock_stream);
  return NO_PARAMS_PARSE;
}

int8_t handleResetDB(){
  uint8_t index;
  disconnect(DB_CLIENT_SOCK);
  config_write(&cfg);
  fputc('\n', &sock_stream);
  fputs_P(PSTR("The connection to database will be reset, the new database address is "), &sock_stream);
  print4dotarr(&sock_stream, cfg.ip_db);
  fputc(':', &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port_db);
  sock_stream_flush();
  // Wait a second to close all sockets
  if(W5100.readSnSR(DB_CLIENT_SOCK) != SnSR::CLOSED){
    close(DB_CLIENT_SOCK);
  }
  return NO_PARAMS_PARSE;
}
int8_t handleReset(){
  uint8_t index;
  // broadcast
  stream_set_sock(MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK); 
  fputc('\n', &sock_stream);
  fputs_P(PSTR("The ethernet service is restarting, the future address will be "), &sock_stream);
  print4dotarr(&sock_stream, cfg.ip);
  fputc(' ', &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port);
  fputc('\n', &sock_stream);
  sock_stream_flush();
  config_write(&cfg);
  for(index= DB_CLIENT_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
    disconnect(index);
  }
  // Wait a some time to close all sockets
  wdt_delay_ms(500);
  for(index= DB_CLIENT_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
    if(W5100.readSnSR(index) != SnSR::CLOSED){
      // If a socket is still not closed, force it
      close(index);
    }
  }
  net_beginService();
  return NO_PARAMS_PARSE;
}

int8_t handleClose(){
  uint8_t index;
  fputs_P(PSTR("\nThe ethernet service will be closed, the future login is "), &sock_stream);
  print4dotarr(&sock_stream, cfg.ip);
  fputc(' ', &sock_stream);
  fprintf_P(&sock_stream, Uint, cfg.port);
  fputc('\n', &sock_stream);
  sock_stream_flush();
  close(stream_get_sock());
  return NO_PARAMS_PARSE;
}

int8_t handleSendDB(){
  int16_t paramsCount=0;
  int16_t param=0;
  int8_t result = FAILED_PARAMS_PARSE;
  paramsCount = fscanf_P(&sock_stream, Int, &param);
  if(paramsCount==1 && (param == 1 || param == 0)){
    cfg.send_db = param;
    result = SUCCESS_PARAMS_PARSE;
  }
  switch(cfg.send_db){
    case 0:
      fputs_P(PSTR(" disabled"), &sock_stream);
      break;
    case 1:
      fputs_P(PSTR(" enabled"), &sock_stream);
      cfg.send_db = 1;
      break;
    default:
      ;
  }
  if(result == SUCCESS_PARAMS_PARSE){
    fputs_P(StoreOption, &sock_stream);
  }
  return result;
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
    fputs_P(ResetDBOption, &sock_stream);
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
    fputs_P(ResetDBOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleCookieDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, FormatCookieDB, &(cfg.cookie_db));
  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs(cfg.cookie_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(StoreOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleNameDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, FormatNameDB, &(cfg.name_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs(cfg.name_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(StoreOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleDocDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, FormatDocDB, &(cfg.doc_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs(cfg.doc_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(StoreOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}

int8_t handleFuncDB(){
  int16_t paramsCount=fscanf_P(&sock_stream, FormatFuncDB, &(cfg.func_db));

  if(paramsCount==1){
    fputs_P(New, &sock_stream);
  }
  fputs_P(Colon, &sock_stream);
  fputs(cfg.func_db, &sock_stream);
  if(paramsCount==1){
    fputs_P(StoreOption, &sock_stream);
    return SUCCESS_PARAMS_PARSE;
  }
  return FAILED_PARAMS_PARSE;
}


int8_t handleViewResponseDB(){
  uint8_t serverSockIndex = stream_get_sock()-FIRST_SERVER_SOCK;
  if(serverSockIndex<MAX_SERVER_SOCK_NUM){
    db_response_request[serverSockIndex]= !db_response_request[serverSockIndex];
  }
  if(db_response_request[serverSockIndex] && W5100.readSnSR(DB_CLIENT_SOCK) != SnSR::ESTABLISHED ){
    if(W5100.readSnSR(DB_CLIENT_SOCK) == SnSR::CLOSED){
      fputs_P(PSTR("\nNot connected with database!"), &sock_stream);
    } else {
      fprintf_P(&sock_stream, PSTR("\nConnection with database failed! Status %x"), W5100.readSnSR(DB_CLIENT_SOCK));
    } 
  }
  return NO_PARAMS_PARSE;
}

void printOption(struct cmd cmd){
	fputs_P(PSTR("\n\t"), &sock_stream);
	fputs(cmd.name, &sock_stream);
	if(cmd.param_format!=NULL){
		fputs_P(PSTR(" \t"), &sock_stream);
		fputs_P(cmd.param_format, &sock_stream);
    fputs_P(PSTR("\n\t"), &sock_stream);
	}
	if(cmd.comment!=NULL){
		fputs_P(PSTR(" \t"), &sock_stream);
		fputs_P(cmd.comment, &sock_stream);
	}
}

int8_t handleHelp(){
  int8_t index;
  fputc('\n', &sock_stream);
  fputs_P(Usage, &sock_stream);
  for(index= 0; index<DEFINED_CMD_COUNT - 1; index++){
    printOption(cmds[index]);
  }
  return 1;
}

uint8_t execCMD(uint8_t sock, char * buff, int8_t hasParams){
  int8_t index;
  struct cmd cmd;
  uint8_t handle_state;
#ifdef DEBUG
  printf_P(PSTR("Compare %s\n\r"), buff);
#endif
  for(index= 0; index<DEFINED_CMD_COUNT; index++){
    cmd = cmds[index];
    if(!(strcmp(buff, cmd.name))){
      fputs(cmd.name, &sock_stream);
			handle_state= cmd.handle();
      if(hasParams && handle_state==FAILED_PARAMS_PARSE && cmd.param_format!=NULL){
        fputc('\n', &sock_stream);
        fputs_P(Usage, &sock_stream);
        printOption(cmd);
      }
      fputc('\n', &sock_stream);
      sock_stream_flush();
      return handle_state;
    }
  }
  fputs_P(CmdNotFound, &sock_stream);
  sock_stream_flush();
	return NO_PARAMS_PARSE;
}

uint8_t ui_handleCMD(uint8_t sock){
	uint8_t pointer=0;
	int16_t b;
	int8_t flag_return=0;
	uint8_t cmd_result = NO_PARAMS_PARSE;
	stream_set_sock(sock); 
	while((b=fgetc(&sock_stream)) != EOF){
    // After handling a cmd, rest of characters will be ignored
    if(flag_return){
      continue;
    }
		if(b == ' ' || b == '\n' || b == '\r'){
			flag_return = 1;
			if(pointer>0){
				cmdBuff[pointer++] = '\0';
				cmd_result = execCMD(sock, cmdBuff, b == ' ');
			}
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
			flag_return = 1;
      cmdBuff[pointer++] = '\0';
      cmd_result = execCMD(sock, cmdBuff, 0);
      continue;
		}
    cmdBuff[pointer++] = b;
	}
	if (flag_return && (cmd_result != SUSPEND)){
		fputs_P(PSTR("% "), &sock_stream);
		sock_stream_flush();
	}
	return cmd_result;
}
