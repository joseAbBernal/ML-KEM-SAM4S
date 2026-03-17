#!/usr/bin/env python3
"""

python power_measurement.py  to use it :)

ChipWhisperer ML-KEM-512 Power Measurement Script
===================================================
This script captures power traces from the ML-KEM-512 implementation
running on a CWHUSKY (SAM4S) target.

====================================================
COMMANDOS SIMPLESERIAL - EXPLICACIÓN
====================================================

El firmware usa la interfaz SimpleSerial v2.1 con los siguientes comandos:

1. KEY GENERATION (Generar claves):
   - Comando: 0x02 0x01
   - Descripción: Genera un par de claves pública/privada ML-KEM-512
   - Retorna: Clave pública (800 bytes) en 'p'
   - Uso típico: Generar claves al inicio
   
   Ejemplo de bytes a enviar: b'\x02\x01'
   Respuesta esperada: 800 bytes de clave pública

2. ENCAPSULATION (Encapsular):
   - Comando: 0x02 0x08  
   - Descripción: Encapsula un secreto compartido usando la clave pública
   - Requiere: Clave pública configurada
   - Retorna: Ciphertext (768 bytes) + Secreto compartido (32 bytes) en 'c'
   
   Ejemplo de bytes a enviar: b'\x02\x08'
   Respuesta esperada: 800 bytes (ct + ss)

3. DECAPSULATION (Desencapsular):
   - Comando: 0x02 0x10
   - Descripción: Desencapsula el secreto compartido usando la clave privada
   - Requiere: Clave privada y ciphertext
   - Retorna: Secreto compartido (32 bytes) en 's'
   
   Ejemplo de bytes a enviar: b'\x02\x10'
   Respuesta esperada: 32 bytes de secreto compartido

====================================================
TRIGGER Y MEDICIÓN DE POTENCIA
====================================================

En verify.c, la función cmov() tiene trigger_high() al inicio y 
trigger_low() al final. Esto permite:

1. El osciloscopio detecta cuando empieza la operación crítica
2. Se captura la traza de potencia durante la ejecución
3. Se puede analizar el consumo de la comparación constante

====================================================
REQUISITOS
====================================================
    chipwhisperer
    matplotlib
    numpy

Install: pip install chipwhisperer matplotlib numpy
"""

import chipwhisperer as cw
import matplotlib.pyplot as plt
import numpy as np
import time
import sys

# ============================================================================
# CONFIGURACIÓN
# ============================================================================

# Configuración del objetivo
PLATFORM = "CWHUSKY"        # Target: CWHUSKY (SAM4S)
SCOPETYPE = "OPENADC"        # Osciloscopio: OpenADC
FW_PATH = "simpleserial-mk-kem-512-CWHUSKY.hex"  # Archivo del firmware

# Configuración de mediciones
NUM_TRACES = 100  # Número de trazas a capturar por operación
TARGET_CLOCK = 7372800  # Reloj del SAM4S

# Comandos SimpleSerial (en hexdecimal)
# Formato: [cmd, subcmd]
# - cmd: comando principal (0x02 = KEM en nuestro firmware)
# - subcmd: operación específica
CMD_KEYGEN   = b'\x02\x01'  # 0x01 = generar claves
CMD_ENCAPS   = b'\x02\x08'  # 0x08 = encapsular
CMD_DECAPS   = b'\x02\x10'  # 0x10 = desencapsular

# ============================================================================
# FUNCIONES DE MEDICIÓN
# ============================================================================

