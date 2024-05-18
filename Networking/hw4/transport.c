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

#define DEBUG 0
#define DEBUG2 2

enum {
  HANDSHAKING,
  CSTATE_ESTABLISHED,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSING,
  CLOSE_WAIT,
  LAST_ACK,
  CLOSED,
};

const char *e_names[] = {
    "HANDSHAKING", "CSTATE_ESTABLISHED", "FIN_WAIT_1", "FIN_WAIT_2",
    "CLOSING",     "CLOSE_WAIT",         "LAST_ACK",   "CLOSED"};

int active = 0;

#define WINDOW_SIZE 3072

typedef struct {
  char wndw[WINDOW_SIZE];
  int free_bytes;
  int ptr_f;
  int ptr_b;
} cbuff;

/* this structure is global to a mysocket descriptor */
typedef struct {
  bool_t done; /* TRUE once connection is closed */

  int connection_state; /* state of the connection (established, etc.) */
  tcp_seq initial_sequence_num;

  cbuff snd_buff;
  cbuff rcv_buff;

  uint seq_num;
  uint ack_num;
  uint rcvr_wndw;

  // Store here to make memory managment easier
  char tmp_buf[STCP_MSS + 20];
  STCPHeader *hdr;

  /* any other connection-wide global variables go here */
  mysocket_t sd;
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);
void _push_to_cbuff(char val, cbuff *buff);
char _deq_from_cbuff(cbuff *buff);
int push_n_to_cbuff(char *src, int len, cbuff *cbuff);
void my_send(context_t *ctx, uint8_t flags, char *snd_buff, uint buff_len);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
#if DEBUG
  printf("size of stcp header is %d\n", (int)sizeof(STCPHeader));
