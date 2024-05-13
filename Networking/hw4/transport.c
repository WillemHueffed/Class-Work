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

enum { CSTATE_ESTABLISHED }; /* you should have more states */

#define WINDOW_SIZE 3072

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;

  char snd_wndw[WINDOW_SIZE];
  int snd_ptr_f;
  int snd_ptr_b;
  uint snd_seq;

  char rcv_wndw[WINDOW_SIZE];
  int rcv_ptr_f;
  int rcv_ptr_b;
  uint rcv_seq;

  /* any other connection-wide global variables go here */
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
  context_t *ctx;

  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);

  generate_initial_seq_num(ctx);

  STCPHeader hdr;
  memset(&hdr, 0, sizeof(STCPHeader));
  if (is_active) {
    // Send SYN
    // Is this the bug?
    // If I run it without the htons it crashes, if I run it with it the client
    // just hangs there
    // hdr.th_flags = TH_SYN;
    // hdr.th_flags = htons();
    // hdr.th_off = 5;
    // hdr.th_win = WINDOW_SIZE;
    // hdr.th_seq = ctx->initial_sequence_num;
    // ctx->snd_seq = ctx->initial_sequence_num + 1;

    if (stcp_network_send(sd, &hdr, sizeof(STCPHeader), NULL) == -1) {
      perror("send broke");
      exit(1);
    }
    printf("sent syn\n");

    // receive SYNACK
    memset(&hdr, 0, sizeof(STCPHeader));
    printf("at recv\n");
    stcp_network_recv(sd, &hdr, sizeof(STCPHeader));
    printf("past recv\n");
    // int ack = hdr.th_seq;
    //  jassert(ctx->snd_seq == ntohl(hdr.th_ack));
    //  assert(ntohl(hdr.th_flags) & (TH_SYN | TH_ACK));
    // printf("received synack\n");

    // send ACK
    memset(&hdr, 0, sizeof(STCPHeader));
    printf("sending ack\n");
    hdr.th_flags |= TH_ACK;
    hdr.th_off = 5;
    hdr.th_seq = ctx->snd_seq++;
    hdr.th_win = WINDOW_SIZE;

  } else {
    // receive SYN
    stcp_network_recv(sd, (void *)&hdr, sizeof(STCPHeader));
    // hdr.th_flags = ntohs(hdr.th_flags);
    // int ack = ntohl(hdr.th_seq) + 1;

    // send SYNACK
    // memset(&hdr, 0, sizeof(STCPHeader));
    memset(&hdr, 0, sizeof(STCPHeader));
    // hdr.th_flags = TH_ACK;
    // hdr.th_off = 5;
    // hdr.th_win = WINDOW_SIZE;
    // hdr.th_seq = ctx->initial_sequence_num;
    // hdr2.th_ack = htonl(ack);
    // ctx->snd_seq = ctx->initial_sequence_num + 1;
    stcp_network_send(sd, &hdr, sizeof(STCPHeader), NULL);
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
  srand(time(NULL));
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
