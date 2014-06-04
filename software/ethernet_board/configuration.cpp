#include "configuration.h"

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include <Ethernet.h>
#include "w5100.h"
#include "socket.h"

// set debug mode
// #define DEBUG
//
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B};
uint8_t ip[] = {10,0,1, 100};
uint8_t gateway[] = {10,0,1, 1 };
uint8_t subnet = 16;
uint8_t subnetM[] = {255, 255, 0, 0};
uint16_t port = 8888;
uint8_t ip_db[] = {10,0,1, 100};
uint16_t port_db = 8888;
uint8_t clientSock = MAX_SERVER_SOCK_NUM;
uint8_t serverSock[MAX_SERVER_SOCK_NUM];
uint8_t listeningSock = MAX_SERVER_SOCK_NUM;
uint8_t closedSock = MAX_SERVER_SOCK_NUM;

char receiveBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
char resBuff[MAX_RESPONSE_LEN];
uint8_t writeBuffPointer[MAX_SERVER_SOCK_NUM] = {0}; // Point to a byte, which will be written

int8_t convertParamToBytes(char * buff, int8_t len, uint8_t * params){
  int8_t index;
  int8_t paramIndex =0;
  int8_t paramCounter =0;
  char paramBuff[MAX_PARAM_LEN];
  for(index=0; index<len; index++){
    printf("Param buff  %c, %d\n\r", buff[index], buff[index]);
    if(paramIndex<MAX_PARAM_LEN && buff[index]>=48 && buff[index]<=57){
      paramBuff[paramIndex] = buff[index];
      paramIndex++;
    printf("oben!!! Param index  %d\n\r", paramIndex);
    }else{
      if(paramIndex>0){
    printf("Param index  %d\n\r", paramIndex);
        paramBuff[paramIndex] = '\0';
        params[paramCounter] = atoi(paramBuff);
        paramCounter++;
        paramIndex = 0;
      }
    }
  }
  if(paramIndex>0){
    paramBuff[paramIndex] = '\0';
    params[paramCounter] = atoi(paramBuff);
    paramCounter++;
  }
  printf("paramCounter = %d\n\r", paramCounter);
  return paramCounter;
}