#endif

  active = is_active; // used to introduce noise into rng process
  context_t *ctx;

  // Initialise context
  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);
  ctx->snd_buff.ptr_f = -1;
  ctx->snd_buff.ptr_b = -1;
  ctx->snd_buff.free_bytes = WINDOW_SIZE;
  ctx->rcv_buff.ptr_f = -1;
  ctx->rcv_buff.ptr_b = -1;
  ctx->rcv_buff.free_bytes = WINDOW_SIZE;
  ctx->sd = sd;
  // store header in ctx to make memory managment easier
  ctx->hdr = (STCPHeader *)calloc(1, sizeof(STCPHeader));
  STCPHeader *hdr = ctx->hdr; // alias header
  memset(hdr, 0, sizeof(STCPHeader));

  generate_initial_seq_num(ctx);
  printf("initial_sequence_num is %d\n", ctx->initial_sequence_num);
  ctx->seq_num = ctx->initial_sequence_num;

  if (is_active) {
    // send SYN
    my_send(ctx, TH_SYN, NULL, 0);

    // receive SYNACK
    memset(hdr, 0, sizeof(STCPHeader));
    if (stcp_network_recv(sd, hdr, sizeof(STCPHeader)) != 20) {
      perror("error recv\n");
    }
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    ctx->ack_num = ntohl(hdr->th_seq) + 1; // rcv_seq is our ack num
    ctx->rcvr_wndw = ntohs(hdr->th_win);

    // send ACK
    my_send(ctx, TH_ACK, NULL, 0);
#if DEBUG || DEBUG2
    printf("client side handshake finsished, current sn: %d, ack: %d, rcvr "
           "window: %d\n",
           ctx->seq_num, ctx->ack_num, ctx->rcvr_wndw);
#endif
  } else {
    // receive SYN
    if (stcp_network_recv(sd, hdr, sizeof(STCPHeader)) == -1) {
      perror("error recv\n");
      return;
    }
    ctx->ack_num = ntohl(hdr->th_seq) + 1;
    ctx->rcvr_wndw = ntohs(hdr->th_win);
    assert((hdr->th_flags & TH_SYN));

    // send SYN-ACK
    my_send(ctx, (TH_SYN | TH_ACK), NULL, 0);

    // receive ACK
    memset(hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    assert(ntohl(hdr->th_seq) == ctx->ack_num);
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    assert(hdr->th_flags & TH_ACK);
    ctx->ack_num = ntohl(hdr->th_seq) + 1;
    ctx->rcvr_wndw = ntohs(hdr->th_win);
#if DEBUG || DEBUG2
    printf("server side handshake finsished, current sn: %d, ack: %d, rcvr win "
           "size: %d\n",
           ctx->seq_num, ctx->ack_num, ctx->rcvr_wndw);
#endif
  }

  ctx->connection_state = CSTATE_ESTABLISHED;
  stcp_unblock_application(sd);

  control_loop(sd, ctx);
  assert(ctx);
  return;

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
    event = stcp_wait_for_event(sd, ANY_EVENT, NULL);

    /* check whether it was the network, app, or a close request */
    if (event & APP_DATA) {
      if (ctx->connection_state == CSTATE_ESTABLISHED) {

        int rcv_len = stcp_app_recv(sd, ctx->tmp_buf, STCP_MSS);
        my_send(ctx, 0, ctx->tmp_buf, rcv_len);
      }
    }
    if (event & NETWORK_DATA) {
      assert(ctx->hdr);

      memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
      int rcv_len = stcp_network_recv(sd, ctx->tmp_buf, sizeof(ctx->tmp_buf));
      assert(rcv_len <= ctx->rcv_buff.free_bytes);
      assert(rcv_len >= 20);

      STCPHeader *hdr = ctx->hdr;
      memset(hdr, 0, sizeof(STCPHeader));
      memcpy(hdr, ctx->tmp_buf, sizeof(STCPHeader));
      assert(ctx->hdr);
      // assert(ntohl(hdr->th_seq) == ctx->ack_num);
      ctx->ack_num += rcv_len - sizeof(STCPHeader);
      printf("updated ack_num to: %d\n", ctx->ack_num);
      assert(ntohl(hdr->th_ack) == ctx->seq_num);
      stcp_app_send(sd, ctx->tmp_buf + sizeof(STCPHeader),
                    rcv_len - sizeof(STCPHeader));
      memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
      assert(ctx->hdr);

      if (ctx->connection_state == CSTATE_ESTABLISHED) {
        // If FIN flag is set
        if (hdr->th_flags & TH_FIN) {
          // transition state
          ctx->connection_state = CLOSE_WAIT;
          // ACK the FIN;
          my_send(ctx, TH_ACK, NULL, 0);
          stcp_fin_received(sd);
          assert(ctx->hdr);
        }
      } else if (ctx->connection_state == FIN_WAIT_1) {
        // goto FIN wait 2
        if (hdr->th_flags & TH_ACK) {
          ctx->connection_state = FIN_WAIT_2;
          stcp_fin_received(sd);
          assert(ctx->hdr);
        }
        // simul close
        else if (hdr->th_flags & TH_FIN) {
          ctx->connection_state = CLOSING;
          my_send(ctx, TH_ACK, NULL, 0);
          stcp_fin_received(sd);
          assert(ctx->hdr);
        }
      } else if (ctx->connection_state == FIN_WAIT_2) {
        if (hdr->th_flags & TH_FIN) {
          my_send(ctx, TH_ACK, NULL, 0);
          ctx->connection_state = CLOSED;
          ctx->done = TRUE;
          assert(ctx->hdr);
        }
      } else if (ctx->connection_state == CLOSE_WAIT) {
        // TODO: return an error, this 1/2 of the conn should be closed
        // i.e. there should be no incoming data
        assert(0);
      } else if (ctx->connection_state == LAST_ACK) {
        if (hdr->th_flags & TH_ACK) {
          ctx->connection_state = CLOSED;
          ctx->done = TRUE;
        }
      } else if (ctx->connection_state == CLOSING) {
        if (hdr->th_flags & TH_ACK) {
          ctx->done = TRUE;
        } else {
          perror("undefined state transition, state closing recieved input "
                 "other than an ACK");
          assert(0);
        }
      }
    }
    if (event & APP_CLOSE_REQUESTED) {
      if ((ctx->connection_state == CSTATE_ESTABLISHED) ||
          ctx->connection_state == CLOSE_WAIT) {
        // Send FIN
        assert(ctx);
        assert(ctx->hdr);
        my_send(ctx, TH_FIN, NULL, 0);
        ctx->connection_state = (ctx->connection_state == CSTATE_ESTABLISHED)
                                    ? FIN_WAIT_1
                                    : LAST_ACK;
      } else {
        assert(0);
      }
    }
  }
  assert(ctx);
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

