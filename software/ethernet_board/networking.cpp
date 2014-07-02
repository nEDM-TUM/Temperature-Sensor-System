#include "networking.h"

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usart.h"
#include "config.h"
#include "socket.h"
#include "sock_stream.h"
#include <Ethernet.h>
#include "w5100.h"
// set debug mode
// #define DEBUG

uint8_t listeningSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;
uint8_t closedSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;

uint8_t data_request[MAX_SERVER_SOCK_NUM] = {0};
uint8_t db_response_request[MAX_SERVER_SOCK_NUM] = {0};
uint32_t measure_interval = 1000;

char receiveBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
uint8_t receiveBuffPointer[MAX_SERVER_SOCK_NUM] = {0}; // Point to a byte, which will be written

void send_result(struct dummy_packet * packets);
void (*twi_access_fun)();
void serve();

uint8_t ui_state = UI_READY;

const char UintDot_4Colon[] PROGMEM = "%u.%u.%u.%u:%u";

void toSubnetMask(uint8_t subnet, uint8_t* addr){
  int8_t indexByte = 0;
  int8_t indexBit = 0;
  while(subnet >= 8 && indexByte<4){
    addr[indexByte] = 255;
    indexByte++;
    subnet -= 8;
  }
  if(indexByte>=4){
    return;
  }
  while(subnet != 0 ){
    addr[indexByte] = (addr[indexByte]<<1)+1;
    indexBit++;
    subnet--;
    if(indexBit>=8){
      return;
    }
  }
  return;
}


uint16_t getAvailableSrcPort(uint16_t srcPort){
  if(srcPort == cfg.port){
    srcPort ++;
  }
  if(srcPort < 1024){
    return 1024;
  }
  return srcPort;
}

void try_connect_db(uint16_t srcPort){
  close(DB_CLIENT_SOCK);
  _delay_ms(100);
  srcPort = getAvailableSrcPort(srcPort);
  socket(DB_CLIENT_SOCK, SnMR::TCP, srcPort, 0);
  connect(DB_CLIENT_SOCK, cfg.ip_db, cfg.port_db);
}

int8_t connect_db(uint16_t srcPort){
  int8_t syn_flag = 0;
  try_connect_db(srcPort);
  // FIXME test
  while(W5100.readSnSR(DB_CLIENT_SOCK) != SnSR::ESTABLISHED) {
    _delay_ms(100);
    printf("Status %x\n\r", W5100.readSnSR(DB_CLIENT_SOCK));
    if (W5100.readSnSR(DB_CLIENT_SOCK) == SnSR::SYNSENT) {
      if(syn_flag){
        syn_flag = 0;
        srcPort++;
        try_connect_db(srcPort);
      }else{
        syn_flag = 1;
      }
    }else if (W5100.readSnSR(DB_CLIENT_SOCK) == SnSR::CLOSED) {
      printf_P(PSTR("DB closed!!!!\n\r"));
      return 0;  
    }
  }
  return 1;
}

void net_beginService() {
  config_read(&cfg);
  uint8_t sn[4]={0};
  //Init and config ethernet device w5100
  W5100.init();
  W5100.setMACAddress(cfg.mac);
  W5100.setIPAddress(cfg.ip);
  W5100.setGatewayIp(cfg.gw);
  toSubnetMask(cfg.subnet, sn);
  W5100.setSubnetMask(sn);
	printf_P(PSTR("ip: %u.%u.%u.%u/%u:%u\n\r"), cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.subnet, cfg.port);
  printf_P(PSTR("subnet %u.%u.%u.%u\n\r"), sn[0], sn[1], sn[2], sn[3]);
	printf_P(PSTR("mac: %u.%u.%u.%u/%u\n\r"), cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.subnet);
	printf_P(PSTR("gw: %u.%u.%u.%u/%u\n\r"), cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.subnet);
	printf_P(PSTR("db: %u.%u.%u.%u:%u/%s/%s/%s\n\r"), cfg.ip_db[0], cfg.ip_db[1], cfg.ip_db[2], cfg.ip_db[3], cfg.port_db, cfg.name_db, cfg.doc_db, cfg.func_db);
  // Create the first server socket
  socket(FIRST_SERVER_SOCK, SnMR::TCP, cfg.port, 0);
  while(!listen(FIRST_SERVER_SOCK)){
    // wait a second and try again
    _delay_ms(1000);
  }
  // Create client to db
  // Port of the client can be arbitary, but it should be different then the port of ui server
  connect_db(cfg.port+1);
}

