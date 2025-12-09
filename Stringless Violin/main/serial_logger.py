#!/usr/bin/env python3
"""
Serial logger for Stringless Violin notes.
Reads serial data from ESP32-S3 and appends note strings to notes.txt
"""

import serial
import sys
import os
from datetime import datetime

# Configuration
SERIAL_PORT = "COM10"  # Match your idf.py settings
BAUD_RATE = 115200
#OUTPUT_FILE = "Stringless Violin/main/notes.txt"
#OUTPUT_FILE = "D:/SeniorDesign/GroupGit/Senior-Design-Stringless-Violin/Stringless Violin/main/notes.txt" #Working
OUTPUT_FILE = "D:/SeniorDesign/GroupGit/Senior-Design-Stringless-Violin/App/live/notes.txt"

def main():
    try:
        # Open serial connection
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connected to {SERIAL_PORT} at {BAUD_RATE} baud")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Writing to {OUTPUT_FILE}")
        print("Press Ctrl+C to stop...\n")
        
        # Ensure output file exists
        os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not line:
                    continue
                
                # Look for "Note: " prefix from your C code
                if "Note: " in line:
                    # Extract just the note (e.g., "C#4")
                    note = line.split("Note: ")[-1].strip()
                    
                    # Log to console
                    timestamp = datetime.now().strftime('%H:%M:%S')
                    print(f"[{timestamp}] {note}")
                    
                    # Append to file
                    try:
                        with open(OUTPUT_FILE, "a") as f:
                            f.write(f"{note}\n")
                            f.flush()
                    except IOError as e:
                        print(f"[ERROR] Failed to write to {OUTPUT_FILE}: {e}")
    
    except serial.SerialException as e:
        print(f"[ERROR] Serial connection failed: {e}")
        print(f"Check that COM port is correct and not in use by another program")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n[STOP] Logging stopped by user")
        ser.close()
        sys.exit(0)
    except Exception as e:
        print(f"[ERROR] Unexpected error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()