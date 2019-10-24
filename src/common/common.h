#ifndef TETHEALLA_COMMON_H
#define TETHEALLA_COMMON_H

#include <stdint.h>
#include "def_structs.h"

#define CLASS_HUMAR 0x00
#define CLASS_HUNEWEARL 0x01
#define CLASS_HUCAST 0x02
#define CLASS_RAMAR 0x03
#define CLASS_RACAST 0x04
#define CLASS_RACASEAL 0x05
#define CLASS_FOMARL 0x06
#define CLASS_FONEWM 0x07
#define CLASS_FONEWEARL 0x08
#define CLASS_HUCASEAL 0x09
#define CLASS_FOMAR 0x0A
#define CLASS_RAMARL 0x0B
#define CLASS_MAX 0x0C

#define SOCKET_ERROR  -1

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

void strupr(char * temp);
int32_t CalculateChecksum(void* data,uint32_t size);
int32_t receive_from_server(int sock, char* packet);
int32_t tcp_accept (int sockfd, struct sockaddr *client_addr, uint32_t *addr_len );
int32_t tcp_sock_connect(char* dest_addr, int32_t port);
int32_t tcp_sock_open(struct in_addr ip, int32_t port);
uint32_t free_connection();
uint8_t hexToByte ( char* hs );
void convertIPString (char* IPData, uint32_t IPLen, int32_t fromConfig );
void debug(char *fmt, ...);
void debug_perror( char * msg );
void display_packet ( uint8_t* buf, int32_t len );
void initialize_connection (BANANA* connect);
void Send19 (uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint16_t ipp, BANANA* client);
void Send1A (const char *mes, BANANA* client);
void SendA0 (BANANA* client);
void SendEE (const char *mes, BANANA* client);
void send_to_server(int sock, char* packet);
void start_encryption(BANANA* connect);
void tcp_listen (int sockfd);
void WriteLog(char *fmt, ...);

#endif
