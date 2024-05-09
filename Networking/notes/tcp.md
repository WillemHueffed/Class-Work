# Review
## TCP
- seq #: the offset of the first byte 
- timer: 1 timer -> with the lowest seq number
- events to trigger retransmission: timeouts & dup ack
- how to retransmit: only resend the one with the lowest seq number
- recv buffer: yes
- uses cumulative ack

## go back n
- seq #: the rank of the packet (which number it is)
- timer: 1-> the unaccepted packet with the lowest seq n
- events to trigger retransmission: timeout 
- how to retransmit: N 
- recv buffer: no 

## Selective Repeat
- seq #: the rank of the packet (which number it is)
- N timers
- events to trigger retransmission: timeout 
- how to retransmit: specific packet 
- recv buffer: yes


# TCP Lecture

## TCP Segments
- 3 events cause tcp segment transmission
 - segemnt not full
 - not full but timeout
 - application calls flush to force write

- MSS: maximum segment size
    - MSS determined by router (indirectly)

- The seq # != segment number
    - The seq # is random and determined via handshake
    - (security)

- TCP requires a setup and a tear-down step

- MTU = maximum transmission unit (determined by router)
    - MSS = MTU - some header stuff

## Duplex
- TCP is full duplex, meaning we can send and recv at the same time

## ARQ
- ARQ = Automatic Repeat reQuest
- simplest ARQ protocol: send message, stop and wait, get ack, repeat

## How long should TCP sender wait?
- RFC calls out 500ms, dated
- Realistically use RTT + slack/head room (account for queing/processing delay)
- Make wait time dynamic/adaptive (measure)
- "Exponential weighted moving average": RTT_n+1 = (alpha)*RTT + (1-alpha)RTT_measured; alpha = learning rate
- Deviation RTT = (alpha)*RTT + (1-alpha)*abs(sampleRTT - estimatedRTT)
- Timeout Interval = estimatedRTT 4 * DevRTT; I'm assuming the 4 comes from 4 stddevs accounting for 99.9% or whatever of area under bellcurve

## TCP fast retransmit
- If sender recvs a bunch of acks with an ack missing immedetly retransmit unacked segment.
- Likely the packet is lost
- Effectiveness:
    - Works well when: 
        - high liklihood of many packets in flight
        - Long data transfers, large window size

## Setup and Teardown
- Setup - 3 way handshake: SYN, SYN-ACK, ACK
    - Don't forget the ACK can't be sent back with data 
- Zhu wen't through the packet structure on the slides. Check them out to do the HW

## What if SYN packet gets lost?
- Eventually no SYN-ACK arrives
- Per RFC default timeout is 3 seconds can be as long as 6 in some implementations

## TCP teardown
- FIN message to close and recv remaining bytes
    - Occupies one byte in the sequence space
    - Other host ACKS the byte to confirm
- Closes A side of the connection but not B's
- Need to do another set of FIN ACK to close B's side

# TCP flow control
- The ACK message contains a field for the current free buffer size
- Sender should adjust accordingly or tail drop may occur
- If recvr responds with 0 bytes free, then sender will still 1 byte to get an updated ACK w/ hopefully more free bytes

# Window size
- Subject to: BDP, recv window size, current network congestion (congestion window size -> abstraction) => take the min of these three
    - In slides current network congestion was represented as a router with a higher incoming bandwidth than outgoing bandwidth
        - The BDP implied the opposite of what was truly the limiting factor (see slides if future me is confused)

# Closing TCP connection
- Before sending FIN A needs to make sure the sliding window data has been sent
- A sends FIN
- B sends back ACK
- B sends a FIN
- A sends back an ACK
- After A sends back the ACK it has a delay window/timeout period, if B doesn't send anything back then we assume we're good to go and close the connection


