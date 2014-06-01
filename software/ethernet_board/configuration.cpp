#include "configuration.h"

#include <avr/io.h>
#include "usart.h"
#include <Ethernet.h>
#include "w5100.h"
#include "socket.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
uint8_t ip[] = {10,0,1, 100};
uint8_t gateway[] = {10,0,1, 1 };
uint8_t subnet[] = {255, 255, 0, 0};
uint16_t port = 8888;
uint8_t clientSock = 3;
uint8_t serverSock[] = {0,0,0};
// telnet defaults to port 23
boolean alreadyConnected = false; // whether or not the client was connected previously

  Server server = Server(23);
void setupServer() {
  printf("Set up server\n\r");
  //Init and config ethernet device w5100
  W5100.init();
  W5100.setMACAddress(mac);
  W5100.setIPAddress(ip);
  W5100.setGatewayIp(gateway);
  W5100.setSubnetMask(subnet);
  // Create the first server socket
  socket(0, SnMR::TCP, port, 0);
  serverSock[0] = 1;
  if(listen(0)){
    	printf("Listened\n\r");
  } else {
    printf("not Linstend \n\r");
  }
}


void setupServerLib(){
 Ethernet.begin(mac, ip, gateway, subnet);

  // start listening for clients
  server.begin();
  printf("Listened\n\r");
}
void loop() {
  // wait for a new client:

}
void startServer(){
;
}
