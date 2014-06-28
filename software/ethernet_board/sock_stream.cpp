#include "sock_stream.h"
#include "socket.h"
#include "networking.h"

FILE sock_stream;
uint8_t currSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;

uint8_t sockBuffPointer = 0;
char sockBuff[MAX_RESPONSE_LEN];

int sock_stream_flush(){
  uint8_t index;
  if(sockBuffPointer > 0){
    if (currSock < MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK && W5100.readSnSR(currSock) == SnSR::ESTABLISHED){
      send(currSock, (uint8_t *)sockBuff, sockBuffPointer);
    } else {
      // broadcast for server socks
      for(index=FIRST_SERVER_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
        if(W5100.readSnSR(index) == SnSR::ESTABLISHED){
          send(index, (uint8_t *)sockBuff, sockBuffPointer);
        }
      }
    }
  }
  sockBuffPointer =0;
}

static int sock_putchar(char c, FILE *stream){
  if(sockBuffPointer == MAX_RESPONSE_LEN){
    sock_stream_flush();
  }
  sockBuff[sockBuffPointer++] = c;
  return (int)c;
}

static int sock_readchar(FILE *stream){
  uint8_t b;  
  if(recv(currSock, &b, 1) >0){
    printf("%c %u\n\r",b, b);
    return (int)b;
  }
  return EOF;
}

uint8_t stream_get_sock(){
	return currSock;
}

void stream_set_sock(uint8_t sock){
  uint8_t b;  
  if( sock== currSock ){
    return;
  }
  currSock = sock;
}


void sock_stream_init(){
	fdev_setup_stream(&sock_stream, sock_putchar, sock_readchar, _FDEV_SETUP_RW);
  sockBuffPointer = 0;
}
