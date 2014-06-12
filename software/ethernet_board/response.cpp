#include "response.h"
#include "socket.h"

FILE res_stream;
uint8_t resBuff2Pointer = 0;
uint8_t currResSock = MAX_SERVER_SOCK_NUM;

char resBuff2[MAX_RESPONSE_LEN];

int res_flush(){
  printf("TEST Flush\n\r");
  if((resBuff2Pointer > 0) && (currResSock < MAX_SERVER_SOCK_NUM)){
    printf("send to sock %d\n\r", currResSock);
    send(currResSock, (uint8_t *)resBuff2, resBuff2Pointer);
  }
  resBuff2Pointer =0;
}

static int res_putchar(char c, FILE *stream){
  printf("TEST put char\n\r");
  if(resBuff2Pointer == MAX_RESPONSE_LEN){
    res_flush();
  }
  resBuff2[resBuff2Pointer++] = c;
}

void res_set_sock(uint8_t sock){
  res_flush();
  currResSock = sock;
}


void res_init(){
	fdev_setup_stream(&res_stream, res_putchar, NULL, _FDEV_SETUP_WRITE);
  resBuff2Pointer = 0;
}
