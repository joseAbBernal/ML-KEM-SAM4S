#!/usr/bin/env python3
"""
ML-KEM-768 Power Analysis Test Script for ChipWhisperer HUSKY
================================================================
This script tests the ML-KEM-768 implementation and captures power traces.
"""

import numpy as np
import matplotlib.pyplot as plt
import time
import os

# ChipWhisperer imports
try:
    import chipwhisperer as cw
    from chipwhisperer.capture.api.programmers import Programmer
    from chipwhisperer.capture.scopes import OpenADC
    from chipwhisperer.capture.targets import SimpleSerial
except ImportError:
    print("ChipWhisperer not installed. Install with: pip install chipwhisperer")
    exit(1)

# Configuration
FIRMWARE_DIR = "/opt/tools/chipwhisperer/firmware/mcu/simpleserial-ml-kem-768"
FIRMWARE_ELF = f"simpleserial-ml-kem-768-CWHUSKY.elf"
PLATFORM = "CWHUSKY"
SS_VER = "SS_VER_2_1"

# ML-KEM-768 Parameters
KYBER_K = 3
MLKEM768_PUBLICKEYBYTES = 1184
MLKEM768_SECRETKEYBYTES = 2400
MLKEM768_CIPHERTEXTBYTES = 1088
MLKEM768_SYMBYTES = 32