def setup_chipwhisperer():
    """Inicializar ChipWhisperer (osciloscopio y objetivo)."""
    print("[*] Conectando al ChipWhisperer...")
    
    # Conectar al osciloscopio OpenADC
    scope = cw.scope()
    scope.default_setup()
    
    # Configurar reloj
    if SCOPETYPE == "OPENADC":
        scope.clock.clkgen_src = "system"
        scope.clock.clkgen_freq = TARGET_CLOCK
    
    # Conectar al objetivo (CWHUSKY)
    target = cw.target(scope, cw.targets.SimpleSerial)
    
    # Programar firmware
    print(f"[*] Programando firmware: {FW_PATH}")
    fw = cw.Firmware(FW_PATH)
    target.set_programmer(cw.programmers.SAM4SProgrammer)
    target.program(fw)
    
    # Configurar trigger
    # El SAM4S tiene un pin de trigger que se activa durante operaciones KEM
    scope.trigger.triggers = "gpio_pd1"  # GPIO como trigger
    
    print("[+] ChipWhisperer inicializado correctamente")
    print(f"    - Plataforma: {PLATFORM}")
    print(f"    - Osciloscopio: {SCOPETYPE}")
    print(f"    - Frecuencia: {TARGET_CLOCK} Hz")
    
    return scope, target


def capture_single_trace(scope, target, command, description=""):
    """
    Capturar una sola traza de potencia.
    
    Args:
        scope: Osciloscopio ChipWhisperer
        target: Objetivo SimpleSerial
        command: Bytes del comando a enviar
        description: Descripción para imprimir
    
    Returns:
        wave: Array de valores de potencia
    """
    # Limpiar buffer
    target.flush()
    
    # Enviar comando al objetivo
    # Esto dispara la operación KEM en el SAM4S
    target.write(command)
    
    # Capturar traza durante la ejecución
    # El trigger se activa automáticamente cuando empieza la operación
    trace = scope.capture()
    
    if trace:
        print(f"[!] Timeout esperando traza para {description}")
        return None
    
    # Obtener forma de onda del osciloscopio
    wave = scope.get_last_trace()
    return wave


def capture_all_operations(scope, target, num_traces=10):
    """
    Capturar trazas para todas las operaciones KEM.
    
    Returns:
        dict: Diccionario con trazas de cada operación
              {'keygen': [trace1, trace2, ...], 'encaps': [...], 'decaps': [...]}
    """
    traces = {
        'keygen': [],   # Generación de claves
        'encaps': [],   # Encapsulamiento
        'decaps': []    # Desencapsulamiento
    }
    
    print(f"[*] Capturando {num_traces} trazas para cada operación KEM...")
    print("=" * 60)
    
    # ====================
    # 1. KEY GENERATION
    # ====================
    print("\n[1] KEY GENERATION (Generación de claves)")
    print("    Enviando comando: 0x02 0x01")
    print("    Descripción: Genera par de claves pública/privada ML-KEM-512")
    print("    Esta operación es la más intensiva computacionalmente")
    print("    - Genera polinomios aleatorios")
    print("    - Realiza NTT (Number Theoretic Transform)")
    print("    - Multiplicaciones de polinomios")
    print("    - Hash SHAKE256")
    
    for i in range(num_traces):
        trace = capture_single_trace(scope, target, CMD_KEYGEN, "keygen")
        if trace is not None:
            traces['keygen'].append(trace)
        
        # Progress indicator
        percent = ((i + 1) / num_traces) * 100
        bar = '█' * int(percent / 5) + '░' * (20 - int(percent / 5))
        print(f"    [{bar}] {i+1}/{num_traces} ({percent:.0f}%)", end='\r')
        sys.stdout.flush()
    
    print(f"\n    ✓ Capturadas {len(traces['keygen'])} trazas de keygen")
    
    # ====================
    # 2. ENCAPSULATION
    # ====================
    print("\n[2] ENCAPSULATION (Encapsulamiento)")
    print("    Enviando comando: 0x02 0x08")
    print("    Descripción: Encapsula secreto con clave pública")
    print("    Proceso:")
    print("    - Genera ruido aleatorio")
    print("    - Multiplica matriz-vector")
    print("    - Aplica NTT inverso")
    print("    - Genera hash del resultado")
    
    for i in range(num_traces):
        trace = capture_single_trace(scope, target, CMD_ENCAPS, "encaps")
        if trace is not None:
            traces['encaps'].append(trace)
        
        percent = ((i + 1) / num_traces) * 100
        bar = '█' * int(percent / 5) + '░' * (20 - int(percent / 5))
        print(f"    [{bar}] {i+1}/{num_traces} ({percent:.0f}%)", end='\r')
        sys.stdout.flush()
    
    print(f"\n    ✓ Capturadas {len(traces['encaps'])} trazas de encaps")
    
    # ====================
    # 3. DECAPSULATION
    # ====================
    print("\n[3] DECAPSULATION (Desencapsulamiento)")
    print("    Enviando comando: 0x02 0x10")
    print("    Descripción: Recupera secreto con clave privada")
    print("    Proceso (similar a encaps pero con clave privada):")
    print("    - Decodifica ciphertext")
    print("    - Aplica NTT")
    print("    - Multiplicación con clave privada")
    print("    - Función de hash (rechazo si falla)")
    print("    - CMOV (aquí está el trigger!)")
    
    for i in range(num_traces):
        trace = capture_single_trace(scope, target, CMD_DECAPS, "decaps")
        if trace is not None:
            traces['decaps'].append(trace)
        
        percent = ((i + 1) / num_traces) * 100
        bar = '█' * int(percent / 5) + '░' * (20 - int(percent / 5))
        print(f"    [{bar}] {i+1}/{num_traces} ({percent:.0f}%)", end='\r')
        sys.stdout.flush()
    
    print(f"\n    ✓ Capturadas {len(traces['decaps'])} trazas de decaps")
    
    print("\n" + "=" * 60)
    print("[+] Captura completada!")
    
    return traces


