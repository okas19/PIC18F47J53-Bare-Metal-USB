import tkinter as tk
import hid
import threading
import time

# ---------- HID Device Information ----------
VID = 0x1234
PID = 0x5678
REPORT_SIZE = 2          # Windows padded 1-byte data into 2 bytes

# ---------- Global Variables ----------
device = None
running = True
current_state = 0        # LED state (bit0: LED0, bit1: LED1)

# ---------- Find and Connect to HID Device ----------
def find_device():
    global device
    try:
        device = hid.device()
        device.open(VID, PID)
        device.set_nonblocking(False)  # <--- CRITICAL: Must be FALSE for blocking read!
        print(f"Device found: VID=0x{VID:04X}, PID=0x{PID:04X}", flush=True)
        return True
    except Exception as e:
        print(f"Device not found: {e}", flush=True)
        return False

# ---------- Send Data to Device (Single byte, no Report ID) ----------
def send_report(data_byte):
    global device
    if device:
        try:
            device.write([0x00, data_byte])   # 2 bytes (ID + data)
            print(f"Sent: 0x{data_byte:02X}")
        except Exception as e:
            print(f"Transmission error: {e}")

# ---------- Read LED Status From Device ----------
def reader_thread():
    global device, running
    while running:
        if device:
            try:
                # Since the device is in blocking mode, no timeout_ms is required.
                # As long as the PIC18F doesn't send data, this thread blocks without consuming CPU.
                # The moment data arrives (every 32ms), it instantly wakes up and processes it.
                data = device.read(1) 
                
                if data and len(data) >= 1:
                    reported_state = data[0]  # Raw data is at index 0
                    
                    # flush=True guarantees immediate print to console
                    print(f"Raw data read: 0x{reported_state:02X}", flush=True) 
                    
                    root.after(0, update_led_indicators,
                               reported_state & 0x01,
                               (reported_state >> 1) & 0x01)
            except Exception as e:
                print(f"Read error: {e}", flush=True)
                time.sleep(1)  # Prevent infinite loop spamming on persistent errors
                
# ---------- Update GUI Indicators ----------
def update_led_indicators(led0, led1):
    if led0:
        led0_label.config(text="LED 0: ● ON", fg="red")
    else:
        led0_label.config(text="LED 0: ○ OFF", fg="gray")
    if led1:
        led1_label.config(text="LED 1: ● ON", fg="red")
    else:
        led1_label.config(text="LED 1: ○ OFF", fg="gray")

# ---------- Button Actions (Independent Control) ----------
def led0_on():
    global current_state
    current_state |= 0x01
    send_report(current_state)

def led0_off():
    global current_state
    current_state &= ~0x01
    send_report(current_state)

def led1_on():
    global current_state
    current_state |= 0x02
    send_report(current_state)

def led1_off():
    global current_state
    current_state &= ~0x02
    send_report(current_state)

# ---------- Safe Program Exit ----------
def on_closing():
    global running, device
    running = False
    if device:
        device.close()
    root.destroy()

# ---------- Main GUI Setup ----------
root = tk.Tk()
root.title("HID LED Control Panel")
root.geometry("400x350")

led_frame = tk.Frame(root)
led_frame.pack(pady=20)

led0_label = tk.Label(led_frame, text="LED 0: ○ OFF", fg="gray", font=("Arial", 14))
led0_label.grid(row=0, column=0, padx=10)
led1_label = tk.Label(led_frame, text="LED 1: ○ OFF", fg="gray", font=("Arial", 14))
led1_label.grid(row=0, column=1, padx=10)

button_frame = tk.Frame(root)
button_frame.pack(pady=20)

btn_led0_on = tk.Button(button_frame, text="Turn ON LED 0", command=led0_on, bg="lightgreen", width=15)
btn_led0_on.grid(row=0, column=0, padx=5, pady=5)
btn_led0_off = tk.Button(button_frame, text="Turn OFF LED 0", command=led0_off, bg="lightcoral", width=15)
btn_led0_off.grid(row=0, column=1, padx=5, pady=5)
btn_led1_on = tk.Button(button_frame, text="Turn ON LED 1", command=led1_on, bg="lightgreen", width=15)
btn_led1_on.grid(row=1, column=0, padx=5, pady=5)
btn_led1_off = tk.Button(button_frame, text="Turn OFF LED 1", command=led1_off, bg="lightcoral", width=15)
btn_led1_off.grid(row=1, column=1, padx=5, pady=5)

status_label = tk.Label(root, text="Waiting for device...", fg="blue", font=("Arial", 10))
status_label.pack(pady=10)

root.protocol("WM_DELETE_WINDOW", on_closing)

if find_device():
    status_label.config(text="Connected to device. Listening...", fg="green")
    reader = threading.Thread(target=reader_thread, daemon=True)
    reader.start()
else:
    status_label.config(text="Device not found! Check connection.", fg="red")

root.mainloop()