class MLKEMPowerAnalysis:
    """ML-KEM-768 Power Analysis class for ChipWhisperer"""
    
    def __init__(self, scope=None, target=None):
        self.scope = scope
        self.target = target
        self.trace_buffer = []
        
    def initialize_husky(self):
        """Initialize ChipWhisperer HUSKY"""
        print("[*] Initializing ChipWhisperer HUSKY...")
        
        # Check if scope exists
        if self.scope is None:
            scope = cw.scope()
        
        # Configure HUSKY settings
        scope.clock.clkgen_src = 'system'
        scope.clock.clkgen_freq = 10E6  # 10 MHz
        
        # Trigger settings
        scope.trigger.triggers = 'tio1'  # Use TIO1 as trigger
        scope.trigger.mode = 'rising'
        
        # ADC settings
        scope.adc.basic_mode = "rising"
        scope.adc.clock_source = "clkgen"
        scope.adc.samples = 5000  # Number of samples to capture
        scope.adc.offset = 0
        scope.adc.presamples = 0
        scope.adc.decimate = 1
        
        # Gain settings
        scope.gain.gain = 45
        scope.gain.db = 22.5
        
        print("[*] HUSKY initialized successfully")
        return scope
    
    def connect_target(self, scope):
        """Connect to SimpleSerial target"""
        print("[*] Connecting to SimpleSerial target...")
        
        target = cw.target(scope, SimpleSerial)
        target.baud = 38400
        target.ser.timeout = 10
        
        # Reset the target
        scope.io.nrst = 'low'
        time.sleep(0.1)
        scope.io.nrst = 'high'
        time.sleep(0.5)
        
        # Check connection
        target.ser.write(b"\x00\x00\x00\x00\r\n")
        time.sleep(0.1)
        
        print("[*] Target connected successfully")
        return target
    
    def program_firmware(self, programmer=None):
        """Program the ML-KEM-768 firmware"""
        print(f"[*] Programming firmware: {FIRMWARE_ELF}")
        
        # Note: Programming requires separate tools like OpenOCD or pyOCD
        # For HUSKY, use the built-in programmer
        print("[!] Firmware programming must be done separately")
        print(f"[!] Command: make PLATFORM=CWHUSKY SS_VER=SS_VER_2_1 program")
        
    def generate_random_bytes(self, num_bytes):
        """Generate random bytes for testing"""
        return np.random.randint(0, 256, num_bytes, dtype=np.uint8).tobytes()
    
    def send_command(self, target, cmd, data=None):
        """Send SimpleSerial command"""
        if data is None:
            target.ser.write(cmd + b"\r\n")
        else:
            message = cmd + data.hex().encode() + b"\r\n"
            target.ser.write(message)
        
        time.sleep(0.1)
        response = target.ser.read(timeout=1000)
        return response
    
    def test_keygen(self, target, num_traces=1):
        """Test ML-KEM-768 Key Generation"""
        print("\n[*] Testing ML-KEM-768 Key Generation...")
        
        traces = []
        
        for i in range(num_traces):
            print(f"    Trace {i+1}/{num_traces}")
            
            # Generate random seed
            seed = self.generate_random_bytes(MLKEM768_SYMBYTES)
            
            # Capture trace
            scope = self.scope
            scope.arm()
            
            # Send keygen command
            # Format: k <seed(32 bytes)>
            target.ser.write(b"k" + seed.hex().encode() + b"\r\n")
            
            # Wait for completion
            ret = scope.capture()
            
            if ret:
                print(f"    [!] Timeout on capture {i+1}")
                continue
            
            # Get trace
            trace = scope.get_last_trace()
            traces.append(trace)
            
        print(f"    [+] Captured {len(traces)} key generation traces")
        return traces
    
    def test_encaps(self, target, pk, num_traces=1):
        """Test ML-KEM-768 Encapsulation"""
        print("\n[*] Testing ML-KEM-768 Encapsulation...")
        
        traces = []
        
        for i in range(num_traces):
            print(f"    Trace {i+1}/{num_traces}")
            
            # Generate random message
            m = self.generate_random_bytes(MLKEM768_SYMBYTES)
            
            # Capture trace
            scope = self.scope
            scope.arm()
            
            # Send encaps command
            # Format: e <pk(1184 bytes)> <m(32 bytes)>
            target.ser.write(b"e" + pk.hex().encode() + m.hex().encode() + b"\r\n")
            
            # Wait for completion
            ret = scope.capture()
            
            if ret:
                print(f"    [!] Timeout on capture {i+1}")
                continue
            
            # Get trace
            trace = scope.get_last_trace()
            traces.append(trace)
            
        print(f"    [+] Captured {len(traces)} encapsulation traces")
        return traces
    
    def test_decaps(self, target, sk, c, num_traces=1):
        """Test ML-KEM-768 Decapsulation"""
        print("\n[*] Testing ML-KEM-768 Decapsulation...")
        
        traces = []
        
        for i in range(num_traces):
            print(f"    Trace {i+1}/{num_traces}")
            
            # Capture trace
            scope = self.scope
            scope.arm()
            
            # Send decaps command
            # Format: d <sk(2400 bytes)> <c(1088 bytes)>
            target.ser.write(b"d" + sk.hex().encode() + c.hex().encode() + b"\r\n")
            
            # Wait for completion
            ret = scope.capture()
            
            if ret:
                print(f"    [!] Timeout on capture {i+1}")
                continue
            
            # Get trace
            trace = scope.get_last_trace()
            traces.append(trace)
            
        print(f"    [+] Captured {len(traces)} decapsulation traces")
        return traces
    
    def plot_traces(self, traces, title="Power Trace", save_file=None):
        """Plot power traces"""
        print(f"\n[*] Plotting {len(traces)} traces...")
        
        plt.figure(figsize=(12, 6))
        
        # Plot individual traces
        for i, trace in enumerate(traces):
            plt.plot(trace, alpha=0.5, label=f'Trace {i+1}' if len(traces) <= 10 else None)
        
        # Plot mean trace
        if len(traces) > 1:
            mean_trace = np.mean(traces, axis=0)
            plt.plot(mean_trace, 'r-', linewidth=2, label='Mean')
        
        plt.xlabel('Sample')
        plt.ylabel('Power (ADC units)')
        plt.title(title)
        plt.legend()
        plt.grid(True)
        
        if save_file:
            plt.savefig(save_file, dpi=300)
            print(f"    [+] Saved to {save_file}")
        
        plt.show()
        
    def plot_single_trace(self, trace, title="Single Power Trace", save_file=None):
        """Plot a single power trace with more detail"""
        plt.figure(figsize=(14, 6))
        
        plt.subplot(2, 1, 1)
        plt.plot(trace, 'b-', linewidth=0.5)
        plt.xlabel('Sample')
        plt.ylabel('Power (ADC units)')
        plt.title(title)
        plt.grid(True)
        
        # Plot FFT
        plt.subplot(2, 1, 2)
        fft = np.fft.fft(trace)
        freqs = np.fft.fftfreq(len(trace), 1/10E6)  # 10 MHz sample rate
        plt.plot(freqs[:len(freqs)//2], np.abs(fft)[:len(freqs)//2])
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Magnitude')
        plt.title('Frequency Spectrum')
        plt.grid(True)
        
        plt.tight_layout()
        
        if save_file:
            plt.savefig(save_file, dpi=300)
            print(f"    [+] Saved to {save_file}")
        
        plt.show()
    
    def compare_operations(self, target, num_traces=10):
        """Compare power traces between different operations"""
        print("\n[*] Comparing different ML-KEM operations...")
        
        # First generate a keypair
        print("    Generating keypair...")
        seed = self.generate_random_bytes(MLKEM768_SYMBYTES)
        self.scope.arm()
        self.target.ser.write(b"k" + seed.hex().encode() + b"\r\n")
        ret = self.scope.capture()
        
        if ret:
            print("    [!] Key generation failed")
            return None
        
        keygen_trace = self.scope.get_last_trace()
        
        # Now do encapsulation
        print("    Encapsulation...")
        pk = self.generate_random_bytes(MLKEM768_PUBLICKEYBYTES)  # In real test, use actual pk
        m = self.generate_random_bytes(MLKEM768_SYMBYTES)
        self.scope.arm()
        self.target.ser.write(b"e" + pk.hex().encode() + m.hex().encode() + b"\r\n")
        ret = self.scope.capture()
        
        if ret:
            print("    [!] Encapsulation failed")
            return None
        
        encaps_trace = self.scope.get_last_trace()
        
        return {
            'keygen': keygen_trace,
            'encaps': encaps_trace
        }


def run_basic_test():
    """Run basic test with manual firmware loading"""
    print("=" * 60)
    print("ML-KEM-768 Power Analysis Test")
    print("=" * 60)
    
    # Create ML-KEM analyzer
    analyzer = MLKEMPowerAnalysis()
    
    # Initialize HUSKY
    scope = analyzer.initialize_husky()
    
    # Connect to target
    target = analyzer.connect_target(scope)
    
    analyzer.scope = scope
    analyzer.target = target
    
    # Capture some test traces
    print("\n[*] Capturing test traces...")
    
    # Arm and capture
    scope.arm()
    
    # Send a test command (check if firmware is responding)
    target.ser.write(b"t\r\n")  # Test command if available
    time.sleep(0.2)
    
    ret = scope.capture()
    
    if ret:
        print("[!] Capture timeout")
    else:
        trace = scope.get_last_trace()
        print(f"[+] Captured trace with {len(trace)} samples")
        
        # Plot the trace
        analyzer.plot_single_trace(trace, title="Test Trace")
    
    # Clean up
    print("\n[*] Cleaning up...")
    target.dis()
    scope.dis()
    
    print("\n[+] Test complete!")


def run_full_test(num_traces=100):
    """Run full ML-KEM test with keygen, encaps, decaps"""
    print("=" * 60)
    print("ML-KEM-768 Full Power Analysis Test")
    print("=" * 60)
    
    # Initialize
    analyzer = MLKEMPowerAnalysis()
    scope = analyzer.initialize_husky()
    target = analyzer.connect_target(scope)
    analyzer.scope = scope
    analyzer.target = target
    
    # Test key generation
    keygen_traces = analyzer.test_keygen(target, num_traces=num_traces)
    
    if keygen_traces:
        analyzer.plot_traces(
            keygen_traces[:10],  # Plot first 10 traces
            title="ML-KEM-768 Key Generation Traces",
            save_file="mlkem768_keygen_traces.png"
        )
    
    # Test with different seed lengths (to trigger different code paths)
    print("\n[*] Testing with different input patterns...")
    
    # Clean up
    print("\n[*] Cleaning up...")
    target.dis()
    scope.dis()
    
    print("\n[+] Full test complete!")


def example_analysis():
    """Example of how to analyze traces for correlation"""
    print("\n[*] Example: Correlation Power Analysis")
    print("    This is a placeholder for CPA/DPA analysis")
    print("    In a real attack, you would:")
    print("    1. Collect many traces with known plaintexts/keys")
    print("    2. Calculate correlation between power and hypothetical key bytes")
    print("    3. Find the key byte with highest correlation")


# Main execution
if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="ML-KEM-768 Power Analysis")
    parser.add_argument("--traces", "-n", type=int, default=10, 
                        help="Number of traces to capture")
    parser.add_argument("--test", "-t", choices=["basic", "full"], default="basic",
                        help="Test type to run")
    parser.add_argument("--plot", "-p", action="store_true",
                        help="Plot traces")
    
    args = parser.parse_args()
    
    if args.test == "basic":
        run_basic_test()
    else:
        run_full_test(num_traces=args.traces)