# ============================================================================
# FUNCIONES DE GRAFICADO
# ============================================================================

def plot_power_traces(traces, save_path=None):
    """
    Graficar trazas de potencia individuales.
    
    Muestra:
    - Primeras 5 trazas de cada operación
    - Traza promedio
    - Estadísticas (media, desviación estándar, máximo)
    """
    fig, axes = plt.subplots(3, 1, figsize=(14, 12))
    
    operations = [('keygen', 'Key Generation', 'blue'),
                  ('encaps', 'Encapsulation', 'green'),
                  ('decaps', 'Decapsulation', 'red')]
    
    for idx, (op, title, color) in enumerate(operations):
        ax = axes[idx]
        
        if traces[op]:
            # Graficar primeras 5 trazas
            for i, trace in enumerate(traces[op][:5]):
                alpha = 0.3 if i > 0 else 1.0
                label = f'Trace {i+1}' if i == 0 else None
                ax.plot(trace, alpha=alpha, label=label,
                       color=color, linewidth=0.5)
            
            # Calcular y graficar promedio
            avg_trace = np.mean(traces[op], axis=0)
            ax.plot(avg_trace, color='black', linewidth=2, 
                   label='Average', linestyle='--')
            
            ax.set_title(f'ML-KEM-512 {title} Power Consumption', fontsize=14)
            ax.set_xlabel('Sample Number', fontsize=12)
            ax.set_ylabel('Power (ADC units)', fontsize=12)
            ax.legend(loc='upper right')
            ax.grid(True, alpha=0.3)
            
            # Estadísticas
            stats = f"Mean: {np.mean(avg_trace):.2f}\n"
            stats += f"Std: {np.std(avg_trace):.2f}\n"
            stats += f"Max: {np.max(avg_trace):.2f}\n"
            stats += f"Min: {np.min(avg_trace):.2f}"
            ax.text(0.02, 0.98, stats, transform=ax.transAxes,
                   verticalalignment='top', fontsize=10,
                   bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"[+] Gráfico guardado: {save_path}")
    
    plt.show()


def plot_comparison(traces, save_path=None):
    """Comparar todas las operaciones en una sola gráfica."""
    plt.figure(figsize=(14, 6))
    
    colors = {'keygen': 'blue', 'encaps': 'green', 'decaps': 'red'}
    labels = {'keygen': 'Key Generation', 
              'encaps': 'Encapsulation', 
              'decaps': 'Decapsulation'}
    
    for op, color in colors.items():
        if traces[op]:
            avg_trace = np.mean(traces[op], axis=0)
            plt.plot(avg_trace, color=color, linewidth=1.5, 
                    label=f'{labels[op]} (avg of {len(traces[op])} traces)', alpha=0.8)
    
    plt.title('ML-KEM-512 Power Consumption Comparison', fontsize=14)
    plt.xlabel('Sample Number', fontsize=12)
    plt.ylabel('Power (ADC units)', fontsize=12)
    plt.legend(loc='upper right')
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"[+] Comparación guardada: {save_path}")
    
    plt.show()


