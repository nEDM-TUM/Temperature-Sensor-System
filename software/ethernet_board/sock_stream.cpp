#include "sock_stream.h"
#include "socket.h"
#include "networking.h"

FILE sock_stream;
uint8_t currSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;

uint8_t sockBuffPointer = 0;
char sockBuff[MAX_RESPONSE_LEN];

inline uint8_t sendBuff(uint8_t sock){
#ifdef DEBUG
    printf_P(PSTR("Before sending, frei %u\n\r"), W5100.getTXFreeSize(sock));
#endif
  if(W5100.readSnSR(sock) == SnSR::ESTABLISHED){
    // XXX By established connection, it takes very long time to find out the connection is actually down, if the other device does not send response any more. 
    // Then the send buffer will be filled until no space any more. 
    // The arduino library socket will block the whole process until the connection is closed or buffer becomes free. 
    // In order to avoid blocking the process, the freesize will be checked before send messages. 
    // Wenn no place any more, the coming message will be droped!
  //if(W5100.readSnSR(sock) == SnSR::ESTABLISHED && W5100.getTXFreeSize(sock)>MAX_RESPONSE_LEN){
    send(sock, (uint8_t *)sockBuff, sockBuffPointer);
    return 1;
  }
  return 0;
}

int sock_stream_flush(){
  uint8_t index;
  if(sockBuffPointer > 0){
#ifdef DEBUG
    printf_P(PSTR("Flush...\n\r"));
#endif 
    if(currSock < MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK){
      sendBuff(currSock);
    } else {
      // broadcast for server socks
      for(index=FIRST_SERVER_SOCK; index<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; index++){
        sendBuff(index);
      }
    }
    sockBuffPointer =0;
  }
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
    return (int)b;
  }
  return EOF;
}

uint8_t stream_get_sock(){
	return currSock;
}

void stream_set_sock(uint8_t sock){
  currSock = sock;
}


void sock_stream_init(){
	fdev_setup_stream(&sock_stream, sock_putchar, sock_readchar, _FDEV_SETUP_RW);
  sockBuffPointer = 0;
}
