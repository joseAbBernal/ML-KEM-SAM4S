import time
import chipwhisperer as cw

target = cw.target(scope, cw.targets.SimpleSerial2)

# Definiciones de tamaños
SK_LEN = 1632
PK_LEN = 800
SS_LEN = 32
chunk_size = 100

# Limpiar basura de ejecuciones anteriores
target.reset_comms()
target.flush()

# Verificar comandos disponibles
cmds = target.get_simpleserial_commands()
print("Comandos reportados:", cmds)

# Configurar chunk size
target.send_cmd(0x04, 0, [chunk_size & 0xFF, (chunk_size >> 8) & 0xFF])
ack = target.simpleserial_wait_ack()
print(f"ACK 0x04 (set_chunk_size): {ack[0]:02x}" if ack else "timeout")
time.sleep(0.05)

# ============== ENVIAR DATOS ==============

def send_buffer(data, total_len, cmd_rx, cmd_num, chunk_size):
    """Enviar buffer en chunks"""
    num_chunks = (total_len + chunk_size - 1) // chunk_size
    for i in range(num_chunks):
        start = i * chunk_size
        end = min(start + chunk_size, total_len)
        tx_chunk = data[start:end]
        
        target.send_cmd(cmd_rx, i, tx_chunk)
        ack = target.simpleserial_wait_ack()
        if ack is None or len(ack) == 0 or ack[0] != 0x00:
            raise RuntimeError(f"ERROR enviando {cmd_num} chunk {i}: ack={ack}")
        print(f"  Chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
        time.sleep(0.02)

def read_buffer(total_len, cmd_tx, cmd_num, chunk_size):
    """Leer buffer en chunks"""
    num_chunks = (total_len + chunk_size - 1) // chunk_size
    received = bytearray()
    
    for i in range(num_chunks):
        start = i * chunk_size
        expected_len = min(chunk_size, total_len - start)
        
        target.send_cmd(cmd_tx, i, [])
        
        rx_chunk = target.simpleserial_read(None, expected_len, ack=False)
        if rx_chunk is None:
            raise RuntimeError(f"Timeout leyendo {cmd_num} chunk {i}")
        
        ack = target.simpleserial_wait_ack()
        if ack is None or len(ack) == 0 or ack[0] != 0x00:
            raise RuntimeError(f"ERROR ACK lectura {cmd_num} chunk {i}: ack={ack}")
        
        received.extend(rx_chunk)
        print(f"  Chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
        time.sleep(0.02)
    
    return received

print("\n--- Enviando SK ---")
send_buffer(sk, SK_LEN, 0x02, "SK", chunk_size)

print("\n--- Enviando PK ---")
send_buffer(pk, PK_LEN, 0x05, "PK", chunk_size)

print("\n--- Enviando SS ---")
send_buffer(ss, SS_LEN, 0x07, "SS", chunk_size)

print("\n--- Leyendo SK ---")
received_sk = read_buffer(SK_LEN, 0x03, "SK", chunk_size)

print("\n--- Leyendo PK ---")
received_pk = read_buffer(PK_LEN, 0x06, "PK", chunk_size)

print("\n--- Leyendo SS ---")
received_ss = read_buffer(SS_LEN, 0x08, "SS", chunk_size)

# ============== VERIFICACIÓN ==============

def verify_buffer(original, received, name):
    """Verificar integridad de un buffer"""
    if len(received) != len(original):
        print(f"ERROR {name}: Longitud mismatch. Esperado: {len(original)}, Recibido: {len(received)}")
        return False
    
    if received == original:
        print(f"ÉXITO {name}: Idéntico ({len(received)} bytes)")
        return True
    else:
        print(f"ERROR {name}: Datos no coinciden")
        for i in range(len(original)):
            if original[i] != received[i]:
                print(f"  Primera diferencia en byte {i}: esperado 0x{original[i]:02x}, recibido 0x{received[i]:02x}")
                break
        return False

print("\n--- Verificación Final ---")
verify_buffer(sk, received_sk, "SK")
verify_buffer(pk, received_pk, "PK")
verify_buffer(ss, received_ss, "SS")
