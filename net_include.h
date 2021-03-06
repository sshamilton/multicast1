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
#include "recv_dbg.h"

#define PORT	     10080 /* assigned address */
#define FCC          600 /* Tuned */
#define PACKET_SIZE  1200
#define MAX_MESS_LEN 1500
#define ARRAY_SIZE   16384
#define MAX_RTR      250

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
  int rtr[MAX_RTR];
  int rtrcount;
  int loss_level;
  int nodata[10];
  int round;
  int aru_lowered_by;
};

struct initializers {
  /* Sequence value of the most recent packet written to log */
  int written_seq;
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
  int local_aru;
  int max_packets;
  int local_round;
  char mess_buf[MAX_MESS_LEN];
  struct sockaddr_in name;
  struct sockaddr_in send_addr;
  struct sockaddr_in next_machine_addr;
  int ss, sr, ts; /*token send socket added */
  /* Array of unwritten packets */
  struct packet_structure* unwritten_packets[ARRAY_SIZE];
};
