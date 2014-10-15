/* This is our implementation of the reliable multicast protocol  */
/* It utilizes a ring in order to provide agreed reliable delivery */

#include "net_include.h"
#include "unistd.h"
#include "stdlib.h"

void setup(struct initializers *i) {
  /* Sets up all ports */
  /* and waits for the start_mcast message to start the actual process */
   int              mcast_addr;
   int              start = 0;
   int              bytes;
   int              num, c;
   socklen_t        from_len;
   fd_set           dummy_mask,temp_mask;
   struct		timeval timeout;
    fd_set             mask;
    struct ip_mreq     mreq;
    unsigned char      ttl_val;
   char logfilename[10];
   snprintf(logfilename, 10, "%d.out", i->machine_index);
   i->written_seq = -1;
   i->prior_token_aru = -1;
   i->local_round = -1;
   i->local_aru = -1;
   i->last_to_send_token = 0;
   /*Null out the data structure*/
   for (c =0; c < ARRAY_SIZE; c++) {
      i->unwritten_packets[c] = NULL;
   }
   i->logfile = fopen(logfilename, "w");

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
                if (i->debug) printf( "received : %d\n", i->mess_buf[0] );
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

void add_packet(struct initializers *i, struct packet_structure *p){
  /*
 *    * Adds an incoming packet to the data structure by mallocing the appropriate
 *       * space and then memcpying the incoming data into the newly allocated space.
 *          */
  i->unwritten_packets[p->sequence % ARRAY_SIZE] = malloc(sizeof(struct packet_structure));
  memcpy(i->unwritten_packets[p->sequence % ARRAY_SIZE], p,
         sizeof(struct packet_structure));
}
void send_id(struct initializers *i)
{
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  if (i->debug) printf("Sending machine id to group\n");
  p->type = 3;
  p->sequence = i->machine_index;
  sendto( i->ss, (char *)p, sizeof(struct packet_structure), 0,
          (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
}
void update_token(struct token_structure *t, int sequence) {
  /* updates the token with sequence number */
}

int write_log(struct initializers *i, struct token_structure *t) {
  /* writes to log for all received data
   * When implementing make sure to free the pointer in the unwritten_packets
   * array and then set that pointer to null.
   */
   int num_to_write = 0;
   int c;
   int seq;
   /* if (i->debug) printf("Writing to log:prior: %d, aru: %d tseq = %d\n", i->prior_token_aru, t->aru, t->sequence); */
   num_to_write = ((i->prior_token_aru > t->aru)?t->aru:i->prior_token_aru);
   for (c=i->written_seq+1; c <= num_to_write; c++) {   
     seq = c % ARRAY_SIZE;
     fprintf(i->logfile, "%2d, %8d, %8d\n", i->unwritten_packets[seq]->machine_index, 
					    i->unwritten_packets[seq]->sequence, 
					    i->unwritten_packets[seq]->random_number);
     /* free(i->unwritten_packets[seq]); */
     if (i->debug) printf("%2d, %8d, %8d\n", i->unwritten_packets[seq]->machine_index,
                                            i->unwritten_packets[seq]->sequence,
                                            i->unwritten_packets[seq]->random_number);
     /* We've written it, so null it out */
     i->unwritten_packets[seq] = NULL; /*Free memory later */

   }
   i->written_seq = num_to_write;
}

// Update_rtr works under the assumption that the rtr is going to clear once it
// gets called. This should always be true because we clear the rtr everytime we
// do a "send_rtr_packets" call.

void update_rtr(struct initializers *i, struct token_structure *t){
  /*Writes rtr to token based on missing packets */
  // May need debugging! Haven't tested this code yet!

  // Using written_seq as an indicator for the sequence number of most recent
  // written packet

    int j;
    t->rtrcount = 0;

    // we use the sequence on the current token to tell us what the highest
    // value packet is that has been sent out. Then we know that the packets
    // were are missing have sequence greater than written_seq and less than the
    // token's sequence.
    // Modified to use local aru instead of written sequence.  
    for (j = i->local_aru + 1; j <= t->sequence; j++) {
        // if we encounter a null value in our array, we want to put the
        // sequence number in the rtr.
        if (i->unwritten_packets[j % ARRAY_SIZE] == NULL) {
            // insert_rtr is used to insert a new rtr into the token.
            t->rtr[t->rtrcount] = j;
	    if (i->debug) printf("Requesting (RTR), packet: %d\n", j);
 	    t->rtrcount++;
        }
    }

    // Should exit loop when we get to the last packet sent by the whole group
    // which has sequence number shown on the token sequence.
}


void printpacket (struct packet_structure *p) {
  printf("Machine id: %d, Seq: %d, Rand: %di\n", p->machine_index, p->sequence, p->random_number);
}

void printtoken (struct token_structure *t, struct initializers *i) {
  printf("Token Seq: %d, Aru: %d, Loss level %d, RTRCount: %d Round %d, Local aru: %d, PriorARU: %d\n", t->sequence, t->aru, t->loss_level, t->rtrcount, t->round, i->local_aru, i->prior_token_aru);
}

struct packet_structure *generate_packet(struct initializers *i, struct token_structure *t){
  /* Generates the next packet, and */
  int r = rand() % 1000000 + 1;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  if (t->aru == t->sequence) { /* If we are in sequence with the token increase the aru */
     t->aru++;
  }
  if (i->local_aru == t->sequence) {
     i->local_aru++;
  }
  t->sequence++; /* Increase the token sequence number */
  p->sequence = t->sequence;
  p->received=0; /* Packet sent is set to 0, so receiving machine can update */
  p->machine_index = i->machine_index;
  p->type = 1; /*packet data type */
  p->random_number=r;
  /* Stores the newly generated packet into the unwritten array */
  add_packet(i, p);
  if (i->debug) printpacket(p);
  if (i->debug) printf("T sequence = %d\n", t->sequence);
  return p;
}
void receive_packet(struct initializers *i, struct token_structure *t) {
  /* receiving data */
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  p = (struct packet_structure *)i->mess_buf;
  //if (i->debug) printpacket(p);
  //if (i->debug) printf("Recieved type %d, seq %d taru = %d, laru = %d\n", p->type, p->sequence, t->aru, i->local_aru);
  add_packet(i, p);
  if (i->local_aru == (p->sequence -1))
  {
     i->local_aru++;
     /*Check to see if we have already received the next packets, and advance the local aru*/
     while (i->unwritten_packets[(i->local_aru+1)%ARRAY_SIZE] != NULL)
     {
       i->local_aru++;
     }
  }
}

void send_data(struct initializers *i, struct token_structure *t){
  /*sends data up to fcc*/
  struct packet_structure *packet;
  int p;
  int pr = t->sequence - ((i->prior_token_aru > t->aru)?t->aru:i->prior_token_aru); /* Number of outstanding packets */
  int psend; /*Number of packets we are sending send */
  int fcc = t->fcc;
  int sent;
  if (pr >= 1024 - t->fcc) { /* Too many tokens on ring, send less than fcc */
    fcc = 0;
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
      sent = sendto(i->ss, packet, sizeof(struct packet_structure), 0,
        (struct sockaddr *)&i->send_addr, sizeof(i->send_addr));
      if (i->debug) printf("Sent sequence %d \n", packet->sequence);
      i->packets_to_send--; /* Decrement the packets to send */
    }
  }
  else {
   t->nodata[i->machine_index-1] = 1; /*add us to the machines with no data to send */
  }
  if (i->debug) printtoken(t, i);
}

void send_token(struct initializers *i,struct token_structure *t) {
  /*sends the current token to the next process (unicast)*/
  int size, from_ip;
  int z;
  size = sendto( i->ts, (char *)t, sizeof(struct token_structure), 0,
          (struct sockaddr *)&i->next_machine_addr, sizeof(i->next_machine_addr));
  if (i->debug) printf("Sending Token TSeq: %d, TAru: %d, Round: %d Recovered: %d\n", t->sequence, t->aru, t->round, t->recovered);
  from_ip = i->next_machine_addr.sin_addr.s_addr;
  if (i->debug) printf( "token sent to (%d.%d.%d.%d): \n",
	(htonl(from_ip) & 0xff000000)>>24,
	(htonl(from_ip) & 0x00ff0000)>>16,
	(htonl(from_ip) & 0x0000ff00)>>8,
	(htonl(from_ip) & 0x000000ff) );
}

/*
 * send_rtr_packets will now set the rtr array values to 0 so that update_rtr can
 * easily put the correct retransmit requests back into the array.
 *
 */

void send_rtr_packets(struct initializers *i, struct token_structure *t) {
    int j = 0;
   
    for (j = 0; j < t->rtrcount; j++) {
        if (i->unwritten_packets[t->rtr[j] % ARRAY_SIZE] != NULL) {
           if (i->debug) printpacket(i->unwritten_packets[t->rtr[j] % ARRAY_SIZE]);
            sendto(i->ss, i->unwritten_packets[t->rtr[j] % ARRAY_SIZE],
                   sizeof(struct packet_structure), 0, (struct sockaddr *)&i->send_addr,
                   sizeof(i->send_addr));
        }
        t->rtr[j] = 0;
    }
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
  int                ss,sr, bytes, num, c, allreceived;
  fd_set             mask;
  fd_set             dummy_mask,temp_mask;
  struct timeval    timeout, start;
  socklen_t        from_len;
  struct sockaddr_in receive_from;
  struct initializers *i=malloc(sizeof(struct initializers));
  struct token_structure *t=malloc(sizeof(struct token_structure));
  struct token_structure *r_token=malloc(sizeof(struct token_structure));
  struct token_structure *last_token=malloc(sizeof(struct token_structure));
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  t->fcc = FCC;
  i->debug = 0; /*Turn on for testing */
  parseargs(argc, argv, i);
  struct timeval ti;
	gettimeofday( &ti, NULL );
	srand( ti.tv_sec );
  i->packet_index = 0;
  setup(i); /*Setup ports and wait for start process */
  send_id(i); /*Send out our ID to the group */
  get_neighbor(i); /* Get our neighbor's id */
  i->max_packets = FCC*6;
  recv_dbg_init(i->loss_rate, i->machine_index);
  if (i->machine_index == 1) {
    t->type = 2;
    t->sequence = -1;
    t->aru = -1;
    t->loss_level = 0;
    t->round = 0;
    t->rtrcount = 0;
    for (c=0; c < i->total_machines; c++) {
      t->nodata[c] = 0;
    }
    sleep(1); /* Work on startup before removing this! */
    send_data(i, t);
    i->prior_token_aru = t->aru; /* Update last aru */
    if (i->debug) printtoken(t, i); 
    i->last_to_send_token =1;
    send_token(i, t);
    if (i->debug) printf("I'm first, sending the initial token\n");
  }
  FD_ZERO(&mask);
  FD_ZERO(&dummy_mask);
  FD_SET(i->sr, &mask);
  while(1) {
    
    if (i->machine_index == 1) {
       timeout.tv_sec = 0;
       timeout.tv_usec = 1000000;
    }
    else {
       timeout.tv_sec = 5;
       timeout.tv_usec = 0;
    }
    temp_mask = mask;
    num = select (FD_SETSIZE, &temp_mask, &dummy_mask, &dummy_mask, &timeout);
    if (num > 0){
      if ( FD_ISSET( i->sr, &temp_mask) ) {
        bytes = recv_dbg( i->sr, i->mess_buf, sizeof(i->mess_buf), 0);
        i->mess_buf[bytes] = 0;
        p = (struct packet_structure *)i->mess_buf;
        i->last_to_send_token = 0; /*Reset since we received data */
        if (p->type == 1 && p->machine_index != i->machine_index){
          /* Data packet. store it to memory */
          if (p->sequence >= i->local_aru) { /* Only accept if we need it */
	     receive_packet(i, t);
	   }
	}
        else if (p->type == 2){
	  r_token = (struct token_structure *)i->mess_buf;
	  if (r_token->round > i->local_round ) { /*New token not a dupe */
	    if (i->debug) printtoken(r_token,i); 
	    if (i->debug) printf("Token Accepted, Round: %d\n", r_token->round);
            /*Copy the received token to our token */
	    t = (struct token_structure *)i->mess_buf;
            /*update the round counter */
            if (i->machine_index == 1) {
	      t->round++;
	      i->local_round++;
	      t->recovered = 0;
            }
  	    else {
	      i->local_round = t->round;
	    }
            if (i->debug) printf("New Round %d\n", t->round);
            write_log(i, t); /*Write what has all been received. */
            /* t->aru = i->local_aru; */
            if (t->aru > i->local_aru) { /*Lower token aru to our aru (It is higher than the highest in order message)*/
	       printf("Lowering ARU from %d to %d\n", t->aru, i->local_aru);
               t->aru = i->local_aru; 
	       t->aru_lowered_by = i->machine_index;
            }
 	    /* If is the one that lowered the aru, and the token.aru is still the same, should set token.aru to its local aru. */
            if (i->prior_token_aru == t->aru && i->local_aru > t->aru && t->aru_lowered_by == i->machine_index) {
		t->aru = i->local_aru; 
		if (i->debug) printf("Raising ARU\n"); 
	     }
            if (i->debug) printf("Prioraru %d t_aru %d, localaru: %d, tseq: %d, lowby: %d\n", i->prior_token_aru, t->aru, i->local_aru, t->sequence, t->aru_lowered_by);
            i->prior_token_aru = t->aru; /*Done with prior token aru, set the prior token aru to the token aru */
            send_rtr_packets(i, t);
            send_data(i, t);  /*Send data is going to update the token too*/
            update_rtr(i, t);
            /* update_token */

          /* End when all have sent their data */
          if (i->packets_to_send == 0 && t->sequence == i->prior_token_aru && t->aru == i->prior_token_aru) {
             if (i->debug) printf("Nodata: %d, tseq: %d, prior aru: %d\n", t->nodata, t->sequence, i->prior_token_aru);
              allreceived=1;
              for (c=0; c < i->total_machines; c++) {
                if (t->nodata[c] == 0) {
                  allreceived = 0;
                }
              }
              if (allreceived) {
		printf("Ending. Sending token to %d", i->next_machine);
		send_token(i, t);
		 write_log(i, t); 
                fclose(i->logfile);
                  exit(0);
                }
	    }
	  i->last_to_send_token = 1;
          if (t->loss_level > 0) {
	    send_token(i,t);send_token(i,t);send_token(i,t);
	  }
	  send_token(i,t); send_token(i,t);  
    	  last_token->sequence = t->sequence;
	  last_token->aru = t->aru;
	  last_token->round = t->round;
	  last_token->aru_lowered_by = t->aru_lowered_by;
    }
    else {
      if (i->debug) printf("Received token already! Local: %d, Token: %d\n", i->local_round, t->round);
      if (i->machine_index == 1) {
          if (i->local_round > t->round && 0)
	    {
		printf("Local round out of sync! This shouldn't happen! local: %d, Token: %d\n", i->local_round, t->round);
		fclose(i->logfile);
		exit(0);
	  }
	}
    }
        }
        else if (p->type == 4){
          /*Request to send machine id */
        }
        else if (p->type == 0) {
	  if (i->debug) printf("Dropped packet\n");
        }
      }

    }
    else /*We timed out */
    {
      printf("Timed out \n");
      if (i->machine_index == 1) {
          if (t->type < 2) {
	    printf("Token data retreival failed -- recovering%d\n", t->type);
            t->type = 2; /*Really bizarre.  For some reason our token got lost--some memory assignment pointer issue for sure */
            t->aru = last_token->aru;
	    t->round = i->local_round;
            t->round = t->round+2;
            i->local_round= i->local_round+1;
            t->aru_lowered_by = last_token->aru_lowered_by;
            t->sequence = last_token->sequence;
	  }
	  t->loss_level++;
	  t->recovered = 1;
	  printf("**********Token Drop Detected, localround: %d\n", i->local_round); 
	  printtoken(t, i); 
	  send_token(i,t); /*We sent the token last, resend it here */
	  printf("Resent token round %d with aru %d to process %d token type: %d", t->round, t->aru, i->next_machine, t->type);
	}
      else {
	write_log(i, t);
        fclose(i->logfile);
	printf("Finishing with localaru: %d, and token:\n", i->local_aru);
	printtoken(t, i);

        exit(0);
	}
    }
  }
  return (0);
}
