#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>

#define PORT	       10080 /* assigned address */
#define FCC          200
#define PACKET_SIZE  1200
#define MAX_MESS_LEN 8192

/* Initializer variables */
struct packet_structure {
  int type;
  int sequence;
  int received;
  int machine_index;
  int packet_index;
  int random_number;
  char data[PACKET_SIZE];
  struct packet_structure *next;
};

struct token_structure {
  int type;
  int sequence;
  int aru;
  int fcc;
  int rtr[FCC];
  int rcv_process_id;
  int loss_level;
};

struct initializers {
  struct packet_structure *head;
  struct packet_structure *tail;
  FILE *logfile;
  int packets_to_send;
  int machine_index;
  int packet_index;
  int total_machines;
  int next_machine;
  int loss_rate;
  int debug;
  int token_timeout;
  int prior_token_aru;
  char  mess_buf[MAX_MESS_LEN];
  struct sockaddr_in name;
  struct sockaddr_in send_addr;
  struct sockaddr_in next_machine_addr;
  int                ss,sr, ts; /*token send socket added */

};
