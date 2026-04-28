/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2017 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "simpleserial.h"

#define KEY_LEN 1632
uint16_t CHUNK_SZ = 248;  // Tamaño de chunk por defecto (1-249)

static uint8_t key_buf[KEY_LEN] = {0};

uint8_t get_key(uint8_t* k, uint8_t len)
{
	// Load key here
	return 0x00;
}

uint8_t get_pt(uint8_t* pt, uint8_t len)
{
	/**********************************
	* Start user-specific code here. */
	trigger_high();

	//16 hex bytes held in 'pt' were sent
	//from the computer. Store your response
	//back into 'pt', which will send 16 bytes
	//back to computer. Can ignore of course if
	//not needed

	trigger_low();
	/* End user-specific code here. *
	********************************/
	simpleserial_put('r', 16, pt);
	return 0x00;
}

uint8_t reset(uint8_t* x, uint8_t len)
{
	// Reset key here if needed
	return 0x00;
}

uint8_t rx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint16_t offset = scmd * CHUNK_SZ;
    // Validar que no sobrepasamos los límites
    if (offset >= KEY_LEN) return SS_ERR_LEN;
    // Limitar la cantidad a copiar al espacio disponible
    uint16_t to_copy = (offset + len > KEY_LEN) ? KEY_LEN - offset : len;
    memcpy(key_buf + offset, buf, to_copy);
    // Debug: retornar el primer byte del buffer para verificar
    trigger_high();
    trigger_low();
    return 0x00;
}

uint8_t tx_key(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint16_t offset = scmd * CHUNK_SZ;
    // Validar que el offset está dentro de límites
    if (offset >= KEY_LEN) return SS_ERR_LEN;
    // Enviar el chunk de datos (CHUNK_SZ o menos si llegamos al final)
    uint16_t to_send = (offset + CHUNK_SZ > KEY_LEN) ? KEY_LEN - offset : CHUNK_SZ;
    simpleserial_put('k', to_send, key_buf + offset);
    return 0x00;
}

uint8_t set_chunk_size(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
	if (len != 2) return SS_ERR_LEN;  // Esperamos exactamente 2 bytes (uint16_t)
	uint16_t new_size = buf[0] | (buf[1] << 8);
	if (new_size < 1 || new_size > 249) return SS_ERR_LEN;  // Límite de SimpleSerial v2.1
	CHUNK_SZ = new_size;
	return 0x00;
}

#if SS_VER == SS_VER_2_1
uint8_t aes(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *buf)
{
    uint8_t req_len = 0;
    uint8_t err = 0;

    if (scmd & 0x02) {
        req_len += 16;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        err = get_key(buf + req_len - 16, 16);
        if (err)
            return err;
    }
    if (scmd & 0x01) {
        req_len += 16;
        if (req_len > len) {
            return SS_ERR_LEN;
        }
        err = get_pt(buf + req_len - 16, 16);
        if (err)
            return err;
    }

    if (len != req_len) {
        return SS_ERR_LEN;
    }

    return 0x00;

}
#endif

int main(void)
{
    platform_init();
	init_uart();
	trigger_setup();

 	/* Uncomment this to get a HELLO message for debug */
	/*
	putch('h');
	putch('e');
	putch('l');
	putch('l');
	putch('o');
	putch('\n');
	*/

	simpleserial_init();
#if SS_VER != SS_VER_2_1
	simpleserial_addcmd('p', 16, get_pt);
	simpleserial_addcmd('k', 16, get_key);
	simpleserial_addcmd('x', 0, reset);
#else
    simpleserial_addcmd(0x01, 16, aes);
    simpleserial_addcmd(0x02, 248, rx_key);   // receive chunk (max V2 size - 1)
    simpleserial_addcmd(0x03, 0, tx_key);     // send chunk (scmd = index only, no data)
    simpleserial_addcmd(0x04, 2, set_chunk_size);  // set chunk size

#endif
	while(1)
		simpleserial_get();
}
