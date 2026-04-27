
target = cw.target(scope, cw.targets.SimpleSerial2) #cw.targets.SimpleSerial can be omitted

make PLATFORM=CWNANO CRYPTO_TARGET=NONE SS_VER=SS_VER_2_1

# Enviar en chunks de 200 bytes
target.con()
for i in range(8):  # 0 a 9
    start = i * 204
    chunk = sk[start:start + 204]
    target.send_cmd(0x02, i, chunk)  # cmd=0x02, scmd=índice del chunk
    print(f"Enviado chunk {i}/7")
    target.con()


target.con()
received_key = bytearray()
for i in range(8):
    target.send_cmd(0x03, i, [])
    chunk = target.simpleserial_read('k', 204)
    target.con()
    #time.sleep(1)
    if chunk is not None:
        received_key.extend(chunk)
        print(f"Recibido chunk {i}/7: {chunk.hex()}")
    else:
        print(f"Timeout en chunk {i}/7, reintentando...")




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