int get_cbuff_free_len(int start, int end, int size) {
  // ripped from
  // https://stackoverflow.com/questions/450558/simplified-algorithm-for-calculating-remaining-space-in-a-circular-buffer
  assert(start >= 0 && end >= 0 && size > 0);
  int remaining = (end - start) + (-((int)(end <= start)) & size);
  return remaining;
}

void _push_to_cbuff(char val, cbuff *buff) {
  int *front = &(buff->ptr_f);
  int *rear = &(buff->ptr_b);
  char *cbuff = &(buff->wndw[0]);
  if ((*front == 0 && *rear == WINDOW_SIZE - 1) ||
      ((*rear + 1) % WINDOW_SIZE == *front)) {
    perror("buffer overflow");
    exit(1);
  } else if (*rear == WINDOW_SIZE - 1 && *front != 0) {
    *rear = 0;
    cbuff[*rear] = val;
  } else {
    (*rear)++;
    cbuff[*rear] = val;
  }
  buff->free_bytes--;
}

char _deq_from_cbuff(cbuff *buff) {
  int *front = &(buff->ptr_f);
  int *rear = &(buff->ptr_b);
  char *cbuff = &(buff->wndw[0]);
  if (*front == -1) {
    perror("cbuff is empty");
    exit(1);
  }
  char val = cbuff[*front];
  cbuff[*front] = 'b'; // considering using '\0' but don't want that mess
                       // stuff up if I need to print the buffer
  if (*front == *rear) {
    *front = -1;
    *rear = -1;
  } else if (*front == WINDOW_SIZE - 1) {
    *front = 0;
  } else {
    (*front)++;
  }
  buff->free_bytes++;
  return val;
}

int push_n_to_cbuff(char *src, int len, cbuff *cbuff) {
  if (cbuff->free_bytes < len) {
    perror("attempted to write too many bytes to buffer");
    return (1);
  }
  for (int i = 0; len > 0; i++) {
    _push_to_cbuff(src[i++], cbuff);
  }
  // silence compiler
  return 1;
}

void my_send(context_t *ctx, uint8_t flags, char *snd_buff, uint buff_len) {
  STCPHeader *hdr = ctx->hdr;
  assert(ctx->hdr);
  memset(hdr, 0, sizeof(STCPHeader));
  hdr->th_flags |= flags;
  hdr->th_off = 5;
  hdr->th_seq = htonl(ctx->seq_num);
  hdr->th_ack = htonl(ctx->ack_num);
  hdr->th_win = htons(WINDOW_SIZE);
  // TODO: we maybe shouldn't be incrementing by +1 for a sequence number
  if ((flags & TH_SYN) || (flags & TH_ACK) || (flags & (TH_SYN | TH_ACK)) ||
      (flags & (TH_FIN)) || (flags & (TH_FIN | TH_ACK))) {
    ctx->seq_num += 1;
  } else {
    ctx->seq_num += buff_len;
  }
  if (snd_buff && (buff_len <= STCP_MSS)) {
    assert(buff_len <= ctx->rcvr_wndw);
    if (stcp_network_send(ctx->sd, hdr, sizeof(STCPHeader), snd_buff, buff_len,
                          NULL) == -1) {
      perror("err snd\n");
      return;
    }
  } else {
    assert(buff_len <= ctx->rcvr_wndw);
    if (stcp_network_send(ctx->sd, hdr, sizeof(STCPHeader), NULL) == -1) {
      perror("err snd\n");
      return;
    }
  }
  assert(ctx->hdr);
}
