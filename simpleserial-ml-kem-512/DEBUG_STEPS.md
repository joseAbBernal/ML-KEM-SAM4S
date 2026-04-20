# ML-KEM-512 Firmware Debug Progress

## Backup Points
- **v0-indcpa_only.c** (4.9 KB): `cp simpleserial-ml-kem-512.c.backup-working simpleserial-ml-kem-512.c`

## Test Versions

### ✅ v1: indcpa_dec() ONLY
**Status:** Works - executes in milliseconds
**Size:** 4.9 KB
**Behavior:** LED1 ON → immediate LED3 ON
**Functions:**
- `PQCLEAN_MLKEM512_CLEAN_indcpa_dec()`

### 🧪 v2: indcpa_dec() + hash_g()
**Status:** Testing
**Size:** 9.3 KB
**Functions:**
- `PQCLEAN_MLKEM512_CLEAN_indcpa_dec()`
- `hash_g()` (SHA3-512 of buf)
**Test Result:** [PENDING - USER TO TEST]

### 🔲 v3: +indcpa_enc()
**Status:** Not started
**Plan:** Add re-encryption for verification check

### 🔲 v4: +verify()
**Status:** Not started
**Plan:** Add constant-time comparison

### 🔲 v5: +rkprf()
**Status:** Not started
**Plan:** Add rejection PRF

### 🔲 v6: Full crypto_kem_dec()
**Status:** Not started
**Plan:** Add cmov() for final secret selection

## Hardware Test Procedure
```
1. Flash simpleserial-ml-kem-512-CWHUSKY.bin to SAM4S
2. Send command 'k' via ChipWhisperer serial
3. Observe LED behavior:
   - LED1 should turn ON
   - If working: LED1 OFF, LED3 ON (immediate)
   - If hanging: LED1 stays ON (infinite loop)
4. Report result
```

## Findings
- `indcpa_dec()` contains `cmov_int16()` for constant-time operations
- Problem functions: Likely in hash computation or re-encryption path