void execCMD(uint8_t sock, char * buff, int8_t len){
  int16_t resLen=0;
  uint8_t params[MAX_PARAM_COUNT];
  int8_t paramCounter =0;
  int8_t index;
  printf("Compare  %s\n\r", buff);
  // TODO lengh include \0 ???
  if(strncmp(buff, "ip", 2) == 0){
    if(len>2){
      paramCounter = convertParamToBytes(buff+2, len-2, params);
      if(paramCounter==5){
        for(index=0; index<4; index++){
          ip[index] = params[index];
        }
        subnet = params[4]; 
        resLen = sprintf(resBuff, "ip will be set to %d.%d.%d.%d/%d, if you input 'update config'\n", ip[0], ip[1], ip[2], ip[3], subnet);
      }else{
        resLen = sprintf(resBuff, "ip is %d.%d.%d.%d/%d (To update the ip address: ip addr/subnet like the output format!)\n", ip[0], ip[1], ip[2], ip[3], subnet);
      }
    }else{
      resLen = sprintf(resBuff, "ip is %d.%d.%d.%d/%d\n", ip[0], ip[1], ip[2], ip[3], subnet);
    }
  } else
  if (strncmp(buff, "gateway", 7) == 0){
    if(len>7){
      paramCounter = convertParamToBytes(buff+7, len-7, params);
      if(paramCounter==4){
        for(index=0; index<4; index++){
          gateway[index] = params[index];
        }
        resLen = sprintf(resBuff, "gateway will be set to %d.%d.%d.%d, if you input 'update config'\n", gateway[0], gateway[1], gateway[2], gateway[3]);
      }else{
        resLen = sprintf(resBuff, "gateway is %d.%d.%d.%d (To update the gateway address: gateway addr like the output format!)\n", gateway[0], gateway[1], gateway[2], gateway[3]);
      }
    }else{
      resLen = sprintf(resBuff, "gateway is %d.%d.%d.%d\n", gateway[0], gateway[1], gateway[2], gateway[3]);
    }
  } else
  if( strncmp(buff, "mac", 3) == 0){
  printf("TEST\n\r");
    if(len>3){
      paramCounter = convertParamToBytes(buff+3, len-3, params);
      if(paramCounter==6){
        for(index=0; index<6; index++){
          mac[index] = params[index];
        }
        resLen = sprintf(resBuff, "mac will be set to %d:%d:%d:%d:%d:%d, if you input 'update config'\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("TEST %d\n\r", resLen);
      }else{
        resLen = sprintf(resBuff, "mac is %d:%d:%d:%d:%d:%d (To update the mac address ip addr/subnet like the output format!)\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("TEST %d\n\r", resLen);
      }
    }else{
      resLen = sprintf(resBuff, "mac is %d:%d:%d:%d:%d:%d\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("TEST %d\n\r", resLen);
    }
  //} else
  //if(strncmp(buff, "port", 4) == 0){
  //  if(len>4){
  //    paramCounter = convertParamToBytes(buff+4, len-4, params);
  //    if(paramCounter==1){
  //      port = params[0];
  //      resLen = sprintf(resBuff, "port will be set to %d, if you input 'update config'\n", port);
  //    }else{
  //      resLen = sprintf(resBuff, "port is %d (To update the port: port number like the output format!)\n", port);
  //    }
  //  }else{
  //    resLen = sprintf(resBuff, "port is %d\n", port);
  //  }
  //} else
  //if(strncmp(buff, "update config", 13) == 1){
  //  resLen = sprintf(resBuff, "The config will be updated, the future login is %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], port);
  //  send(sock, (uint8_t *)resBuff, resLen);
  //  // TODO update config!
  //  return;

  //} else
  //if(strncmp(buff, "ip-db", 5) == 0){
  //  if(len>5){
  //    paramCounter = convertParamToBytes(buff+5, len-5, params);
  //    if(paramCounter==4){
  //      for(index=0; index<4; index++){
  //        ip_db[index] = params[index];
  //      }
  //      // TODO update db ip
  //      resLen = sprintf(resBuff, "ip of the datebase is set to %d.%d.%d.%d now!\n", ip_db[0], ip_db[1], ip_db[2], ip_db[3]);
  //    }else{
  //      resLen = sprintf(resBuff, "ip of the datebase is %d.%d.%d.%d (To update the ip address: ip-db addr like the output format!)\n", ip_db[0], ip_db[1], ip_db[2], ip_db[3]);
  //    }
  //  }else{
  //    resLen = sprintf(resBuff, "ip of the database is %d.%d.%d.%d\n", ip_db[0], ip_db[1], ip_db[2], ip_db[3]);
  //  }
  //} else
  //if(strncmp(buff, "port-db", 7) == 0){
  //  if(len>7){
  //    paramCounter = convertParamToBytes(buff+7, len-7, params);
  //    if(paramCounter==1){
  //      port_db = params[0];
  //      resLen = sprintf(resBuff, "port of the database is set to %d now!\n", port_db);
  //    }else{
  //      resLen = sprintf(resBuff, "port of the databse is %d (To update the port: port-db number like the output format!)\n", port_db);
  //    }
  //  }else{
  //    resLen = sprintf(resBuff, "port of the database is %d\n", port_db);
  //  }
  } else {
    // TODO usage:
    resLen = sprintf(resBuff, "Usage: TODO\n");
  }
  send(sock, (uint8_t *)resBuff, resLen);
 }

void handleCMD(uint8_t sock){
  uint8_t b;
  while(recv(sock, &b, 1) >0){
    if(b=='\n' || b==';'){
      receiveBuff[sock][writeBuffPointer[sock]] = '\0';
      execCMD(sock, receiveBuff[sock], writeBuffPointer[sock]);
      writeBuffPointer[sock] = 0;
    }else{
      // XXX The first char should be alphabet in low case 
      if(writeBuffPointer[sock] == 0 && (b<97 || b>122){
        continue;
      }
      receiveBuff[sock][writeBuffPointer[sock]] = b;
      writeBuffPointer[sock]++;
      writeBuffPointer[sock] %= MAX_CMD_LEN;
    }
#ifdef DEBUG
  printf("Receive lenk: %d\n\r", ret);
#endif
  }
}

void serve(){
  uint8_t i;
  for(i=0; i<MAX_SERVER_SOCK_NUM; i++){
    serverSock[i] = W5100.readSnSR(i);
#ifdef DEBUG
    printf("%u. Status: %x\n\r",i, serverSock[i]);
#endif
    switch (serverSock[i]){
      case SnSR::CLOSED:
        closedSock = i;
        break;
      case SnSR::INIT:
        printf("Init Sock: %u\n\r", i);
        break;
      case SnSR::LISTEN:
        listeningSock = i;
        break;
      case SnSR::ESTABLISHED:
        handleCMD(i);
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
      case SnSR::IPRAW:
        printf("IP RAW Sock: %u\n\r", i);
        break;
      default:
        printf("Sock %u Status: %x\n\r", i, serverSock[i]);
    }
  }
  // TODO open more sockets
}

int8_t toSubnetMask(uint8_t subnet, uint8_t* addr){
  int8_t indexByte = 0;
  int8_t indexBit = 0;
  printf("Subnet Mask: %d.%d.%d.%d\n\r", subnetM[0], subnetM[1], subnetM[2], subnetM[3]);
  while((subnet >> 3) >0 ){
  printf("Subnet: %d %d\n\r", subnet, subnet-8<subnet);
    addr[indexByte] = 255;
    indexByte++;
    subnet -= 8;
  }
  if(indexByte>=4){
    return 0;
  }
  while(subnet != 0 ){
    addr[indexByte] |= 1 << indexBit;
    indexBit ++;
    subnet--;
    if(indexBit>=8){
      return 0;
    }
  }
  printf("Subnet Mask: %d.%d.%d.%d\n\r", subnetM[0], subnetM[1], subnetM[2], subnetM[3]);
  return 1;
}

void setupServer() {
  printf("Set up server\n\r");
  //Init and config ethernet device w5100
  toSubnetMask(subnet, subnetM);
  W5100.init();
  W5100.setMACAddress(mac);
  W5100.setIPAddress(ip);
  W5100.setGatewayIp(gateway);
  W5100.setSubnetMask(subnetM);
  // Create the first server socket
  socket(0, SnMR::TCP, port, 0);
  serverSock[0] = W5100.readSnSR(0);
  if(listen(0)){
    	printf("Listened\n\r");
  } else {
    printf("not Linstend \n\r");
  }
  printf("0. Status: %x\n\r", serverSock[0]);
  while(1){
    serve();
  }
}