void net_sendHeadToDB(uint16_t len){
  // 1st line: POST /{name_db}/_design/{doc_db}/_update/{func_db} HTTP/1.1
  fputs_P(PSTR("POST /"), &sock_stream);
  // TODO to compute content length??? fix??
  fputs(cfg.name_db, &sock_stream);
  fputs_P(PSTR("/_design/"), &sock_stream);
  fputs(cfg.doc_db, &sock_stream);
  fputs_P(PSTR("/_update/"), &sock_stream);
  fputs(cfg.func_db, &sock_stream);
  fputs_P(PSTR(" HTTP/1.1\n"), &sock_stream);
  // 2nd line: Host: {ip}:{port}
  fputs_P(PSTR("Host: "), &sock_stream);
  fprintf_P(&sock_stream, UintDot_4Colon, cfg.ip_db[0], cfg.ip_db[1], cfg.ip_db[2], cfg.ip_db[3], cfg.port_db);
  fputs_P(PSTR("\n"), &sock_stream);
  // 3rd line: Cookie: AuthSession="{cookie_db}"
  // fputs_P(PSTR("Cookie: AuthenSeesion=\""), &sock_stream);
  // fputs(cfg.cookie_db, &sock_stream);
  //fputs_P(PSTR("\"\n"), &sock_stream);
  // 4th line: X-CouchDB-WWW-Authenticate: Cookie
  //fputs_P(PSTR("X-CouchDB-WWW-Authenticate: Cookie\n"), &sock_stream);
  // 5rd line: Content-type: application/json
  fputs_P(PSTR("Content-type: application/json\n"), &sock_stream);
  // 6th line: Content-Length: {len}
  fprintf_P(&sock_stream, PSTR("Content-Length: %u\n"), len);
  if(len>0){
    // 7th line: \newline
    fputs_P(PSTR("\n"), &sock_stream);
    // 8th line: {data}
  }
}

// TODO
void net_sendTestToDB(){
  stream_set_sock(DB_CLIENT_SOCK);
  net_sendHeadToDB(17);
  fputs_P(PSTR("{\"value\": \"test\"}"), &sock_stream);
  sock_stream_flush();
}

void send_response(uint8_t toSock){
	int16_t b;
  stream_set_sock(toSock); 
	while(true){
    stream_set_sock(DB_CLIENT_SOCK); 
    if((b=fgetc(&sock_stream)) == EOF){
      stream_set_sock(toSock); 
      sock_stream_flush();
      // TODO
      putc('\n', stdout);
      putc('\r', stdout);
      return;
    }
    if(toSock == DB_CLIENT_SOCK){
      // TODO make away
      putc(b, stdout);
      continue;
    }
    stream_set_sock(toSock); 
    fputc(b, &sock_stream);
  }
  stream_set_sock(toSock); 
  sock_stream_flush();
}

void net_sendResultToDB(struct dummy_packet *packets, uint8_t board_addr){
  int8_t sensor_index;
  int8_t comma_flag = 0;
  int16_t value;
  uint16_t len=0;
  uint8_t currSock = stream_get_sock();
  if( W5100.readSnSR(DB_CLIENT_SOCK) != SnSR::ESTABLISHED ){
    if(!connect_db(cfg.port+1)){
      return;
    }
  }
  for (sensor_index=0;sensor_index<8;sensor_index++){
    if(packets[sensor_index].header.error && packets[sensor_index].header.connected){
      // XXX IF ERROR HANDLING
      //len += 12; 
      continue;
    }
    if(packets[sensor_index].header.connected){
      switch(packets[sensor_index].header.type){
        case PACKET_TYPE_TSIC:
          len += JSON_TEMP_LEN;
          // length of "," or "}" at the end
          len++;
          break;
        case PACKET_TYPE_HYT:
          len += JSON_TEMP_LEN;
          len++;
          len += JSON_HUM_LEN;
          len++;
          break;
        default:
          break;
      }
    }
  }
  if(len==0){
    return;
  }
  len+=JSON_PREFIX_LEN;
  // length of "}" at the end
  len++;
  stream_set_sock(DB_CLIENT_SOCK);
  net_sendHeadToDB(len);
  printf_P(PSTR("Send Head \n\r"));

  /* convert packet data to json format
   * {"type":"value","data",{"bdddsdTEMP":"ddd.dd","bdddsdHUM":"ddd.dd"}}
   */
  fputs_P(PSTR(JSON_PREFIX), &sock_stream);
  comma_flag = 0;
  for (sensor_index=0;sensor_index<8;sensor_index++){
    if(packets[sensor_index].header.error && packets[sensor_index].header.connected){
      continue;
    }
    if(packets[sensor_index].header.connected){
      switch(packets[sensor_index].header.type){
        case PACKET_TYPE_TSIC:
          if(comma_flag){
            fputc(',', &sock_stream);
          }else{
            comma_flag = 1; 
          }
          value =  ((struct tsic_packet *)(packets))[sensor_index].temperature;
          fprintf_P(&sock_stream, PSTR(JSON_TEMP), JSON_OUTPUT);
          printf_P(PSTR("Send temperature \n\r"));
          break;
        case PACKET_TYPE_HYT:
          if(comma_flag){
            fputc(',', &sock_stream);
          }else{
            comma_flag = 1; 
          }
          value = ((struct hyt_packet *)(packets))[sensor_index].temperature;
          fprintf_P(&sock_stream, PSTR(JSON_TEMP), JSON_OUTPUT);
          value = ((struct hyt_packet *)(packets) )[sensor_index].humidity;
          fputc(',', &sock_stream);
          fprintf_P(&sock_stream, PSTR(JSON_HUM), JSON_OUTPUT); 
          break;
        default:
          break;
      }
    }
  }
  fputc('}', &sock_stream);
  fputc('}', &sock_stream);
  printf_P(PSTR("Send finished \n\r"));
  sock_stream_flush();
  // TODO send response
  send_response(DB_CLIENT_SOCK);
  stream_set_sock(currSock);
}