def plot_analysis(traces, save_path=None):
    """Análisis detallado de las trazas."""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    ops = ['keygen', 'encaps', 'decaps']
    colors = ['blue', 'green', 'red']
    titles = ['Key Generation', 'Encapsulation', 'Decapsulation']
    
    # 1. Trazas promedio
    ax = axes[0, 0]
    for op, color, title in zip(ops, colors, titles):
        if traces[op]:
            avg = np.mean(traces[op], axis=0)
            ax.plot(avg, color=color, label=title, linewidth=1.5)
    ax.set_title('Average Power Traces')
    ax.set_xlabel('Sample')
    ax.set_ylabel('Power (ADC units)')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 2. Desviación estándar (variabilidad)
    ax = axes[0, 1]
    for op, color, title in zip(ops, colors, titles):
        if traces[op]:
            std = np.std(traces[op], axis=0)
            ax.plot(std, color=color, label=title, linewidth=1.5)
    ax.set_title('Trace Variability (Standard Deviation)')
    ax.set_xlabel('Sample')
    ax.set_ylabel('Standard Deviation')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 3. Potencia máxima por muestra
    ax = axes[1, 0]
    for op, color, title in zip(ops, colors, titles):
        if traces[op]:
            max_vals = np.max(traces[op], axis=0)
            ax.plot(max_vals, color=color, label=title, linewidth=1.5)
    ax.set_title('Maximum Power per Sample')
    ax.set_xlabel('Sample')
    ax.set_ylabel('Max Power')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 4. Boxplot de distribución
    ax = axes[1, 1]
    data = []
    labels_box = []
    for op, title in zip(ops, titles):
        if traces[op]:
            all_values = np.concatenate(traces[op])
            data.append(all_values)
            labels_box.append(title)
    
    bp = ax.boxplot(data, labels=labels_box, patch_artist=True)
    for patch, color in zip(bp['boxes'], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.5)
    ax.set_title('Power Distribution')
    ax.set_ylabel('Power (ADC units)')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"[+] Análisis guardado: {save_path}")
    
    plt.show()


# ============================================================================
# FUNCIÓN PRINCIPAL
# ============================================================================

def main():
    """Función principal."""
    print("=" * 60)
    print("  ML-KEM-512 Power Measurement Script")
    print("  ChipWhisperer + CWHUSKY (SAM4S)")
    print("=" * 60)
    print()
    print("Configuración:")
    print(f"  - Plataforma: {PLATFORM}")
    print(f"  - Osciloscopio: {SCOPETYPE}")
    print(f"  - Firmware: {FW_PATH}")
    print(f"  - Trazas a capturar: {NUM_TRACES}")
    print()
    
    try:
        # 1. Inicializar ChipWhisperer
        scope, target = setup_chipwhisperer()
        
        # 2. Capturar trazas
        traces = capture_all_operations(scope, target, NUM_TRACES)
        
        # 3. Cerrar conexiones
        scope.dis()
        target.dis()
        
        # 4. Generar gráficos
        print("\n[*] Generando gráficos...")
        
        plot_power_traces(traces, save_path='power_traces.png')
        plot_comparison(traces, save_path='power_comparison.png')
        plot_analysis(traces, save_path='power_analysis.png')
        
        # Resumen
        print("\n" + "=" * 60)
        print("RESUMEN")
        print("=" * 60)
        print(f"✓ Trazas de KeyGen: {len(traces['keygen'])}")
        print(f"✓ Trazas de Encaps:  {len(traces['encaps'])}")
        print(f"✓ Trazas de Decaps:  {len(traces['decaps'])}")
        print("\nArchivos generados:")
        print("  - power_traces.png     (trazas individuales)")
        print("  - power_comparison.png (comparación)")  
        print("  - power_analysis.png   (análisis detallado)")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n[!] Error: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
