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

enum { CSTATE_ESTABLISHED }; /* you should have more states */

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;
  tcp_seq send_seq_num;
  tcp_seq recv_seq_num;

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

  if (is_active) {
    STCPHeader SYNpacket;
    SYNpacket.th_seq = ctx->initial_sequence_num;
    ctx->send_seq_num = ctx->initial_sequence_num;
    SYNpacket.th_off = 5;
    SYNpacket.th_flags = TH_SYN;
    SYNpacket.th_win = 3072;
    printf("err1");
    int status = stcp_network_send(sd, &SYNpacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("send");
    }
    ctx->send_seq_num++;

    STCPHeader activeRcvPacket;
    printf("err2");
    status = stcp_network_recv(sd, &activeRcvPacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("recv");
    }
    // check for if SYN
    ctx->recv_seq_num = activeRcvPacket.th_seq++;

    STCPHeader ACKpacket;
    ACKpacket.th_seq = ctx->send_seq_num;
    ACKpacket.th_ack = ctx->recv_seq_num;
    ctx->send_seq_num++;
    ACKpacket.th_off = 5;
    ACKpacket.th_flags = TH_ACK;
    ACKpacket.th_win = 3072;
    printf("err3");
    status = stcp_network_send(sd, &ACKpacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("send");
    }
  } else {
    STCPHeader passiveRcvPacket;
    printf("err4");
    int status = stcp_network_recv(sd, &passiveRcvPacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("recv");
    }
    ctx->recv_seq_num = passiveRcvPacket.th_seq++;
    // check if syn maybe

    STCPHeader SYN_ACKpacket;
    SYN_ACKpacket.th_seq = ctx->initial_sequence_num;
    ctx->send_seq_num = ctx->initial_sequence_num;
    SYN_ACKpacket.th_off = 5;
    SYN_ACKpacket.th_flags = TH_SYN | TH_ACK;
    SYN_ACKpacket.th_win = 3072;
    printf("err5");
    status = stcp_network_send(sd, &SYN_ACKpacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("send");
    }
    ctx->send_seq_num++;

    STCPHeader finalACKpacket;
    printf("err6");
    status = stcp_network_recv(sd, &finalACKpacket, sizeof(STCPHeader));
    if (status == -1) {
      perror("recv");
    }
    ctx->recv_seq_num = finalACKpacket.th_seq++;
    printf("end of ack");
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
  int seq_num = rand() % 255;
  ctx->initial_sequence_num = seq_num;
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
