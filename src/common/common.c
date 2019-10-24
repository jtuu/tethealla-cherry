#include "common.h"

void strupr(char * temp) {

  // Convert to upper case
  char *s = temp;
  while (*s) {
    *s = toupper(*s);
    s++;
  }

}

int32_t CalculateChecksum(void* data,uint32_t size)
{
    int32_t offset,y,cs = 0xFFFFFFFF;
    for (offset = 0; offset < (long)size; offset++)
    {
        cs ^= *(uint8_t*)((long)data + offset);
        for (y = 0; y < 8; y++)
        {
            if (!(cs & 1)) cs = (cs >> 1) & 0x7FFFFFFF;
            else cs = ((cs >> 1) & 0x7FFFFFFF) ^ 0xEDB88320;
        }
    }
    return (cs ^ 0xFFFFFFFF);
}

void send_to_server(int sock, char* packet)
{
 int32_t pktlen;

 pktlen = strlen (packet);

  if (send(sock, packet, pktlen, 0) != pktlen)
  {
    printf ("send_to_server(): failure");
    exit(1);
  }

}

int32_t receive_from_server(int sock, char* packet)
{
 int32_t pktlen;

  if ((pktlen = recv(sock, packet, TCP_BUFFER_SIZE - 1, 0)) <= 0)
  {
    printf ("receive_from_server(): failure");
    exit(1);
  }
  packet[pktlen] = 0;
  return pktlen;
}

void tcp_listen (int sockfd)
{
  if (listen(sockfd, 10) < 0)
  {
    debug_perror ("Could not listen for connection");
    exit(1);
  }
}

int32_t tcp_accept (int sockfd, struct sockaddr *client_addr, uint32_t *addr_len )
{
  int32_t fd;

  if ((fd = accept (sockfd, client_addr, addr_len)) < 0)
    debug_perror ("Could not accept connection");

  return (fd);
}

int32_t tcp_sock_connect(char* dest_addr, int32_t port)
{
  int32_t fd;
  struct sockaddr_in sa;

  /* Clear it out */
  memset((void *)&sa, 0, sizeof(sa));

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  /* Error */
  if( fd < 0 )
    debug_perror("Could not create socket");
  else
  {

    memset (&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr (dest_addr);
    sa.sin_port = htons((uint16_t) port);

    if (connect(fd, (struct sockaddr*) &sa, sizeof(sa)) < 0)
      debug_perror("Could not make TCP connection");
    else
      debug ("tcp_sock_connect %s:%u", inet_ntoa (sa.sin_addr), sa.sin_port );
  }
  return(fd);
}

/*****************************************************************************/
int32_t tcp_sock_open(struct in_addr ip, int32_t port)
{
  int32_t fd, turn_on_option_flag = 1;

  struct sockaddr_in sa;

  /* Clear it out */
  memset((void *)&sa, 0, sizeof(sa));

  fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  /* Error */
  if( fd < 0 ){
    debug_perror("Could not create socket");
    exit(1);
  }

  sa.sin_family = AF_INET;
  memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
  sa.sin_port = htons((uint16_t) port);

  /* Reuse port */

  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &turn_on_option_flag, sizeof(turn_on_option_flag));

  /* bind() the socket to the interface */
  if (bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0){
    debug_perror("Could not bind to port");
    exit(1);
  }

  return(fd);
}

/*****************************************************************************
* same as debug_perror but writes to debug output.
*
*****************************************************************************/
void debug_perror( char * msg ) {
  debug( "%s : %s\n" , msg , strerror(errno) );
}
/*****************************************************************************/
void debug(char *fmt, ...)
{
#define MAX_MESG_LEN 1024

  va_list args;
  char text[ MAX_MESG_LEN ];

  va_start (args, fmt);
  strcpy (text + vsprintf( text,fmt,args), "\r\n");
  va_end (args);

  fprintf( stderr, "%s", text);
}


uint32_t free_connection()
{
  uint32_t fc;
  BANANA* wc;

  for (fc=0;fc<serverMaxConnections;fc++)
  {
    wc = connections[fc];
    if (wc->plySockfd<0)
      return fc;
  }
  return 0xFFFF;
}


void initialize_connection (BANANA* connect)
{
  uint32_t ch, ch2;

  // Free backup character memory

  if (connect->character_backup)
  {
    if (connect->mode)
      memcpy (&connect->character, connect->character_backup, sizeof (connect->character));
    free (connect->character_backup);
    connect->character_backup = NULL;
  }

  if (connect->guildcard)
  {
    removeClientFromLobby (connect);

    if ((connect->block) && (connect->block <= serverBlocks))
      blocks[connect->block - 1]->count--;

    if (connect->gotchardata == 1)
    {
      connect->character.playTime += (unsigned) servertime - connect->connected;
      ShipSend04 (0x02, connect, logon);
    }
  }

  if (connect->plySockfd >= 0)
  {
    ch2 = 0;
    for (ch=0;ch<serverNumConnections;ch++)
    {
      if (serverConnectionList[ch] != connect->connection_index)
        serverConnectionList[ch2++] = serverConnectionList[ch];
    }
    serverNumConnections = ch2;
    close (connect->plySockfd);
  }

  if (logon_ready)
  {
    printf ("Player Count: %u\n", serverNumConnections);
    ShipSend0E (logon);
  }

  memset (connect, 0, sizeof (BANANA) );
  connect->plySockfd = -1;
  connect->block = -1;
  connect->lastTick = 0xFFFFFFFF;
  connect->slotnum = -1;
  connect->sending_quest = -1;
}

uint8_t hexToByte ( char* hs )
{
  uint32_t b;

  if ( hs[0] < 58 ) b = (hs[0] - 48); else b = (hs[0] - 55);
  b *= 16;
  if ( hs[1] < 58 ) b += (hs[1] - 48); else b += (hs[1] - 55);
  return (uint8_t) b;
}


void convertIPString (char* IPData, uint32_t IPLen, int32_t fromConfig, uint8_t* IPStore )
{
  uint32_t p,p2,p3;
  char convert_buffer[5];

  p2 = 0;
  p3 = 0;
  for (p=0;p<IPLen;p++)
  {
    if ((IPData[p] > 0x20) && (IPData[p] != 46))
      convert_buffer[p3++] = IPData[p]; else
    {
      convert_buffer[p3] = 0;
      if (IPData[p] == 46) // .
      {
        IPStore[p2] = atoi (&convert_buffer[0]);
        p2++;
        p3 = 0;
        if (p2>3)
        {
          if (fromConfig)
            printf ("ship.ini is corrupted. (Failed to read IP information from file!)\n"); else
            printf ("Failed to determine IP address.\n");
          exit (1);
        }
      }
      else
      {
        IPStore[p2] = atoi (&convert_buffer[0]);
        if (p2 != 3)
        {
          if (fromConfig)
            printf ("ship.ini is corrupted. (Failed to read IP information from file!)\n"); else
            printf ("Failed to determine IP address.\n");
          exit (1);
        }
        break;
      }
    }
  }
}

void display_packet ( uint8_t* buf, int32_t len )
{
  packet_to_text ( buf, len );
  printf ("%s\n\n", &dp[0]);
}
