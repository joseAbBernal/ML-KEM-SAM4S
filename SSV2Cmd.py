
import time
import chipwhisperer as cw

target = cw.target(scope, cw.targets.SimpleSerial2)

# Compilar firmware desde terminal (no es Python):
# make PLATFORM=CWNANO CRYPTO_TARGET=NONE SS_VER=SS_VER_2_1

TOTAL_LEN = 1632
chunk_size = 100  # Mantener <= 100 para reducir riesgo de UART overrun
num_chunks = (TOTAL_LEN + chunk_size - 1) // chunk_size

# Limpiar basura de ejecuciones anteriores para evitar desincronizacion del parser
target.reset_comms()
target.flush()

# Verificar comandos disponibles en el firmware cargado
cmds = target.get_simpleserial_commands()
print("Comandos reportados:", cmds)

# 1) Configurar chunk size en firmware y esperar ACK
target.send_cmd(0x04, 0, [chunk_size & 0xFF, (chunk_size >> 8) & 0xFF])
ack = target.simpleserial_wait_ack()
print(f"ACK 0x04: {ack}")
if ack is None or len(ack) == 0 or ack[0] != 0x00:
    raise RuntimeError(f"Fallo en cmd 0x04, ack={ack}")
time.sleep(0.05)

# 2) Enviar SK en chunks con ACK por cada envio
for i in range(num_chunks):
    start = i * chunk_size
    end = min(start + chunk_size, TOTAL_LEN)
    tx_chunk = sk[start:end]
    target.send_cmd(0x02, i, tx_chunk)
    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR envio chunk {i}: ack={ack}")
    print(f"Enviado chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
    time.sleep(0.02)

# 3) Leer cada chunk: primero paquete 'k', luego ACK del comando 0x03
received_key = bytearray()
for i in range(num_chunks):
    start = i * chunk_size
    expected_len = min(chunk_size, TOTAL_LEN - start)

    target.send_cmd(0x03, i, [])

    # Leer respuesta de datos sin consumir ACK automaticamente
    rx_chunk = target.simpleserial_read('k', expected_len, ack=False)
    if rx_chunk is None:
        raise RuntimeError(f"Timeout leyendo chunk {i}")

    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR ACK lectura chunk {i}: ack={ack}")

    received_key.extend(rx_chunk)
    print(f"Recibido chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
    time.sleep(0.02)




print("\n--- Verificación ---")
if len(received_key) != len(sk):
    print(f"ERROR: Longitud mismatch. Esperado: {len(sk)}, Recibido: {len(received_key)}")
else:
    if received_key == sk:
        print("ÉXITO: Llave recibida idéntica a la enviada")
    else:
        print("ERROR: Llave recibida no coincide")
        # Encontrar primera diferencia
        for i in range(len(sk)):
            if sk[i] != received_key[i]:
                print(f"Primera diferencia en byte {i}: esperado 0x{sk[i]:02x}, recibido 0x{received_key[i]:02x}")
                break
