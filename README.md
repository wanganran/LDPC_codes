# LDPC_codes
Low density parity check codes. Encoder and belief propagation decoder written in C++.

It supports:

1. generating parity check matrix: quasi-cyclic codes, regular array codes, irregular array codes, gallager codes, or codes from alist file from http://www.inference.phy.cam.ac.uk/mackay/codes/data.html.

2. encoding messages (bit-wise or message-wise) by a given parity check matrix.

3. decoding messages by a given parity check matrix using belief propagation. BEC and BSC channels are supported. For BSC, soft-decision decoding and hard-decision decoding are supported.

Although worked, this project still lacks performance optimizations. 

The compiler must support C++11.
