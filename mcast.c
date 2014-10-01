/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"
#define FCC 200
#define PACKET_SIZE 1200

/* Initializer variables */
struct initializers {
	struct packet_structure *head;
	struct packet_structure *tail;
	FILE *logfile;
	int packets_to_send;
	int machine_index;
	int packet_index;
	int total_machines;
	int loss_rate;
	int token_timeout;
	int prior_token_aru;
};

struct packet_structure {
	int token_sequence;
	int received;
	int machine_index;
	int packet_index;
	int random_number;
	char data[PACKET_SIZE];
	struct packet_structure *next;
};

struct token_structure {
  int sequence;
  int aru;
  int fcc;
  int rtr[FCC];
  int rcv_process_id;
  int loss_level;
};
/* Variables */
struct initializers *i;
struct token_structure *t;
struct packet_structure *p;

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

void generate_packet(struct initializers *i){
  /* Generates the next packet, and */
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
  parseargs(argc, argv, i);


}
