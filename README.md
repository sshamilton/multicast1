multicast1
==========

UDP Multicast client/server

This project allows testing of data loss rates when multicasting with up to 10 clients.  There are up to 6 senders,
and they each send a specified amount of packets with a random number assigned to each.  The goal is that each
client will receive all the packets in agreed order when it is complete.
