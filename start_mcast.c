/* This program is used to invoke the transmissions of the */
/* mcast protocol.  It sends a packet to the multicast ip  */
/* that intiates the packet sending.  */
#include "net_include.h"

void main(){
  char  mess_buf[1];
  mess_buf[0] = 1;
  int ss;
  struct ip_mreq     mreq;
  struct sockaddr_in send_addr;
  int mcast_addr;
  mcast_addr = 225 << 24 | 1 << 16 | 2 << 8 | 108;
  mreq.imr_multiaddr.s_addr = htonl( mcast_addr );
  mreq.imr_interface.s_addr = htonl( INADDR_ANY );
  send_addr.sin_family = AF_INET;

  send_addr.sin_addr.s_addr = htonl(mcast_addr);  /* mcast address */
  send_addr.sin_port = htons(PORT);
  ss = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for sending */
  if (ss < 0) {
      perror("Mcast: socket");
      exit(1);
  }
  sendto( ss, mess_buf, sizeof(mess_buf), 0,
          (struct sockaddr *)&send_addr, sizeof(send_addr));
          printf("Started processes!\n");
  exit(0);
}
