import pygame
import serial
import time

# ===== CONFIG =====
SERIAL_PORT = "/dev/ttyUSB0" #or "COMx" for windows, where x is the port number
BAUD = 115200

MAX_SPEED = 100
STEERING_GAIN = 55
DEADZONE = 0.05

# ===== SERIAL =====
ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
time.sleep(2)

# ===== PYGAME =====
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No G29 detected")
    exit()

js = pygame.joystick.Joystick(0)
js.init()

print("Connected:", js.get_name())

# ===== HELPERS =====
def clamp(v, mn=-100, mx=100):
    return max(mn, min(mx, v))

def apply_deadzone(v, dz):
    if abs(v) < dz:
        return 0
    return v

def send(left, right):
    msg = f"{int(left)} {int(right)}\n"
    ser.write(msg.encode())
    print(msg.strip())

# ===== MAIN LOOP =====
while True:

    pygame.event.pump()

    # ===== READ AXES =====
    steering = js.get_axis(0)
    accel = js.get_axis(1)
    brake = js.get_axis(2)

    # ===== DEADZONE =====
    steering = apply_deadzone(steering, DEADZONE)

    # ===== PEDAL NORMALIZATION =====
    # released = +1
    # pressed  = -1

    throttle = ((1 - accel) / 2) * MAX_SPEED
    reverse  = ((1 - brake) / 2) * MAX_SPEED

    speed = throttle - reverse

    # ===== STEERING SCALE =====
    steering = steering * STEERING_GAIN

    # ===== DIFFERENTIAL DRIVE MIX =====
    left  = speed + steering
    right = speed - steering

    left = clamp(left)
    right = clamp(right)

    send(left, right)

    time.sleep(0.05)