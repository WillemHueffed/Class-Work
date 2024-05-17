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
  HANDSHAKING,
  CSTATE_ESTABLISHED,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSING,
  CLOSE_WAIT,
  LAST_ACK,
  CLOSED,
}; /* you should have more states */

const char *e_names[] = {"HANDSHAKING",
                         "CSTATE_ESTABLISHED",
                         "FIN_WAIT_1",
                         "FIN_WAIT_2",
                         "CLOSING",
                         "CLOSE_WAIT",
                         "LAST_ACK",
                         "CLOSED"}; /* you should have more states */

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
  char tmp_buf[WINDOW_SIZE];
  STCPHeader *hdr;

  /* any other connection-wide global variables go here */
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);
void _push_to_cbuff(char val, cbuff *buff);
char _deq_from_cbuff(cbuff *buff);
int push_n_to_cbuff(char *src, int len, cbuff *cbuff);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
  active = is_active; // used to introduce noise into rng process
  context_t *ctx;
  ctx = (context_t *)calloc(1, sizeof(context_t));
  assert(ctx);
  ctx->snd_buff.ptr_f = -1;
  ctx->snd_buff.ptr_b = -1;
  ctx->snd_buff.free_bytes = WINDOW_SIZE;
  ctx->rcv_buff.ptr_f = -1;
  ctx->rcv_buff.ptr_b = -1;
  ctx->rcv_buff.free_bytes = WINDOW_SIZE;

  generate_initial_seq_num(ctx);

  // store header in ctx to make memory managment easier
  ctx->hdr = (STCPHeader *)calloc(1, sizeof(STCPHeader));
  STCPHeader *hdr = ctx->hdr; // alias header
  memset(hdr, 0, sizeof(STCPHeader));

  if (is_active) {
    // send SYN
    hdr->th_flags |= TH_SYN;
    hdr->th_off = 5;
    hdr->th_win = htons(WINDOW_SIZE);
    hdr->th_seq = htonl(ctx->initial_sequence_num);
    ctx->seq_num = ctx->initial_sequence_num + 1;
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
    ctx->ack_num = ntohl(hdr->th_seq) + 1; // rcv_seq is our ack num
    printf("received synack: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    ctx->rcvr_wndw = ntohs(hdr->th_win);

    // send ACK
    memset(hdr, 0, sizeof(STCPHeader));
    hdr->th_flags |= TH_ACK;
    hdr->th_off = 5;
    hdr->th_seq = htonl(ctx->seq_num);
    hdr->th_ack = htonl(ctx->ack_num);
    hdr->th_win = htons(WINDOW_SIZE);
    ctx->seq_num += 1;
    printf("sending ack sn: %d and ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    if (stcp_network_send(sd, hdr, sizeof(STCPHeader)) == -1) {
      printf("err snd\n");
      return;
    }
    printf("client side handshake finsished, current sn: %d, ack: %d, rcvr "
           "window: %d\n",
           ctx->seq_num, ctx->ack_num, ctx->rcvr_wndw);
  } else {
    // receive SYN
    if (stcp_network_recv(sd, hdr, sizeof(STCPHeader)) == -1) {
      printf("error recv\n");
      return;
    }
    printf("receiving syn: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    ctx->ack_num = ntohl(hdr->th_seq) + 1;
    ctx->rcvr_wndw = ntohs(hdr->th_win);
    assert((hdr->th_flags & TH_SYN));

    // send SYN-ACK
    ctx->seq_num = ctx->initial_sequence_num;
    memset(hdr, 0, sizeof(STCPHeader));
    hdr->th_flags |= TH_SYN | TH_ACK;
    hdr->th_off = 5;
    hdr->th_win = htons(WINDOW_SIZE);
    hdr->th_seq = htonl(ctx->seq_num);
    hdr->th_ack = htonl(ctx->ack_num);
    ctx->seq_num += 1;
    printf("sending synack with sn: %d and ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);

    // receive ACK
    memset(hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    printf("receiving ack: sn: %d, ack: %d\n", ntohl(hdr->th_seq),
           ntohl(hdr->th_ack));
    // printf("th_seq: %d | ack: %d\n", ntohl(hdr->th_seq), ctx->ack_num);
    assert(ntohl(hdr->th_seq) == ctx->ack_num);
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    assert(hdr->th_flags & TH_ACK);
    ctx->ack_num = ntohl(hdr->th_seq) + 1;
    ctx->rcvr_wndw = ntohl(hdr->th_win);
    printf("server side handshake finsished, current sn: %d, ack: %d, rcvr win "
           "size: %d\n",
           ctx->seq_num, ctx->ack_num, ctx->rcvr_wndw);
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
    printf("\ncurrent connection state is %s\n",
           e_names[ctx->connection_state]);

    /* see stcp_api.h or stcp_api.c for details of this function */
    /* XXX: you will need to change some of these arguments! */
    event = stcp_wait_for_event(sd, ANY_EVENT, NULL);

    /* check whether it was the network, app, or a close request */
    if (event & APP_DATA) {
      if (ctx->connection_state == CSTATE_ESTABLISHED) {

        printf("reading data from app\n");
        printf("free buff size: %d\n", ctx->snd_buff.free_bytes);
        printf("rcvr_wndw size: %d\n", ctx->rcvr_wndw);
        // account for header w/ -20
        int rcv_len = stcp_app_recv(sd, ctx->tmp_buf, STCP_MSS);
        printf("recieved %d bytes from app layer\n", rcv_len);
        printf("buffer is: %s", ctx->tmp_buf);

        // TODO: Add checks to ensure flow control
        memset(ctx->hdr, 0, sizeof(STCPHeader));
        STCPHeader *hdr = ctx->hdr;
        hdr->th_win = htons(ctx->snd_buff.free_bytes);
        hdr->th_seq = htonl(ctx->seq_num);
        hdr->th_ack = htonl(ctx->ack_num);
        hdr->th_off = 5;
        ctx->seq_num += 20 + rcv_len;
        assert((sizeof(STCPHeader) + rcv_len) <= ctx->rcvr_wndw);
        stcp_network_send(sd, hdr, sizeof(STCPHeader), ctx->tmp_buf, rcv_len,
                          NULL);
        printf("sucsessfully pushed data to network\n");
      }
    }
    if (event & NETWORK_DATA) {
      printf("reading data from network\n");
      printf("free buff size: %d\n", ctx->snd_buff.free_bytes);
      printf("rcvr_wndw size: %d\n", ctx->rcvr_wndw);

      memset(ctx->tmp_buf, 0, WINDOW_SIZE);
      int rcv_len = stcp_network_recv(sd, ctx->tmp_buf, WINDOW_SIZE);
      printf("passed rcv\n");
      assert(rcv_len >= 20);

      STCPHeader *hdr = ctx->hdr;
      memset(hdr, 0, sizeof(STCPHeader));
      // copy header over
      memcpy(hdr, ctx->tmp_buf, sizeof(STCPHeader));
      printf("seq num: %d | ack_num: %d\n", ntohl(hdr->th_seq), ctx->ack_num);
      assert(ntohl(hdr->th_seq) == ctx->ack_num);
      assert(ntohl(hdr->th_ack) == ctx->seq_num);
      ctx->ack_num = ntohl(hdr->th_seq) + rcv_len;
      printf("received data: %s\n", ctx->tmp_buf + 20);
      stcp_app_send(sd, ctx->tmp_buf + 20, rcv_len - 20);
      printf("pushed data up to app layer\n%s\n", ctx->tmp_buf + 20);
      memset(ctx->tmp_buf, 0, WINDOW_SIZE);

      if (ctx->connection_state == CSTATE_ESTABLISHED) {
        // If FIN flag is set
        if (hdr->th_flags & TH_FIN) {
          // transition state
          ctx->connection_state = CLOSE_WAIT;
          // ACK the FIN;
          memset(hdr, 0, sizeof(STCPHeader));
          hdr->th_flags |= TH_ACK;
          hdr->th_win = htons(ctx->snd_buff.free_bytes);
          hdr->th_seq = htonl(ctx->seq_num);
          hdr->th_ack = htonl(ctx->ack_num);
          hdr->th_off = 5;
          hdr->th_flags |= TH_ACK;
          ctx->seq_num += 1;
          assert(sizeof(STCPHeader) <= ctx->rcvr_wndw);
          stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);
          printf("sent ACK in response to FIN\n");
          stcp_fin_received(sd);
        }
      } else if (ctx->connection_state == FIN_WAIT_1) {
        // goto FIN wait 2
        if (hdr->th_flags & TH_ACK) {
          ctx->connection_state = FIN_WAIT_2;
          stcp_fin_received(sd);
        }
        // simul close
        else if (hdr->th_flags & TH_FIN) {
          ctx->connection_state = CLOSING;
          memset(hdr, 0, sizeof(STCPHeader));
          hdr->th_flags |= TH_ACK;
          hdr->th_win = htons(ctx->snd_buff.free_bytes);
          hdr->th_seq = htonl(ctx->seq_num);
          hdr->th_ack = htonl(ctx->ack_num);
          hdr->th_off = 5;
          hdr->th_flags |= TH_ACK;
          ctx->seq_num += 1;
          assert(sizeof(STCPHeader) <= ctx->rcvr_wndw);
          stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);
          printf("In state FIN_WAIT_1: sent ACK in response to FIN \n");
          stcp_fin_received(sd);
        }
      } else if (ctx->connection_state == FIN_WAIT_2) {
        printf("in FIN_WAIT_2 branch\n");
        if (hdr->th_flags & TH_FIN) {
          memset(hdr, 0, sizeof(STCPHeader));
          hdr->th_flags |= TH_ACK;
          hdr->th_win = htons(ctx->snd_buff.free_bytes);
          hdr->th_seq = htonl(ctx->seq_num);
          hdr->th_ack = htonl(ctx->ack_num);
          hdr->th_off = 5;
          hdr->th_flags |= TH_ACK;
          ctx->seq_num += 1;
          assert(sizeof(STCPHeader) <= ctx->rcvr_wndw);
          stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);
          printf("In state FIN_WAIT_2: sent ACK in response to FIN \n");
          ctx->connection_state = CLOSED;
          ctx->done = TRUE;
        }
      } else if (ctx->connection_state == CLOSE_WAIT) {
        // TODO: return an error, this 1/2 of the conn should be closed
        // i.e. there should be no incoming data
        assert(0);
      } else if (ctx->connection_state == LAST_ACK) {
        if (hdr->th_flags & TH_ACK) {
          printf("Received ACK, socket should be closed...\n");
          ctx->connection_state = CLOSED;
          ctx->done = TRUE;
        }
      }
    }
    if (event & APP_CLOSE_REQUESTED) {
      printf("app close requested\n");
      if ((ctx->connection_state == CSTATE_ESTABLISHED) ||
          ctx->connection_state == CLOSE_WAIT) {
        // Send FIN
        STCPHeader *hdr = ctx->hdr;
        memset(hdr, 0, sizeof(STCPHeader));
        hdr->th_win = htons(ctx->snd_buff.free_bytes);
        hdr->th_seq = htonl(ctx->seq_num);
        hdr->th_ack = htonl(ctx->ack_num);
        hdr->th_off = 5;
        hdr->th_flags |= TH_FIN;
        ctx->seq_num += 20;
        assert(sizeof(STCPHeader) <= ctx->rcvr_wndw);
        stcp_network_send(sd, hdr, sizeof(STCPHeader), NULL);
        printf("sent FIN\n");
        ctx->connection_state = (ctx->connection_state == CSTATE_ESTABLISHED)
                                    ? FIN_WAIT_1
                                    : LAST_ACK;
      } else {
        printf("attempting to close from a non-established connection");
        assert(0);
      }
    }
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
