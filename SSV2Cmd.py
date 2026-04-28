
import time
import chipwhisperer as cw

target = cw.target(scope, cw.targets.SimpleSerial2)

scope.default_setup()

# Compilar firmware desde terminal (no es Python):
# make PLATFORM=CWNANO CRYPTO_TARGET=NONE SS_VER=SS_VER_2_1

SK_LEN = 1632
chunk_size = 100  # Mantener <= 100 para reducir riesgo de UART overrun
num_chunks = (SK_LEN + chunk_size - 1) // chunk_size

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
    end = min(start + chunk_size, SK_LEN)
    tx_chunk = sk[start:end]
    target.send_cmd(0x02, i, tx_chunk)
    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR envio chunk {i}: ack={ack}")
    print(f"Enviado chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
    time.sleep(0.02)

# 3) Leer cada chunk: primero paquete 'k', luego ACK del comando 0x03
rec_sk = bytearray()
for i in range(num_chunks):
    start = i * chunk_size
    expected_len = min(chunk_size, SK_LEN - start)

    target.send_cmd(0x03, i, [])

    # Leer respuesta de datos sin consumir ACK automaticamente
    rx_chunk = target.simpleserial_read('k', expected_len, ack=False)
    if rx_chunk is None:
        raise RuntimeError(f"Timeout leyendo chunk {i}")

    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR ACK lectura chunk {i}: ack={ack}")

    rec_sk.extend(rx_chunk)
    print(f"Recibido chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
    time.sleep(0.02)


print("\n--- Verificación ---")
if len(rec_sk) != len(sk):
    print(f"ERROR: Longitud mismatch. Esperado: {len(sk)}, Recibido: {len(rec_sk)}")
else:
    if rec_sk == sk:
        print("ÉXITO: Llave recibida idéntica a la enviada")
    else:
        print("ERROR: Llave recibida no coincide")
        # Encontrar primera diferencia
        for i in range(len(sk)):
            if sk[i] != rec_sk[i]:
                print(f"Primera diferencia en byte {i}: esperado 0x{sk[i]:02x}, recibido 0x{rec_sk[i]:02x}")
                break

#PK_LEN = 800 test
PK_LEN = 800
chunk_size = 100  # Mantener <= 100 para reducir riesgo de UART overrun
num_chunks = (PK_LEN + chunk_size - 1) // chunk_size

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

# 2) Enviar PK en chunks con ACK por cada envio
for i in range(num_chunks):
    start = i * chunk_size
    end = min(start + chunk_size, PK_LEN)
    tx_chunk = pk[start:end]
    target.send_cmd(0x05, i, tx_chunk)
    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR envio chunk {i}: ack={ack}")
    print(f"Enviado chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
    time.sleep(0.02)

# 3) Leer cada chunk: primero paquete 'k', luego ACK del comando 0x03
rec_pk = bytearray()
for i in range(num_chunks):
    start = i * chunk_size
    expected_len = min(chunk_size, PK_LEN - start)

    target.send_cmd(0x06, i, [])

    # Leer respuesta de datos sin consumir ACK automaticamente
    rx_chunk = target.simpleserial_read('p', expected_len, ack=False)
    if rx_chunk is None:
        raise RuntimeError(f"Timeout leyendo chunk {i}")

    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR ACK lectura chunk {i}: ack={ack}")

    rec_pk.extend(rx_chunk)
    print(f"Recibido chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
    time.sleep(0.02)

print("\n--- Verificación ---")
if len(rec_pk) != len(pk):
    print(f"ERROR: Longitud mismatch. Esperado: {len(pk)}, Recibido: {len(rec_pk)}")
else:
    if rec_pk == pk:
        print("ÉXITO: Pk recibida idéntica a la enviada")
    else:
        print("ERROR: Pk recibida no coincide")
        # Encontrar primera diferencia
        for i in range(len(pk)):
            if pk[i] != rec_pk[i]:
                print(f"Primera diferencia en byte {i}: esperado 0x{pk[i]:02x}, recibido 0x{rec_pk[i]:02x}")
                break


#CT_LEN = 768 test
CT_LEN = 768
chunk_size = 100  # Mantener <= 100 para reducir riesgo de UART overrun
num_chunks = (CT_LEN + chunk_size - 1) // chunk_size

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

# 2) Enviar CT en chunks con ACK por cada envio
for i in range(num_chunks):
    start = i * chunk_size
    end = min(start + chunk_size, CT_LEN)
    tx_chunk = ct[start:end]
    target.send_cmd(0x07, i, tx_chunk)
    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR envio chunk {i}: ack={ack}")
    print(f"Enviado chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
    time.sleep(0.02)

# 3) Leer cada chunk: primero paquete 'k', luego ACK del comando 0x03
rec_ct = bytearray()
target.con()
for i in range(num_chunks):
    start = i * chunk_size
    expected_len = min(chunk_size, CT_LEN - start)

    target.send_cmd(0x08, i, [])

    # Leer respuesta de datos sin consumir ACK automaticamente
    rx_chunk = target.simpleserial_read('c', expected_len, ack=False)
    if rx_chunk is None:
        raise RuntimeError(f"Timeout leyendo chunk {i}")

    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR ACK lectura chunk {i}: ack={ack}")

    rec_ct.extend(rx_chunk)
    print(f"Recibido chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
    time.sleep(0.02)

print("\n--- Verificación ---")
if len(rec_ct) != len(ct):
    print(f"ERROR: Longitud mismatch. Esperado: {len(ct)}, Recibido: {len(rec_ct)}")
else:
    if rec_ct == ct:
        print("ÉXITO: Ct recibido idéntico al enviado")
    else:
        print("ERROR: Ct recibido no coincide")
        # Encontrar primera diferencia
        for i in range(len(ct)):
            if ct[i] != rec_ct[i]:
                print(f"Primera diferencia en byte {i}: esperado 0x{ct[i]:02x}, recibido 0x{rec_ct[i]:02x}")
                break

# SS_LEN = 32  test
SS_LEN = 32
chunk_size = 32  # Mantener <= 100 para reducir riesgo de UART overrun
num_chunks = (SS_LEN + chunk_size - 1) // chunk_size

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

# 2) Enviar SS en chunks con ACK por cada envio
for i in range(num_chunks):
    start = i * chunk_size
    end = min(start + chunk_size, SS_LEN)
    tx_chunk = ss[start:end]
    target.send_cmd(0x09, i, tx_chunk)
    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR envio chunk {i}: ack={ack}")
    print(f"Enviado chunk {i}/{num_chunks-1}: {len(tx_chunk)} bytes")
    time.sleep(0.02)

# 3) Leer cada chunk: primero paquete 'k', luego ACK del comando 0x03
rec_ss = bytearray()
for i in range(num_chunks):
    start = i * chunk_size
    expected_len = min(chunk_size, SS_LEN - start)

    target.send_cmd(0x0A, i, [])

    # Leer respuesta de datos sin consumir ACK automaticamente
    rx_chunk = target.simpleserial_read('s', expected_len, ack=False)
    if rx_chunk is None:
        raise RuntimeError(f"Timeout leyendo chunk {i}")

    ack = target.simpleserial_wait_ack()
    if ack is None or len(ack) == 0 or ack[0] != 0x00:
        raise RuntimeError(f"ERROR ACK lectura chunk {i}: ack={ack}")

    rec_ss.extend(rx_chunk)
    print(f"Recibido chunk {i}/{num_chunks-1}: {len(rx_chunk)} bytes")
    time.sleep(0.02)

print("\n--- Verificación ---")
if len(rec_ss) != len(ss):
    print(f"ERROR: Longitud mismatch. Esperado: {len(ss)}, Recibido: {len(rec_ss)}")
else:
    if rec_ss == ss:
        print("ÉXITO: Llave recibida idéntica a la enviada")
    else:
        print("ERROR: Llave recibida no coincide")
        # Encontrar primera diferencia
        for i in range(len(ss)):
            if ss[i] != rec_ss[i]:
                print(f"Primera diferencia en byte {i}: esperado 0x{ss[i]:02x}, recibido 0x{rec_ss[i]:02x}")
                break