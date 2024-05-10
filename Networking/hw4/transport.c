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
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIN_SIZE 3072

enum { CSTATE_ESTABLISHED }; /* you should have more states */

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;

  /* any other connection-wide global variables go here */

  char snd_wndw_buf[WIN_SIZE];
  uint snd_wndw_f_ptr;
  uint snd_wndw_b_ptr;
  uint snd_seq_num;

  char rcv_wndw_buf[WIN_SIZE];
  uint rcv_wndw_f_ptr;
  uint rcv_wndws_b_ptr;
  uint rcv_seq_num;

} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);
void our_dprintf(const char *format, ...);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
  context_t *ctx;

  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);

  generate_initial_seq_num(ctx);

  if (is_active == TRUE) {
    printf("in is active\n");
    STCPHeader hdr;
    memset(&hdr, 0, sizeof(STCPHeader));

    hdr.th_off = htonl(5);
    hdr.th_seq = htonl(ctx->initial_sequence_num);
    printf("the initial sequence number is: %d\n", ctx->initial_sequence_num);
    // hdr->th_ack = htonl(1);
    hdr.th_flags |= htons(TH_SYN);
    hdr.th_win = htonl(WIN_SIZE);
    ctx->snd_seq_num = ctx->initial_sequence_num++;
    our_dprintf("sender SYN: seq num %" PRIu32 "\n", ntohl(hdr.th_seq));
    stcp_network_send(sd, &hdr, sizeof(STCPHeader), NULL);

    // Wait for SYN-ACK
    memset(&hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, &hdr, sizeof(STCPHeader));
    hdr.th_off = ntohl(hdr.th_off);
    hdr.th_seq = ntohl(hdr.th_seq);
    hdr.th_ack = ntohl(hdr.th_ack);
    hdr.th_flags |= ntohl(hdr.th_flags);
    hdr.th_win = ntohl(hdr.th_win);
    our_dprintf("sender received SYN-ACK: seq num: %d, ack num: %d\n",
                hdr.th_seq, hdr.th_ack);

    assert(ntohs(hdr.th_flags) & (TH_SYN | TH_ACK));
    assert(ctx->snd_seq_num == hdr.th_ack - 1);
    ctx->rcv_seq_num = ntohl(hdr.th_seq);

    // Send the ACK
    memset(&hdr, 0, sizeof(STCPHeader));
    hdr.th_off = htonl(5);
    hdr.th_seq = htonl(ctx->snd_seq_num++);
    hdr.th_flags |= htons(TH_ACK);
    hdr.th_win = htonl(WIN_SIZE);
    hdr.th_ack = htonl(ctx->rcv_seq_num + 1);
    stcp_network_send(sd, &hdr, sizeof(STCPHeader), NULL);
    our_dprintf("sender sending ACK: seq num: %d, ack num: %d\n", hdr.th_seq,
                hdr.th_ack);
    // Should be safe to move state to ESTABLISHED
  } else {
    // How do I get evidence that this else statement runs???
    char *ptr = NULL;
    free(ptr);
    free(ptr);
    assert(0);
    FILE *log_file;
    char *log_filename = "logfile.txt";

    // Open the log file in write mode ("w")
    log_file = fopen(log_filename, "w");

    // Write to the log file using fprintf()
    fprintf(log_file, "This is a log message.\n");
    fprintf(log_file, "Another log message.\n");

    // Close the log file
    fclose(log_file);
    // get the SYN
    // STCPHeader *hdr = (STCPHeader *)calloc(1, sizeof(STCPHeader));
    our_dprintf("in receiver side\n");
    STCPHeader hdr;
    memset(&hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, &hdr, sizeof(STCPHeader));
    hdr.th_off = ntohl(hdr.th_off);
    hdr.th_seq = ntohl(hdr.th_seq);
    hdr.th_ack = ntohl(hdr.th_ack);
    hdr.th_flags |= ntohl(hdr.th_flags);
    hdr.th_win = ntohl(hdr.th_win);
    assert(hdr.th_flags & TH_SYN);
    our_dprintf("recvr received SYN: seq num: %d, ack num: %d\n", hdr.th_seq,
                hdr.th_ack);

    // Send the SYN-ACK
    ctx->rcv_seq_num = hdr.th_seq + 1;
    memset(&hdr, 0, sizeof(STCPHeader));
    hdr.th_off = htonl(5);
    hdr.th_seq = htonl(ctx->initial_sequence_num);
    hdr.th_flags |= htons(TH_SYN | TH_ACK);
    hdr.th_win = htonl(WIN_SIZE);
    hdr.th_ack = htonl(ctx->rcv_seq_num);
    stcp_network_send(sd, &hdr, sizeof(STCPHeader), NULL);
    ctx->snd_seq_num = ctx->initial_sequence_num + 1;
    our_dprintf("recvr sending SYN-ACK: seq num: %d, ack num: %d\n", hdr.th_seq,
                hdr.th_ack);

    // Receive the ACK
    memset(&hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, &hdr, sizeof(STCPHeader));
    hdr.th_off = ntohl(hdr.th_off);
    hdr.th_seq = ntohl(hdr.th_seq);
    hdr.th_ack = ntohl(hdr.th_ack);
    hdr.th_flags |= ntohl(hdr.th_flags);
    hdr.th_win = ntohl(hdr.th_win);
    our_dprintf("recvr received ACK: seq num: %d, ack num: %d\n", hdr.th_seq,
                hdr.th_ack);

    assert(hdr.th_ack == ctx->snd_seq_num + 1);
    assert(hdr.th_flags == TH_ACK);

    our_dprintf("finished passive side\n");
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
  /*ctx->initial_sequence_num =;*/
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

/*
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
*/

void our_dprintf(const char *format, ...) {
  va_list argptr;
  FILE *fp;

  assert(format);

  // Open the log file in append mode
  fp = fopen("logfile", "a");
  if (fp == NULL) {
    fprintf(stderr, "Error opening log file.\n");
    return;
  }

  va_start(argptr, format);
  vfprintf(fp, format, argptr); // Write to the log file
  va_end(argptr);

  fclose(fp); // Close the log file
}
