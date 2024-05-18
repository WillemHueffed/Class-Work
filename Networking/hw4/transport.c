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

// Used to print name of enum state
const char *e_names[] = {
    "HANDSHAKING", "CSTATE_ESTABLISHED", "FIN_WAIT_1", "FIN_WAIT_2",
    "CLOSING",     "CLOSE_WAIT",         "LAST_ACK",   "CLOSED"};

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

  // helper buffer for "simulating" our circular buffers
  char win_buf[WINDOW_SIZE];

  int is_active;
  mysocket_t sd;
} context_t;

static void generate_initial_seq_num(context_t *ctx);
static void control_loop(mysocket_t sd, context_t *ctx);
void push_to_cbuff(char val, cbuff *buff);
char deq_from_cbuff(cbuff *buff);
void my_send(context_t *ctx, uint8_t flags, char *snd_buff, uint buff_len);
void my_inc_ack(context_t *ctx, uint8_t flags, int byte_len);

/* initialise the transport layer, and start the main loop, handling
 * any data from the peer or the application.  this function should not
 * return until the connection is closed.
 */
void transport_init(mysocket_t sd, bool_t is_active) {
#if DEBUG
  printf("size of stcp header is %d\n", (int)sizeof(STCPHeader));
#endif

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
  ctx->is_active = is_active; // used to introduce noise into rng process
  // store header in ctx to make memory managment easier
  ctx->hdr = (STCPHeader *)calloc(1, sizeof(STCPHeader));
  STCPHeader *hdr = ctx->hdr; // alias header
  memset(hdr, 0, sizeof(STCPHeader));

  generate_initial_seq_num(ctx);
#if DEBUG
  printf("initial_sequence_num is %d\n", ctx->initial_sequence_num);
#endif
  ctx->seq_num = ctx->initial_sequence_num;

  if (is_active) {
    // send SYN
    my_send(ctx, TH_SYN, NULL, 0);
#if DEBUG
    printf("sending syn, current sn: %d, ack: %d\n", ctx->seq_num,
           ctx->ack_num);
#endif

    // receive SYNACK
    memset(hdr, 0, sizeof(STCPHeader));
    uint8_t rcv_len = stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    if (rcv_len < 20) {
      errno = EIO; // Not really sure what the appropriate errno value should be
                   // - this seems generally appropriate
      exit(1);
    }
    if (rcv_len < 20) {
      errno = EIO; // Not really sure what the appropriate errno value should be
                   // - this seems generally appropriate
      exit(1);
    }
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    ctx->ack_num = ntohl(hdr->th_seq); // This gets inc in my_inc_ack just have
                                       // to initialize it here
    my_inc_ack(ctx, hdr->th_flags, rcv_len);
    ctx->rcvr_wndw = ntohs(hdr->th_win);

    // send ACK
    my_send(ctx, TH_ACK, NULL, 0);
#if DEBUG
    printf("client side handshake finsished, current sn: %d, ack: %d, rcvr "
           "window: %d\n",
           ctx->seq_num, ctx->ack_num, ctx->rcvr_wndw);
#endif
  } else {
    // receive SYN
    uint8_t rcv_len = stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    if (rcv_len < 20) {
      errno = EIO; // Not really sure what the appropriate errno value should be
                   // - this seems generally appropriate
      exit(1);
    }
    if (rcv_len < 20) {
      errno = EIO; // Not really sure what the appropriate errno value should be
                   // - this seems generally appropriate
      exit(1);
    }
    assert((hdr->th_flags & TH_SYN));
    ctx->rcvr_wndw = ntohs(hdr->th_win);
    ctx->ack_num = ntohl(hdr->th_seq); // This gets inc in my_inc_ack just have
                                       // to initialize it here
    my_inc_ack(ctx, hdr->th_flags, rcv_len);

    // send SYN-ACK
    my_send(ctx, (TH_SYN | TH_ACK), NULL, 0);

    // receive ACK
    memset(hdr, 0, sizeof(STCPHeader));
    stcp_network_recv(sd, hdr, sizeof(STCPHeader));
    if (rcv_len < 20) {
      errno = EIO; // Not really sure what the appropriate errno value should be
                   // - this seems generally appropriate
      exit(1);
    }
    assert(ntohl(hdr->th_seq) == ctx->ack_num);
    assert(ntohl(hdr->th_ack) == ctx->seq_num);
    assert(hdr->th_flags & TH_ACK);
    my_inc_ack(ctx, hdr->th_flags, rcv_len);
    ctx->rcvr_wndw = ntohs(hdr->th_win);
#if DEBUG
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
  free(ctx->hdr);
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
  if (ctx->is_active) {
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
// ^^^ document says underlying connection is reliable so we don't have to
// implement timeout functionality... inconsistent, frustrating, and confusing

static void control_loop(mysocket_t sd, context_t *ctx) {
  assert(ctx);
  assert(!ctx->done);

  while (!ctx->done) {
    unsigned int event;

    // Any event doesn't contain timeout mask
    event = stcp_wait_for_event(sd, ANY_EVENT, NULL);

    /* check whether it was the network, app, or a close request */
    if (event & APP_DATA) {
      if (ctx->connection_state == CSTATE_ESTABLISHED) {

        int rcv_len = stcp_app_recv(sd, ctx->win_buf, ctx->rcvr_wndw);
        for (int i = 0; rcv_len; rcv_len--, i++) {
          push_to_cbuff(ctx->win_buf[i], &ctx->snd_buff);
        }
#if DEBUG
        printf("the snd buff has %d bytes free\n", ctx->snd_buff.free_bytes);
#endif
        while (ctx->snd_buff.free_bytes != WINDOW_SIZE) {
          int b_in_send_buff = WINDOW_SIZE - ctx->snd_buff.free_bytes;
          if (b_in_send_buff > STCP_MSS) {
            // send full packet
            memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
            for (int i = 0; i < STCP_MSS; i++) {
              ctx->tmp_buf[i] = deq_from_cbuff(&ctx->snd_buff);
            }
            my_send(ctx, 0, ctx->tmp_buf, STCP_MSS);
            memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
          } else {
            // send small packet
            memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
            for (int i = 0; i < b_in_send_buff; i++) {
              ctx->tmp_buf[i] = deq_from_cbuff(&ctx->snd_buff);
            }
            my_send(ctx, 0, ctx->tmp_buf, b_in_send_buff);
            memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
          }
        }
      }
    }
    if (event & NETWORK_DATA) {
#if DEBUG
      printf("start network recv - sn: %d | ack: %d\n", ctx->seq_num,
             ctx->ack_num);
#endif
      assert(ctx->hdr);

      memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
      int rcv_len = stcp_network_recv(sd, ctx->tmp_buf, sizeof(ctx->tmp_buf));
      if (rcv_len < 20) {
        errno =
            EIO; // Not really sure what the appropriate errno value should be
                 // - this seems generally appropriate
        exit(1);
      }
      assert(rcv_len <= ctx->rcv_buff.free_bytes);
      assert(rcv_len >= 20);

      STCPHeader *hdr = ctx->hdr;
      memset(hdr, 0, sizeof(STCPHeader));
      memcpy(hdr, ctx->tmp_buf, sizeof(STCPHeader));
#if DEBUG
      printf("advertized window size is: %d\n", ntohs(hdr->th_win));
#endif
      assert(ctx->hdr);
      // th_win could technically be zero if buffer is full -> unlikely ->
      // sanity check to ensure I'm writing the field
      assert(hdr->th_win > 0);
      ctx->rcvr_wndw = ntohs(hdr->th_win);
      // assert(ntohl(hdr->th_seq) == ctx->ack_num);
      my_inc_ack(ctx, hdr->th_flags, rcv_len);
#if DEBUG
      printf("updated ack_num to: %d\n", ctx->ack_num);
      printf("recieved ack %d | my sn is: %d\n", ntohl(hdr->th_ack),
             ctx->seq_num);
#endif

      // Push data into cbuff
      assert(rcv_len - sizeof(STCPHeader) <= ctx->rcv_buff.free_bytes);
      for (int i = sizeof(STCPHeader); i < rcv_len; i++) {
        push_to_cbuff(ctx->tmp_buf[i], &ctx->rcv_buff);
      }
      // printf("cbuff contains: %s\n", ctx->rcv_buff.wndw);

      // This assert will fail, server sends 2 packets gets ack for packet 1
      // while server side sn is updated to packet 2 assert(ntohl(hdr->th_ack)
      // == ctx->seq_num); We don't have to implement retransmit but I think
      // assert(ntohl(hdr->th_ack) <= ctx->seq_num) is more appropriate?

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
        if (!(hdr->th_flags == TH_ACK)) {
#if DEBUG
          printf("sending ack with ack_num: %d\n", ctx->ack_num);
#endif
          my_send(ctx, TH_ACK, NULL, 0);
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
      // printf("end network recv - sn: %d | ack: %d\n\n", ctx->seq_num,
      // ctx->ack_num);
      // printf("pushing data from rcvr buffer up to app layer...\n");
      // printf("\ncbuff is currently: %s\n", ctx->rcv_buff.wndw);
      int i = 0;
      for (; ctx->rcv_buff.free_bytes != WINDOW_SIZE; i++) {
        ctx->win_buf[i] = deq_from_cbuff(&ctx->rcv_buff);
      }
      // printf("free bytes: %d\n", ctx->rcv_buff.free_bytes);
      stcp_app_send(sd, ctx->win_buf, i);
      memset(ctx->win_buf, 0, sizeof(ctx->win_buf));
      memset(ctx->tmp_buf, 0, sizeof(ctx->tmp_buf));
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

void push_to_cbuff(char val, cbuff *buff) {
  int *front = &(buff->ptr_f);
  int *rear = &(buff->ptr_b);
  char *cbuff = &(buff->wndw[0]);
  if ((*front == 0 && *rear == WINDOW_SIZE - 1) ||
      ((*rear + 1) % WINDOW_SIZE == *front)) {
    perror("buffer overflow");
    exit(1);
  } else if (*front == -1) {
    *front = *rear = 0;
    cbuff[*rear] = val;
  } else if (*rear == WINDOW_SIZE - 1 && *front != 0) {
    *rear = 0;
    cbuff[*rear] = val;
  } else {
    (*rear)++;
    cbuff[*rear] = val;
  }
  buff->free_bytes--;
}

char deq_from_cbuff(cbuff *buff) {
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

void my_send(context_t *ctx, uint8_t flags, char *snd_buff, uint buff_len) {
  STCPHeader *hdr = ctx->hdr;
  assert(ctx->hdr);
  memset(hdr, 0, sizeof(STCPHeader));
  hdr->th_flags |= flags;
  hdr->th_off = 5;
  hdr->th_seq = htonl(ctx->seq_num);
  hdr->th_ack = htonl(ctx->ack_num);
  hdr->th_win = htons(ctx->rcv_buff.free_bytes);
  // TODO: we maybe shouldn't be incrementing by +1 for a sequence number
  if (flags == TH_ACK) {
#if DEBUG
    printf("sending ack\n");
#endif
  } else if ((flags & TH_SYN) || (flags & TH_ACK) ||
             (flags & (TH_SYN | TH_ACK)) || (flags & (TH_FIN)) ||
             (flags & (TH_FIN | TH_ACK))) {
    ctx->seq_num += 1;
  } else {
#if DEBUG
    printf("sending data packet of len %d\n", buff_len);
#endif
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

void my_inc_ack(context_t *ctx, uint8_t flags, int byte_len) {
  if (flags == TH_ACK) { // doing (flags & TH_ACK) catches SYN-ACK pkts too...
                         // this doesnt...
#if DEBUG
    printf("this is an ack pkt, not incing syn\n");
#endif
  } else if (flags) { // this should be more specific...
#if DEBUG
    printf("conn start/teardown pkt detected -> sn++\n");
#endif
    ctx->ack_num++;
  } else {
    uint offset = byte_len - sizeof(STCPHeader);
#if DEBUG
    printf("increasing sn by %d\n", offset);
#endif
    ctx->ack_num += offset;
  }
}
