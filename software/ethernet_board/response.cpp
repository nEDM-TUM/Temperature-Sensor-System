#include "response.h"
#include "socket.h"
#include "configuration.h"

FILE res_stream;
uint8_t currSock = MAX_SERVER_SOCK_NUM;

uint8_t resBuffPointer = 0;
char resBuff[MAX_RESPONSE_LEN];

int res_flush(){
  uint8_t index;
  if(resBuffPointer > 0){
    if (currSock < MAX_SERVER_SOCK_NUM){
      printf("send to sock %d\n\r", currSock);
      send(currSock, (uint8_t *)resBuff, resBuffPointer);
    } else {
      for(index= 0; index<MAX_SERVER_SOCK_NUM; index++){
        if(W5100.readSnSR(index) == SnSR::ESTABLISHED){
          send(index, (uint8_t *)resBuff, resBuffPointer);
        }
      }
    }
  }
  resBuffPointer =0;
}

static int res_putchar(char c, FILE *stream){
  if(resBuffPointer == MAX_RESPONSE_LEN){
    res_flush();
  }
  resBuff[resBuffPointer++] = c;
  return (int)c;
}

static int res_readchar(FILE *stream){
  uint8_t b;  
  if(recv(currSock, &b, 1) >0){
    if(b==' '){
      return EOF;
    }
    if(b==';'){
      return (int)'\n';
    }
    return (int)b;
  }
  return EOF;
}

void res_set_sock(uint8_t sock){
  int8_t flag=1;
  while(W5100.getRXReceivedSize(sock) && recv(sock, &b, 1) >0){
    if(flag){
      fprintf(&res_stream, "State Error");
    }
  }
  res_flush();
  
  currSock = sock;
}


void res_init(){
	fdev_setup_stream(&res_stream, res_putchar, NULL, _FDEV_SETUP_WRITE);
  resBuffPointer = 0;
}
