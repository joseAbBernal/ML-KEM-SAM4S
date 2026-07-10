#ifndef LIB_CHUNKS_H
#define LIB_CHUNKS_H

#include <inttypes.h>

/*
Receive data via Simple Serial V 2.1 
The following functions serve to receive the following variables

Sk = (Secret Key)
Pk = (Public Key)
Ct = (Cipher Text / Plaintext)
SS = (Tag or other values)

*/

/*
 * @brief Receives a chunk of the secret key.
 *
 * Writes the incoming payload into the secret key buffer at the
 * offset derived from the subcommand byte.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Receives a chunk of the public key.
 *
 * Writes the incoming payload into the public key buffer at the
 * offset derived from the subcommand byte.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Receives a chunk of ciphertext or plaintext.
 *
 * Writes the incoming payload into the ciphertext/plaintext buffer
 * at the offset derived from the subcommand byte.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Receives a chunk of the shared secret or tag.
 *
 * Writes the incoming payload into the shared-secret/tag buffer
 * at the offset derived from the subcommand byte.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);


/*
Send data via Simple Serial V 2.1
The following functions serve to transmit or send the following variables
Sk = (Secret Key)
Pk = (Public Key)
Ct = (Cipher Text / Plaintext)
SS = (Tag or other values)

*/


/*
 * @brief Transmits a chunk of the secret key.
 *
 * Sends the requested chunk from the secret key buffer back to the
 * host over SimpleSerial.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer (unused).
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t tx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Transmits a chunk of the public key.
 *
 * Sends the requested chunk from the public key buffer back to the
 * host over SimpleSerial.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer (unused).
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t tx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Transmits a chunk of ciphertext or plaintext.
 *
 * Sends the requested chunk from the ciphertext/plaintext buffer
 * back to the host over SimpleSerial.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer (unused).
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t tx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

/*
 * @brief Transmits a chunk of the shared secret or tag.
 *
 * Sends the requested chunk from the shared-secret/tag buffer back
 * to the host over SimpleSerial.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer (unused).
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t tx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);


/*
The Max size of the chunk is 248 bytes of payload, and can be configured 
from 1 byte upto 248 bytes.

Set a different value for the Chunk size used by each function, for example:
a secret key of 1632 bytes length can be transmitted or received as Chunks of
variable size for example  8 Chunks of 200 bytes and 1 Chunk of 32 bytes,
(8 * 200) + 32 = 1632 bytes.

Other example  is to set the Chunk size of 102 bytes, then the system will send
(16 * 102) = 1632 bytes

Such that the system uses the following formula

Len 				= L,
chunk size    		= S,
Number of chunks	= N.

L / S = N; in some cases the value can't be a full payload but the functions

already solve this issue and it solves the issue automatically.
*/

/*
 * @brief Updates the active chunk size at runtime.
 *
 * The new size must be between 1 and 248 bytes, inclusive.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  SimpleSerial subcommand byte.
 * @param len   Payload length in bytes; must be exactly 2.
 * @param buf   Pointer to a 2-byte little-endian uint16_t payload.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on bad length or invalid size.
 */
uint8_t set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf);

#endif