void net_dataAvailable(struct dummy_packet * received, uint8_t src_addr){
	uint8_t i;
  if(cfg.send_db){
    net_sendResultToDB(received, src_addr);
  }
	for(i=FIRST_SERVER_SOCK; i< MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; i++){
		// TODO: check if socket is still connected.
		if (data_request[i-FIRST_SERVER_SOCK]){
			stream_set_sock(i);
			fprintf(&sock_stream, "%u :: ", src_addr);
			send_result(received);
		}
	}
	sock_stream_flush();
}

void serve(){
  uint8_t i;
  uint8_t snSR;
  uint8_t cmd_state;
  closedSock = MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK;
  listeningSock = MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK;
  for(i=FIRST_SERVER_SOCK; i<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; i++){
    snSR = W5100.readSnSR(i);
#ifdef DEBUG
    printf("%u. Status: %x\n\r",i, snSR);
#endif
    switch (snSR){
      case SnSR::CLOSED:
        closedSock = i;
        break;
      case SnSR::INIT:
#ifdef DEBUG
        printf("Init Sock: %u\n\r", i);
#endif
        break;
      case SnSR::LISTEN:
        listeningSock = i;
        break;
      case SnSR::ESTABLISHED:
        cmd_state = ui_handleCMD(i);
				if(cmd_state == SUSPEND){
					// handleCMD requested, to not accept new commands,
					// so we return here:
					return;
				}
        break;
      case SnSR::CLOSING:
        // TODO if readCMD??
        printf("Closing Sock: %u\n\r", i);
        break;
      case SnSR::TIME_WAIT:
        printf("Time wait Sock: %u\n\r", i);
        break;
      case SnSR::CLOSE_WAIT:
        // TODO implement see Arduino Ethernet library
        printf("Close wait Sock: %u\n\r", i);
        break;
      default:
        break;
    }
  }
#ifdef DEBUG
  printf("Listening socket %d, closed socket %d\n\r",listeningSock, closedSock);
#endif
  if(listeningSock == MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK && closedSock < MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK){
    socket(closedSock, SnMR::TCP, cfg.port, 0);    
    listen(closedSock);
  }
}

void net_loop(){
	switch(ui_state){
		case UI_READY:
			serve();
			break;
		case UI_TWILOCK:
			// we do not process new socket inputs, as we have to wait for the twi bus to become ready:
			if(twi_try_lock_bus()){
				// we aquired the bus
				// now we have to do our access as fast as possible, as others might wait for bus access:
				// call command handler, who waits for access to the bus:
				twi_access_fun();
				
				fputs_P(PSTR("% "), &sock_stream);
				sock_stream_flush();
				// we again accept commands
				ui_state = UI_READY;
				twi_free_bus();
				int16_t b;
				while((b=fgetc(&sock_stream)) != EOF){
					if(b == '\n' || b == ';'){
						break;
					}
				}
					

				// handle other already existing commands in the buffer
				//ui_handleCMD(stream_get_sock());
			}
			break;
	}
}

void net_setupServer() {
  sock_stream_init();
#ifdef DEBUG
  printf("Set up server\n\r");
#endif
  net_beginService();
}


