/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"

struct token_structure token_generate() {
  struct token_structure t;
  return t;
};

void setup(struct sockaddr_in *name, struct sockaddr_in *send_addr, int*
            mcast_addr, struct ip_mreq *mreq, unsigned char *ttl_val, int *ss,
            int *sr, fd_set *mask, fd_set *dummy_mask, fd_set *temp_mask) {
  /* waits for the start_mcast message to start the actual process */
    *mcast_addr = 225 << 24 | 0 << 16 | 1 << 8 | 1; // Will be changed later

    *sr = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for receiving */
    if (*sr < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    name->sin_family = AF_INET;
    name->sin_addr.s_addr = INADDR_ANY;
    name->sin_port = htons(PORT);

    if (bind( *sr, (struct sockaddr *) name, sizeof(*name) ) < 0) {
        perror("Mcast: bind");
        exit(1);
    }

    mreq->imr_multiaddr.s_addr = htonl( *mcast_addr );

    mreq->imr_nterface.s_addr = htonl( INADDR_ANY );

    if (setsockopt(*sr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)mreq,
        sizeof(*mreq)) < 0) {
        perror("Mcast: problem in setsockopt to join multicast address" );
    }

    *ss - socket(AF_INT, SOCK_DGRAM, 0); /* Socket for sending */

    if (ss < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    *ttl_val = 1;
    if (setsockopt(*ss, IPPROTO_IP, IP_MULTICAST_TTL, (void *)ttl_val,
        sizeof(*ttl_val)) < 0 ) {
        printf("Mcast: problem in setsockopt of multicast ttl %d - ignore in
            WinNT or Win95\n", *ttl_val);
    }

    send_addr->sin_family = AF_INET;
    send_addr->sin_addr.s_addr = htonl(mcast_addr);
    send_addr->sin_port = htons(PORT);

    FD_ZERO( mask );
    FD_ZERO( &dummy_mask );
    FD_SET( sr, mask );
    FD_SET( (long)0, &mask );

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

void send_data(struct initializers *i, struct token_structure *t){
  /*sends data up to fcc*/
}

void send_token(struct token_structure *t){
  /*sends the current token to the next process*/
}

void add_packet(struct initializers *i,struct packet_structure *p){
  /*adds an incoming packet to the data structure.*/
}

void send_rtr_packets(struct initializers *i, struct token_structure *t){
  /* sends packets needed to be retransmitted from token rtr */
}

struct packet_structure generate_packet(struct initializers *i, struct token_structure *t){
  /* Generates the next packet, and */
  int r = rand() % 1000000 + 1;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  t->sequence++; /* Increase the token sequence number */
  p->token_sequence = t->sequence;
  i->packet_index++; /*Increase the packet sequence number */
  p->packet_index = i->packet_index;
  p->received=0; /* Packet sent is set to 0, so receiving machine can update */
  p->machine_index = i->machine_index;
  p->random_number=r;
  return *p;
}

int parseargs(int argc, char **argv, struct initializers *i)
{
    char               *at; /* position of @ symbol in 3rd arg */
    char               *compname; /* remote computer name */
    /*Ensure we got the right number of arguments */
    if(argc !=5) {
        printf("Usage: mcast <num_of_packets> <machine_index> <number of machines> <loss rate>");
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

    setup(&name, &send_addr, &mcast_addr, &mreq, &ttl_val, &ss, &sr, &mask,
            &dummy_mask, &temp_mask);

	struct initializers *i=malloc(sizeof(struct initializers));
	struct token_structure *t=malloc(sizeof(struct token_structure));
	struct packet_structure *p=malloc(sizeof(struct packet_structure));
  parseargs(argc, argv, i);
  struct timeval ti;
	gettimeofday( &ti, NULL );
	srand( ti.tv_sec );
  i->packet_index = 0;

  return (0);
}
