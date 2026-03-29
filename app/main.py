#!/bin/python3
import os
import socket
import threading
import time
import sys
import curses
from dotenv import load_dotenv

# Load environment variables from .env
load_dotenv()

# Update to your HC-06 MAC address
MAC_ADDRESS = os.getenv('MAC_ADDRESS')
if not MAC_ADDRESS:
    print("Error: MAC_ADDRESS not found in .env file.")
    sys.exit(1)
PORT = 1  # RFCOMM Channel 1 is standard for HC-06

telemetry_data = "Waiting for data..."
running = True

def receive_data(bt_socket):
    global telemetry_data, running
    buffer = ""
    try:
        while running:
            data = bt_socket.recv(1024)
            if not data:
                break
            
            buffer += data.decode('ascii', errors='ignore')
            while '\n' in buffer:
                line, buffer = buffer.split('\n', 1)
                telemetry_data = line.strip()
    except Exception:
        pass

def run_curses(stdscr, bt_socket):
    global telemetry_data, running
    
    # Setup curses environment
    curses.curs_set(1)
    stdscr.nodelay(True)
    stdscr.clear()
    
    input_str = ""
    status_msg = ""
    
    while running:
        stdscr.addstr(0, 0, "--- Robot Control ---")
        stdscr.addstr(1, 0, "Commands: <left> <right> (Values -100 to 100)")
        stdscr.addstr(2, 0, "Example: '50 50' or 'q' to quit")
        stdscr.addstr(3, 0, "-" * 50)
        
        # Telemetry line (always updates without breaking typing)
        stdscr.move(4, 0)
        stdscr.clrtoeol()
        stdscr.addstr(4, 0, f"TELEMETRY: {telemetry_data}")
        
        stdscr.addstr(5, 0, "-" * 50)
        
        # Status messages
        stdscr.move(6, 0)
        stdscr.clrtoeol()
        if status_msg:
            stdscr.addstr(6, 0, status_msg)
            
        # The input line
        stdscr.move(8, 0)
        stdscr.clrtoeol()
        stdscr.addstr(8, 0, f"Enter speeds: {input_str}")
        
        stdscr.refresh()
        
        try:
            char = stdscr.getch()
            if char != -1:
                # Enter keys
                if char in (curses.KEY_ENTER, 10, 13):
                    cmd = input_str.strip()
                    input_str = ""
                    
                    if cmd.lower() in ['q', 'quit', 'exit']:
                        bt_socket.send(b"0 0\n")
                        running = False
                        break
                    
                    parts = cmd.split()
                    if len(parts) == 2:
                        try:
                            left = max(-100, min(100, int(parts[0])))
                            right = max(-100, min(100, int(parts[1])))
                            command = f"{left} {right}\n"
                            bt_socket.send(command.encode('ascii'))
                            status_msg = f"Sent successfully: {command.strip()}"
                        except ValueError:
                            status_msg = "Error: Invalid numbers."
                    elif len(parts) > 0:
                        status_msg = "Error: Invalid format. Use 'Left Right'."
                # Backspace
                elif char in (curses.KEY_BACKSPACE, 127, 8):
                    input_str = input_str[:-1]
                # Normal typing
                elif 32 <= char <= 126:
                    input_str += chr(char)
        except Exception:
            pass
            
        time.sleep(0.01)  # brief sleep to prevent CPU hogging

def main():
    try:
        print(f"Connecting to Robot at {MAC_ADDRESS}...")
        with socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM) as bt_socket:
            bt_socket.connect((MAC_ADDRESS, PORT))
            
            rx_thread = threading.Thread(target=receive_data, args=(bt_socket,), daemon=True)
            rx_thread.start()
            
            # Start the UI
            curses.wrapper(run_curses, bt_socket)
            
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"\nConnection Failed: {e}")

if __name__ == "__main__":
    main()
