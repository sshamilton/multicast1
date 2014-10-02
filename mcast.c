/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"

struct token_structure token_generate() {
  struct token_structure t;
  return t;
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
