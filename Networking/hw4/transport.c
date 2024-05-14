/*
 * transport.c
 *
 * CPSC4510: Project 3 (STCP)
 *
 * This file implements the STCP layer that sits between the
 * mysocket and network layers. You are required to fill in the STCP
 * functionality in this file.
 *
 */

#include "transport.h"
#include "mysock.h"
#include "stcp_api.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum {
  CSTATE_ESTABLISHED,
  FIN_WAIT_1,
  FIT_WAIT_2,
  CLOSING,
  CLOSE_WAIT,
  LAST_ACK
}; /* you should have more states */

int active = 0;

#define WINDOW_SIZE 3072

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;

  char snd_wndw[WINDOW_SIZE];
  int snd_ptr_f;
  int snd_ptr_b;

  char rcv_wndw[WINDOW_SIZE];
  int rcv_ptr_f;
  int rcv_ptr_b;

  uint seq_num;
  uint ack_num;

  /* any other connection-wide global variables go here */
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
  active = is_active; // used to introduce noise into rng process
  context_t *ctx;

  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);

  generate_initial_seq_num(ctx);

  STCPHeader *hdr = (STCPHeader *)calloc(1, sizeof(STCPHeader));
  memset(hdr, 0, sizeof(STCPHeader));
  if (is_active) {
    // send SYN
    hdr->th_flags |= TH_SYN;
    hdr->th_off = 5;
    hdr->th_win = htons(WINDOW_SIZE);
    hdr->th_seq = htonl(ctx->initial_sequence_num);
    ctx->seq_num = ctx->initial_sequence_num + 20;
    printf("syn sn: %d and ack: %d\n", ntohl(hdr->th_seq), ntohl(hdr->th_ack));
    if (stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL) == -1) {
      printf("error\n");
      return;
    }

    // receive SYNACK
    memset(hdr, 0, sizeof(STCPHeader));
    if (stcp_network_recv(sd, hdr, sizeof(STCPHeader)) != 20) {
      printf("error recv\n");
    }
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    ctx->ack_num = ntohl(hdr->th_seq) + 20; // rcv_seq is our ack num
    printf("received synack: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));

    // send ACK
    memset(hdr, 0, sizeof(STCPHeader));
    hdr->th_flags |= TH_ACK;
    hdr->th_off = 5;
    hdr->th_seq = htonl(ctx->seq_num);
    hdr->th_ack = htonl(ctx->ack_num);
    ctx->seq_num += 20;
    printf("sending ack sn: %d and ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    if (stcp_network_send(sd, hdr, sizeof(STCPHeader)) == -1) {
      printf("err snd\n");
      return;
    }
    printf("client side handshake finsished, current sn: %d, ack: %d\n",
           ctx->seq_num, ctx->ack_num);
  } else {
    // receive SYN
    if (stcp_network_recv(sd, hdr, sizeof(STCPHeader)) == -1) {
      printf("error recv\n");
      return;
    }
    printf("receiving syn: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    ctx->ack_num = ntohl(hdr->th_seq) + 20;
    assert((hdr->th_flags & TH_SYN));

    // send SYN-ACK
    ctx->seq_num = ctx->initial_sequence_num;
    memset(hdr, 0, sizeof(STCPHeader));
    hdr->th_flags |= TH_SYN | TH_ACK;
    hdr->th_off = 5;
    hdr->th_win = htons(WINDOW_SIZE);
    hdr->th_seq = htonl(ctx->seq_num);
    hdr->th_ack = htonl(ctx->ack_num);
    ctx->seq_num += 20;
    printf("sending synack with sn: %d and ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);

    // receive ACK
    memset(hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    printf("receiving ack: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    printf("th_seq: %d | ack: %d\n", ntohl(hdr->th_seq), ctx->ack_num);
    assert(ntohl(hdr->th_seq) == ctx->ack_num);
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    assert(hdr->th_flags & TH_ACK);
    ctx->ack_num = ntohl(hdr->th_seq) + 20;
    printf("server side handshake finsished, current sn: %d, ack: %d\n",
           ctx->seq_num, ctx->ack_num);
  }

  /* XXX: you should send a SYN packet here if is_active, or wait for one
   * to arrive if !is_active.  after the handshake completes, unblock the
   * application with stcp_unblock_application(sd).  you may also use
   * this to communicate an error condition back to the application, e.g.
   * if connection fails; to do so, just set errno appropriately (e.g. to
   * ECONNREFUSED, etc.) before calling the function.
   */
  ctx->connection_state = CSTATE_ESTABLISHED;
  stcp_unblock_application(sd);

  control_loop(sd, ctx);

  /* do any cleanup here */
  free(ctx);
}

/* generate random initial sequence number for an STCP connection */
static void generate_initial_seq_num(context_t *ctx) {
  assert(ctx);

#ifdef FIXED_INITNUM
  /* please don't change this! */
  ctx->initial_sequence_num = 1;
#else
  /* you have to fill this up */
  // The ISN was ending up the same if I wasn't differentiating by active and
  // passive -> not sure why if the srand(time) has a granularity of seconds.
  if (active) {
    srand(time(NULL) / 2);
  } else {
    srand(time(NULL) / 5);
  }
  ctx->initial_sequence_num = rand() % 256;
#endif
}

/* control_loop() is the main STCP loop; it repeatedly waits for one of the
 * following to happen:
 *   - incoming data from the peer
 *   - new data from the application (via mywrite())
 *   - the socket to be closed (via myclose())
 *   - a timeout
 */
static void control_loop(mysocket_t sd, context_t *ctx) {
  assert(ctx);
  assert(!ctx->done);

  while (!ctx->done) {
    unsigned int event;

    /* see stcp_api.h or stcp_api.c for details of this function */
    /* XXX: you will need to change some of these arguments! */
    event = stcp_wait_for_event(sd, 0, NULL);

    /* check whether it was the network, app, or a close request */
    if (event & APP_DATA) {
      /* the application has requested that data be sent */
      /* see stcp_app_recv() */
    }

    /* etc. */
  }
}

/**********************************************************************/
/* our_dprintf
 *
 * Send a formatted message to stdout.
 *
 * format               A printf-style format string.
 *
 * This function is equivalent to a printf, but may be
 * changed to log errors to a file if desired.
 *
 * Calls to this function are generated by the dprintf amd
 * dperror macros in transport.h
 */
void our_dprintf(const char *format, ...) {
  va_list argptr;
  char buffer[1024];

  assert(format);
  va_start(argptr, format);
  vsnprintf(buffer, sizeof(buffer), format, argptr);
  va_end(argptr);
  fputs(buffer, stdout);
  fflush(stdout);
}
