#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "simpleserial.h"
#include "Chunks.h"

#if HAL_TYPE == HAL_stm32f0_nano
#define SK_LEN 800
#define PK_LEN 800
#define PT_LEN 100
#define CT_LEN 768
#define SS_LEN 32
#else
#define SK_LEN 1632
#define PK_LEN 800
#define PT_LEN 768
#define CT_LEN 768
#define SS_LEN 32
#endif

uint16_t CHUNK_SZ = 248; // Tamaño de chunk por defecto (1-248)

#if HAL_TYPE == HAL_stm32f0_nano
static uint8_t sk[SK_LEN] = {0};
static uint8_t pk[PK_LEN] = {0};
static uint8_t pt[PT_LEN] = {0};
static uint8_t ct[CT_LEN] = {0};
static uint8_t ss[SS_LEN] = {0};
#else
static uint8_t sk[SK_LEN] = {0};
static uint8_t pk[PK_LEN] = {0};
static uint8_t pt[PT_LEN] = {0};
static uint8_t ct[CT_LEN] = {0};
static uint8_t ss[SS_LEN] = {0};
#endif

/*
 * @brief Receives a chunk of the secret key.
 *
 * Writes the incoming payload into the secret key buffer at the
 * offset derived from the subcommand byte multiplied by the
 * current chunk size.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that we don't overflow the buffer
	if (offset >= SK_LEN) return SS_ERR_LEN;
	// Limit the amount to copy to the available space
	uint16_t to_copy = (offset + len > SK_LEN) ? SK_LEN - offset : len;
	memcpy(sk + offset, buf, to_copy);
	return SS_ERR_OK;
}

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
uint8_t tx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that the offset is within bounds
	if (offset >= SK_LEN) return SS_ERR_LEN;
	// Send the data chunk (CHUNK_SZ or less if we reach the end of the buffer)
	uint16_t to_send = (offset + CHUNK_SZ > SK_LEN) ? SK_LEN - offset : CHUNK_SZ;
	simpleserial_put('k', to_send, sk + offset);
	return SS_ERR_OK;
}

/*
 * @brief Receives a chunk of the public key.
 *
 * Writes the incoming payload into the public key buffer at the
 * offset derived from the subcommand byte multiplied by the
 * current chunk size.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that we don't overflow the buffer
	if (offset >= PK_LEN) return SS_ERR_LEN;
	// Limit the amount to copy to the available space
	uint16_t to_copy = (offset + len > PK_LEN) ? PK_LEN - offset : len;
	memcpy(pk + offset, buf, to_copy);
	return SS_ERR_OK;
}

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
uint8_t tx_pk(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that the offset is within bounds
	if (offset >= PK_LEN) return SS_ERR_LEN;
	// Send the data chunk (CHUNK_SZ or less if we reach the end of the buffer)
	uint16_t to_send = (offset + CHUNK_SZ > PK_LEN) ? PK_LEN - offset : CHUNK_SZ;
	simpleserial_put('p', to_send, pk + offset);
	return SS_ERR_OK;
}

/*
 * @brief Receives a chunk of ciphertext or plaintext.
 *
 * Writes the incoming payload into the ciphertext/plaintext buffer
 * at the offset derived from the subcommand byte multiplied by the
 * current chunk size.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that we don't overflow the buffer
	if (offset >= CT_LEN) return SS_ERR_LEN;
	// Limit the amount to copy to the available space
	uint16_t to_copy = (offset + len > CT_LEN) ? CT_LEN - offset : len;
	memcpy(ct + offset, buf, to_copy);
	return SS_ERR_OK;
}

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
uint8_t tx_ct(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that the offset is within bounds
	if (offset >= CT_LEN) return SS_ERR_LEN;
	// Send the data chunk (CHUNK_SZ or less if we reach the end of the buffer)
	uint16_t to_send = (offset + CHUNK_SZ > CT_LEN) ? CT_LEN - offset : CHUNK_SZ;
	simpleserial_put('c', to_send, ct + offset);
	return SS_ERR_OK;
}

/*
 * @brief Receives a chunk of the shared secret or tag.
 *
 * Writes the incoming payload into the shared-secret/tag buffer
 * at the offset derived from the subcommand byte multiplied by the
 * current chunk size.
 *
 * @param cmd   SimpleSerial command byte.
 * @param scmd  Subcommand byte encoding the chunk index.
 * @param len   Payload length in bytes.
 * @param buf   Pointer to the payload buffer.
 * @return      SS_ERR_OK on success, SS_ERR_LEN on out-of-range access.
 */
uint8_t rx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that we don't overflow the buffer
	if (offset >= SS_LEN) return SS_ERR_LEN;
	// Limit the amount to copy to the available space
	uint16_t to_copy = (offset + len > SS_LEN) ? SS_LEN - offset : len;
	memcpy(ss + offset, buf, to_copy);
	return SS_ERR_OK;
}

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
uint8_t tx_ss(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	uint16_t offset = scmd * CHUNK_SZ;
	// Validate that the offset is within bounds
	if (offset >= SS_LEN) return SS_ERR_LEN;
	// Send the data chunk (CHUNK_SZ or less if we reach the end of the buffer)
	uint16_t to_send = (offset + CHUNK_SZ > SS_LEN) ? SS_LEN - offset : CHUNK_SZ;
	simpleserial_put('s', to_send, ss + offset);
	return SS_ERR_OK;
}

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
uint8_t set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	if (len != 2) return SS_ERR_LEN; // Expect exactly 2 bytes (uint16_t)
	uint16_t new_size = buf[0] | (buf[1] << 8);
	if (new_size < 1 || new_size > 248) return SS_ERR_LEN; // SimpleSerial v2.1 limit
	CHUNK_SZ = new_size;
	return SS_ERR_OK;
}
