This is an implementation of a reliable transfer service on top of the UDP/IP protocol. In other words, we need to implement a service that guarantees the
arrival of datagrams in the correct order on top of the UDP/IP protocol, along with congestion
control.

*Headlines*:
* Reliable Data Transfer:
  - Stop-and-Wait
  - Selective Repeat

* Congestion Control:
  - Additive increase Multiplicative decrease

* Packet Loss Simulation
