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

#define MSS 536

// NOTE: all functions in transport.c provide an arg for context_t

// TODO:
// what are the interfaces for working with the packets?
// Maybe I just type cast them? what are the relevant types then???
//

// Relevant API fucntions:
//

// Don't forget! -> Use for any relevant multibyte fields of the stcp header
// htonl, ntohl
// htons, ntohs

// STCP_API datatypes
// stcp_event_type_t
//
// transport.h dataypes
// tcphdr

// https://en.wikipedia.org/wiki/File:Tcp_state_diagram_fixed.svg
// according to docs we don't have to support TIME WAIT
enum fsm_state {
  HANDSHAKING, // handshaking as a state is referenced in the project slides,
               // was it a literal state???
  ClOSED,
  SYN_SENT,
  SYN_RECEIVED,
  LISTEN,
  CLOSE_WAIT,
  LAST_ACK,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSING,
  CSTATE_ESTABLISHED,
}; /* you should have more states */
// TODO: How do we handle undefined state transitions?

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;

  /* any other connection-wide global variables go here */
  char rcv_wndw[3072];
  int rcv_wndw_f_ptr;
  int rcv_wndw_b_ptr;
  int rcv_seq_num;

  char snd_wndw[3072];
  int snd_wndw_f_ptr;
  int snd_wndw_b_ptr;
  int snd_seq_num;

  // TODO: Add: send + recv window buffers (circular)?, ack #, other things??
} context_t;

static void generate_initial_seq_num(context_t *ctx);
void our_dprintf(const char *format, ...);
static void control_loop(mysocket_t sd, context_t *ctx);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
  context_t *ctx;

  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);
  our_dprintf("test\n");

  /* XXX: you should send a SYN packet here if is_active, or wait for one
   * to arrive if !is_active.  after the handshake completes, unblock the
   * application with stcp_unblock_application(sd).  you may also use
   * this to communicate an error condition back to the application, e.g.
   * if connection fails; to do so, just set errno appropriately (e.g. to
   * ECONNREFUSED, etc.) before calling the function.
   */

  if (is_active) {
    printf("in is active\n");
    generate_initial_seq_num(ctx);
    STCPHeader hdr;
    hdr.th_flags = TH_SYN;
    hdr.th_seq = ctx->snd_seq_num;
    hdr.th_off = 5;

    hdr.th_win = 3072;

    printf("The size of the hdr is %d\n", (int)sizeof(hdr));

    stcp_network_send(sd, &hdr, sizeof(STCPHeader));
    // stcp_network_send(sd, &hdr, sizeof(hdr));
    printf("sending syn packet");

    // TODO: this is garunteed to circularly wrap right? write may go out of
    // bounds...
    printf("listening for the synack\n");
    struct tcphdr synack_hdr;
    int rcv_len = stcp_network_recv(sd, &synack_hdr, sizeof(synack_hdr));
    if (rcv_len == -1) {
      printf("recv error in initalization\n");
      exit(1);
    }

    // stcp_network_send(mysocket_t sd, const void *src, size_t src_len, ...);
  } else {
    our_dprintf("on not active side\n");
    STCPHeader hdr;
    if (stcp_network_recv(sd, &hdr, sizeof(STCPHeader))) {
      printf("error in else");
      exit(1);
    }
    printf("sucsessfully read in data\n");
    int syn_flag = hdr.th_flags;
    assert(syn_flag == TH_SYN);
    int sndr_seq_num = hdr.th_seq;
    int sndr_win_len = hdr.th_win;
    printf("data received\n");
    /*
        generate_initial_seq_num(ctx);
        struct tcphdr hdr;
        hdr.th_flags = TH_SYN | TH_ACK;
        hdr.th_seq = ctx->snd_seq_num;
        hdr.th_win = 3072;
        */
  }

  ctx->connection_state = CSTATE_ESTABLISHED;
  stcp_unblock_application(sd);

  control_loop(sd, ctx);

  /* do any cleanup here */
  free(ctx);
}

/* generate random initial sequence number for an STCP connection */
static void generate_initial_seq_num(context_t *ctx) {
  assert(ctx);

#ifdef FIXED_INITNUM /* please don't change this! */
  ctx->initial_sequence_num = 1;
#else
  /* you have to fill this up */
  /*ctx->initial_sequence_num =;*/
  srand(time(NULL));
  ctx->snd_seq_num = rand() % 256;
  printf("Random number generated: %d\n", ctx->snd_seq_num);
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
    if (event & TIMEOUT) {
    } else if (event & APP_DATA) {
      /* the application has requested that data be sent */
      /* see stcp_app_recv() */
    } else if (event & NETWORK_DATA) {
    } else if (event & APP_CLOSE_REQUESTED) {
    } else if (event & ANY_EVENT) {
      printf("at anyevent\n");
    } else {
      printf("error in crtl loop\n");
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
