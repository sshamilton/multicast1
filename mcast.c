/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"
#include "unistd.h"
#include "stdlib.h"

struct token_structure token_generate() {
  struct token_structure t;
  return t;
};

void setup(struct initializers *i) {
  /* Sets up all ports */
  /* and waits for the start_mcast message to start the actual process */
   int              mcast_addr;
   int              start = 0;
   int              bytes;
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
    while (start == 0) {
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      temp_mask = mask;
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
                start = i->mess_buf[0];

        }

      }
      else {printf("No start sig\n");}
    }

};
void get_neighbor(struct initializers *i){
  fd_set  dummy_mask,temp_mask;
  struct	timeval timeout, end_time, start_time;
  fd_set  mask;
  int     next_machine_ip =0;
  int     num,bytes,sender_id;
  socklen_t        from_len;
  gettimeofday(&start_time, NULL);
  double t1, t2;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  t1=start_time.tv_sec+(start_time.tv_usec/1000000.0);
  /* Calculate our neighbor */
  if (i->machine_index == i->total_machines) {
    /*We are largest, so our neighbor is first machine */
    i->next_machine =1;
  }
  else{
    i->next_machine = i->machine_index + 1;
  }
  FD_ZERO(&mask);
  FD_ZERO(&dummy_mask);
  FD_SET(i->sr, &mask);
  p->type = 3;
  p->sequence = i->machine_index;
  while (next_machine_ip == 0) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 2000;
    temp_mask = mask;
    gettimeofday(&end_time, NULL);
    t2=end_time.tv_sec+(start_time.tv_usec/1000000.0);
    if (t1 < (t2-10)) {
      /*Timed out.  Requesting machine ids*/
      if (i->debug) printf("Timed out requesting machine id\n");
      p->sequence = i->machine_index;
      p->type = 4;
      sendto( i->ss, (char *)p, sizeof(p), 0,
              (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
            if (i->debug) printf("sent request for machine ids\n");
      t1 = t2;
    }
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
	      p = (struct packet_structure *)i->mess_buf;
              printf( "received :Type %d, machine id %d \n", p->type, p->sequence );
      	printf("Neighbor is %d", i->next_machine);
	/* Check to see if this is our neighbor */
        if (p->type == 3) {
          sender_id = p->sequence;
          if (i->next_machine == sender_id)
            {
              if (i->debug) printf("Found our neighbor %d\n", sender_id);
              next_machine_ip = i->next_machine_addr.sin_addr.s_addr;
	      i->next_machine_addr.sin_port = htons(PORT);
            }
        }
        else {
          if (i->debug) printf("Expected machine id type, got: %d\n", i->mess_buf[0]);

        }
      }
    }

  }
}
void send_id(struct initializers *i)
{
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  if (i->debug) printf("Sending machine id to group\n");
  p->type = 3;
  p->sequence = i->machine_index;
  sendto( i->ss, (char *)p, sizeof(struct packet_structure), 0,
          (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
          printf("sent\n");
}
void update_token(struct token_structure *t, int sequence) {
  /* updates the token with sequence number */
}

int write_log(struct initializers *i) {
  /* writes to log for all received data
   * When implementing make sure to free the pointer in the unwritten_packets
   * array and then set that pointer to null.
   */
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
  /* Stores the newly generated packet into the unwritten array */
  i->unwritten_packets[p->sequence % ARRAY_SIZE] = p;
  return p;
}
void receive_packet(struct initializers *i, struct token_structure *t) {
  /* receiving data */
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  p = (struct packet_structure *)i->mess_buf;
  printf("Recieved type %d, seq %d\n", p->type, p->sequence);
}
void send_data(struct initializers *i, struct token_structure *t){
  /*sends data up to fcc*/
  struct packet_structure *packet;
  int p;
  int pr = t->sequence - ((i->prior_token_aru > t->aru)?t->aru:i->prior_token_aru); /* Number of outstanding packets */
  int psend; /*Number of packets we are sending send */
  int fcc = t->fcc;
  if (pr < t->fcc) { /* Too many tokens on ring, send less than fcc */
    fcc = pr;
  }
  if (i->packets_to_send > 0) {
    if (i->packets_to_send > fcc ) {
      psend = fcc; /* send to max fcc */
    }
    else {
      psend = i->packets_to_send; /*we are getting to the end of our packets, send the rest */
    }
    for (p=1; p<=psend; p++) {
      packet = generate_packet(i, t);
      sendto(i->ss, packet, sizeof(struct packet_structure), 0,
        (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
      i->packets_to_send--; /* Decrement the packets to send */
    }
  }
}

void send_token(struct initializers *i,struct token_structure *t) {
  /*sends the current token to the next process (unicast)*/
  int size, from_ip;
  int z;
  sleep(2);
  printf("Sending token");
  size = sendto( i->ts, (char *)t, sizeof(struct token_structure), 0,
          (struct sockaddr *)&i->next_machine_addr, sizeof(i->next_machine_addr));
  printf("Sendto result: %d\n", size);
  from_ip = i->next_machine_addr.sin_addr.s_addr;
  printf( "sent to (%d.%d.%d.%d): \n",
	(htonl(from_ip) & 0xff000000)>>24,
	(htonl(from_ip) & 0x00ff0000)>>16,
	(htonl(from_ip) & 0x0000ff00)>>8,
	(htonl(from_ip) & 0x000000ff) );
}

void add_packet(struct initializers *i, struct packet_structure *p){
  /*
   * Adds an incoming packet to the data structure by mallocing the appropriate
   * space and then memcpying the incoming data into the newly allocated space.
   */
  i->unwritten_packets[p->sequence % ARRAY_SIZE] = malloc(sizeof(struct packet_structure));
  memcpy(i->unwritten_packets[p->sequence % ARRAY_SIZE], p,
         sizeof(struct packet_structure));
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
/* Message types: */
/* 1 = Data */
/* 2 = Token */
/* 3 = Machine id message */
/* 4 = Machine id request */
int main(int argc, char **argv)
{
	/* Variables */
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    int                mcast_addr;
    struct ip_mreq     mreq;
    unsigned char      ttl_val;
    int                ss,sr, bytes, num;
    fd_set             mask;
    fd_set             dummy_mask,temp_mask;
    struct timeval    timeout, start;
    socklen_t        from_len;
    struct sockaddr_in receive_from;

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
  setup(i); /*Setup ports and wait for start process */
  send_id(i); /*Send out our ID to the group */
  get_neighbor(i); /* Get our neighbor's id */
  i->max_packets = FCC*6;
  if (i->machine_index == 1) {
    send_data(i, t); /*Send data is going to update the token too*/
    t->type = 2;
    t->sequence = 0;
    t->aru = 0;
    t->loss_level = 0;
    send_data(i, t);
    send_token(i, t);
    if (i->debug) printf("I'm first, sending the initial token\n");
  }
  FD_ZERO(&mask);
  FD_ZERO(&dummy_mask);
  FD_SET(i->sr, &mask);
  while(1) {
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    temp_mask = mask;
    num = select (FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, &timeout);
    if (num > 0){
      if (i->debug) printf("Rcv\n");
      if ( FD_ISSET( i->sr, &temp_mask) ) {
        bytes = recvfrom( i->sr, i->mess_buf, sizeof(i->mess_buf), 0,
                          (struct sockaddr *)&receive_from,
                          &from_len );
        i->mess_buf[bytes] = 0;
        p = (struct packet_structure *)i->mess_buf;
        if (i->debug) printf("Pack type %d rcv in main\n", p->type);
        sleep(1);
        if (p->type == 1){
          /* Data packet. store it to memory */
          printf("Rcv type 1\n");
          receive_packet(i, t);
	}
        else if (p->type == 2){
          /* Token received */
          if (i->debug) printf("Received token");
          printf("Rcv type 2\n|");
          /* send_rtr(i, t); */
          /* send_data(i, t); */ /*Send data is going to update the token too*/
          /* update_rtr(i, t); */
          /* update_token */
	  t = (struct token_structure *)i->mess_buf;
          send_token(i, t);
        }
        else if (p->type == 4){
          /*Request to send machine id */
        }
      }

    }
  }
  return (0);
}
