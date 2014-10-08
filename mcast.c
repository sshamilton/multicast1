/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"
#include "unistd.h"
struct token_structure token_generate() {
  struct token_structure t;
  return t;
};

void setup(struct initializers *i) {
  /* Sets up all ports */
  /* and waits for the start_mcast message to start the actual process */
   int              mcast_addr;
   int              next_machine_ip =0;
   int              bytes;
   int              sender_id;
   int              num;
   socklen_t        from_len;
   fd_set           dummy_mask,temp_mask;
   struct		timeval timeout;
    fd_set             mask;
    struct ip_mreq     mreq;
    unsigned char      ttl_val;
    mcast_addr = 225 << 24 | 1 << 16 | 2 << 8 | 108; // 225.1.2.108
    i->sr = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for multicast receiving */
    if (i->sr < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    i->name.sin_family = AF_INET;
    i->name.sin_addr.s_addr = INADDR_ANY;
    i->name.sin_port = htons(PORT);
    mreq.imr_multiaddr.s_addr = htonl( mcast_addr );
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );
    i->send_addr.sin_family = AF_INET;
    i->send_addr.sin_addr.s_addr = htonl(mcast_addr);  /* mcast address */
    i->send_addr.sin_port = htons(PORT);
    if (bind( i->sr, (struct sockaddr *) &i->name, sizeof(i->name) ) < 0) {
        perror("Mcast: bind");
        exit(1);
    }

    if (setsockopt(i->sr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq,
        sizeof(mreq)) < 0) {
        perror("Mcast: problem in setsockopt to join multicast address" );
    }

    i->ss = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for sending */

    if (i->ss < 0) {
        perror("Mcast: socket");
        exit(1);
    }
    i->ts = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for token sending */

    if (i->ts < 0) {
        perror("Ucast: socket");
        exit(1);
    }

    ttl_val = 1;
    if (setsockopt(i->ss, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl_val,
        sizeof(ttl_val)) < 0 ) {
        printf("Mcast: problem in setsockopt of multicast ttl %d - ignore in"
               "WinNT or Win95\n", ttl_val); }

    FD_ZERO(&mask);
    FD_ZERO(&dummy_mask);
    FD_SET(i->sr, &mask);

    /* Now we need to send our machine info out periodically, and wait to find our neighbor */

    /* Calculate our neighbor */
    if (i->machine_index == i->total_machines) {
      /*We are largest, so our neighbor is first machine */
      i->next_machine =1;
    }
    else{
      i->next_machine = i->machine_index + 1;
    }

    /* Send out hello packet and wait for neighbor's hello */
    if (i->debug) printf("Sending hello to group\n");
    i-> mess_buf[0] = i->machine_index;
    sendto( i->ss, i->mess_buf, sizeof(i->machine_index), 0,
            (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
            printf("sent\n");
    temp_mask = mask;
    while (next_machine_ip == 0) {
      timeout.tv_sec = 5;
      timeout.tv_usec = 2000;
      num = select (FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, &timeout);
      if (num > 0){
        printf("Rcv\n");
        if ( FD_ISSET( i->sr, &temp_mask) ) {
                bytes = recvfrom( i->sr, i->mess_buf, sizeof(i->mess_buf), 0,
                                  (struct sockaddr *)&i->next_machine_addr,
                                  &from_len );
                /* Note: This may not be the next machine, but we are using
                        the sockaddr called this for reuse purposes */
                i->mess_buf[bytes] = 0;
                printf( "received : %d\n", i->mess_buf[0] );
        /* Check to see if this is our neighbor */
            sender_id = i->mess_buf[0];
            if (i->next_machine == sender_id)
              {
                if (i->debug) printf("Found our neighbor %d", sender_id);
                next_machine_ip = i->next_machine_addr.sin_addr.s_addr;
              }
        }
      }
    }

};

void update_token(struct token_structure *t, int sequence) {
  /* updates the token with sequence number */
}

int write_log(struct initializers *i) {
  /*writes to log for all received data */
}

void update_rtr(struct initializers *i){
  /*Writes rtr to token based on missing packets */
}
struct packet_structure *generate_packet(struct initializers *i, struct token_structure *t){
  /* Generates the next packet, and */
  int r = rand() % 1000000 + 1;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  t->sequence++; /* Increase the token sequence number */
  p->sequence = t->sequence;
  i->packet_index++; /*Increase the packet sequence number */
  p->packet_index = i->packet_index;
  p->received=0; /* Packet sent is set to 0, so receiving machine can update */
  p->machine_index = i->machine_index;
  p->random_number=r;
  return p;
}
void send_data(struct initializers *i, struct token_structure *t){
  /*sends data up to fcc*/
  struct packet_structure *packet;
  int p;
  int psend; /*Number of packets we are sending send */
  if (i->packets_to_send > 0) {
    if (i->packets_to_send > t->fcc){
      psend = t->fcc; /*We have max fcc packets */
    }
    else {
      psend = i->packets_to_send; /*we are getting to the end of our packets, send the rest */
    }
    for (p=1; p<=psend; p++) {
      packet = generate_packet(i, t);
      sendto(i->ss, packet, sizeof(packet), 0,
        (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
      i->packets_to_send--; /* Decrement the packets to send */
    }
  }
}

void send_token(struct initializers *i,struct token_structure *t){
  /*sends the current token to the next process (unicast)*/
}

void add_packet(struct initializers *i,struct packet_structure *p){
  /*adds an incoming packet to the data structure.*/
}

void send_rtr_packets(struct initializers *i, struct token_structure *t){
  /* sends packets needed to be retransmitted from token rtr */
}

int parseargs(int argc, char **argv, struct initializers *i)
{
    char               *at; /* position of @ symbol in 3rd arg */
    char               *compname; /* remote computer name */
    /*Ensure we got the right number of arguments */
    if(argc !=5) {
        printf("Usage: mcast <num_of_packets> <machine_index> <number of machines> <loss rate>\n");
        exit(0);
    }
    else {
        i->packets_to_send = atoi(argv[1]);
        i->machine_index = atoi(argv[2]);
        i->total_machines = atoi(argv[3]);
        i->loss_rate = atoi(argv[4]);
        return 1;
    }
}
int main(int argc, char **argv)
{
	/* Variables */
    struct sockaddr_in name;
    struct sockaddr_in send_addr;

    int                mcast_addr;

    struct ip_mreq     mreq;
    unsigned char      ttl_val;

    int                ss,sr;
    fd_set             mask;
    fd_set             dummy_mask,temp_mask;

	struct initializers *i=malloc(sizeof(struct initializers));
	struct token_structure *t=malloc(sizeof(struct token_structure));
	struct packet_structure *p=malloc(sizeof(struct packet_structure));
  t->fcc = FCC;
  i->debug = 1; /*Turn on for testing */
  parseargs(argc, argv, i);
  struct timeval ti;
	gettimeofday( &ti, NULL );
	srand( ti.tv_sec );
  i->packet_index = 0;
  setup(i);
  if (i->machine_index == 1) {
    send_data(i, t); /*Send data is going to update the token too*/
    send_token(i, t);
  }
  return (0);
}
