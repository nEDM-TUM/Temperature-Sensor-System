#include "response.h"
#include "socket.h"

FILE res_stream = FDEV_SETUP_STREAM(res_putchar, NULL, _FDEV_SETUP_WRITE);
uint8_t resBuffPointer = 0;
uint8_t currResSock = MAX_SERVER_SOCK_NUM;

char resBuff[MAX_RESPONSE_LEN];

int res_flush(){
  printf("TEST Flush\n\r");
  if((resBuffPointer > 0) && (currResSock < MAX_SERVER_SOCK_NUM)){
    printf("send to sock %d\n\r", currResSock);
    send(currResSock, (uint8_t *)resBuff, resBuffPointer);
  }
  resBuffPointer =0;
}

static int res_putchar(char c, FILE *stream){
  printf("TEST put char\n\r");
  if(resBuffPointer == MAX_RESPONSE_LEN){
    res_flush();
  }
  resBuff[resBuffPointer++] = c;
}

void res_set_sock(uint8_t sock){
  res_flush();
  currResSock = sock;
}


void response_init(){
  resBuffPointer = 0;
}
