# Congestion Control - 5/7/24

## Dynamic Resource allocation
- Congestion control is a resource allocation problem
- Allocations in our contest are highly interdependent

## Goals
From a resource perspective
- low packet delay and loss
- high link utilization
- fair sharing of resources
From a system perspective
- scalable
- decentralized
- adaptive

## Possible Approaches
- (1) Reservations
- (2) Pricing/Priorities
- (3) Dyanmic Adjustment
- Real world: use dynamic adjustment -> assumes good actors

## Questions
- (1) How does A know network is congested?
- (2) When will A increase its transmission rate?
  - When there are no duplicae ACKs for a time period
- (3) How much increase/decrease each time?
  - Additive increase, multiplicitive decreease
- (4) Time interval between adjusting transmission rate?
  - Related to timeout timer
- (5) Whats your initial transmission rate?

## Two broad classes of solution (for dynamic adjustment)
- Host based CC -> Jacobson's original TCP approach
  - no special support from routers
  - host adjust rate based on implicit feedback from routers
- Router-assisted CC
  - Routers signal congestion back to hosts
  - Hosts pick rate baeed on explicit feedback
  - Special bit gets flipped by router

## Note not all loses the same
- Duplicate ACKs
  - isolated, not a big deal
- Timeout
  - serious issue

## Discovering initial rate
- Start low, ramp up fast
- Ramp up fast -> double transmission rate
  - Once first issue occurs -> cut transmission rate in half -> this is the safe rate

## Target:
- For two sers x1 and x2
- x1+x2=1 means full utilization
- x1=x2 means fairness
- We want the intersection of these two linear relationships
- Only AIMD achieves this (additive increase, multiplicitive decrease)
  - This is a function of the way the slope changes or doesn't change (see slides for more detail)
- Note: Each sender does this independently -> lower overhead than having to do synchronization
