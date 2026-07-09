#include "simpleserial.h"

/*
Receive data via Simple Serial V 2.1 
The following functions serve to receive the following variables

Sk = (Secret Key)
Pk = (Public Key)
Ct = (Cipher Text / Plaintext)
SS = (Tag or other values)

*/

/* Receive Secret Key as chunks */
uint8_t rx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Receive Public Key as Chunks */
uint8_t rx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Receive Ciphertext / Plaintext as Chunks */
uint8_t rx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Receive SS or Tag as Chunks */
uint8_t rx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);


/*
Send data via Simple Serial V 2.1
The following functions serve to transmit or send the following variables
Sk = (Secret Key)
Pk = (Public Key)
Ct = (Cipher Text / Plaintext)
SS = (Tag or other values)

*/


/* Transmit Secre Key as Chunks */
uint8_t tx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Transmit Public Key as Chunks */
uint8_t tx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Transmit Ciphertext / Plaintext as Chunks*/
uint8_t tx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/* Transmit SS / Tag as Chunks */
uint8_t tx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);


/*
The Max size of the chunk is 248 bytes of payload, and can be configured 
from 1 byte upto 248 bytes.

Set a different value for the Chunk size used by each function, for example:
a secret key of 1632 bytes length can be transmited or received as chunks of
varible size for example  8 Chuks of 200 bytes and 1 chunk of 32 bytes,
(8 * 200) + 32 = 1632 bytes.

Other example  is to set de chunk size of 102 bytes, then the system will send
(16 * 102) = 1632 bytes

Such that the system uses the following formula

Len 				= L,
chunk size    		= S,
Number of chunks	= N.

L / S = N; in some cases the value can't be a full payload but the functions

already solve this issue and it solve the issu automatically.
*/

uint8_t set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);