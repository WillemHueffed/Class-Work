# TCP Congestion Control 

## Review
- RWND: recv window size
- CWND: flow control window
- Sender side window = min { CWND, RWND }

## ACK Clocking
- Sender will send packets contiguously
- Due to a bottleneck link the packets may become space outed
- ACKS maintain the spread via the return path
- Sender now mirrors the interval between ACKs

## TCP slow start
- Initally start with 1 MSS (maximum segment size)
- At every ACK double until loss is detected

## What triggers resending
- Loss
  - timeouts
  - 3 duplicate ACKs
- Reaction algorithm
  - AIMD (additive increase, multiplicative decrease)

## Slow start implementation
- Double CWND uyntil first RTT loss event
- introduce a slow start threshold parameter - SSTHRESH
- On first loss: SSTHRESH = CWND/2

## On timeout event
- indicative of critical failure
- go back to slow start
- byte method: cwnd -> cwnd + (1/cwnd)
## Slow start implementation
- Double CWND uyntil first RTT loss event
- introduce a slow start threshold parameter - SSTHRESH
- On first loss: SSTHRESH = CWND/2

## On timeout event
- indicative of critical failure
- go back to slow start
- byte method: cwnd -> cwnd + (1/cwnd)
- segment method: cwnd -> cwnd + (mss * (mss/cwnd))

## On 3 dup ACK
- CWND -> CWND/2
- then do additive increase

## On timeout
- tretransmit first missing packet (as usual)
- set SSTHRESH <- CWND/2

## Implemntation
- CWND (initalized to a 1 MSS)
- SSTHRESH (initialized to a large constant)
- dupACKCount (initialized to zero)
- Timer

## TCP patch - Fast Recovory
- big of a hack but boosts performance
- AI is too slow to recover from an isolated loss
- Can't use slow start because it is too aggressive

## Fast recovery
- Idea: grant the sender temporary "credit" for each dup-ACK so as to keep packets in flight
- if dupACKcount=3
  - SSTHRESH = CWND/2
  - CWND = SSTHRESH + 3
- While in fast recovery
  - CWND = CWND + 1 (MSS) for each additional duplicate ACK
- Exit fast recovery after receiving new ACK
  - set CWND == SSTHRESH

## TCP throughput equation
- Average throughput is 3/4*W_max * MSS/RTT -> see slides for details
- given that the packet drop rate is p=1/A then avg throughput = sqrt(3/2)*(MSS/(RTT*sqrt(p)))
- insights:
  - flow is inversely proportional to RTT
  - 
