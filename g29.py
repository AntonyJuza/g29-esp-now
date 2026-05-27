import pygame
import serial
import time

# ===== CONFIG =====
SERIAL_PORT = "/dev/ttyUSB0" #or "COMx" for windows, where x is the port number
BAUD = 115200

MAX_SPEED = 100
STEERING_GAIN = 55
DEADZONE = 0.05

# Clutch config
CLUTCH_AXIS = 3       # G29 clutch pedal axis
CLUTCH_THRESHOLD = 0.5 # how far clutch must be pressed to allow gear change (0-1)

# Throttle trim: compensates for motors not starting at the same point.
# Positive values push the motor harder in that direction.
# Adjust these until both wheels start moving at the same pedal input.
TRIM_LEFT_FWD  = 0   # trim added to left motor when going forward  (try 3-8)
TRIM_LEFT_REV  = 0   # trim added to left motor when going reverse  (try -3 to -8)
TRIM_RIGHT_FWD = 0   # trim added to right motor when going forward (try 3-8)
TRIM_RIGHT_REV = 0   # trim added to right motor when going reverse (try -3 to -8)

# Gear speed limits as a fraction of MAX_SPEED (0 to 1.0)
GEAR_LIMITS = {
    0: 0.0,   # Neutral
    1: 0.20,  # Gear 1: 20% speed
    2: 0.40,  # Gear 2: 40% speed
    3: 0.60,  # Gear 3: 60% speed
    4: 0.80,  # Gear 4: 80% speed
    5: 0.90,  # Gear 5: 90% speed
    6: 1.00,  # Gear 6: 100% speed
}

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

# ===== STATE =====
shifter_detected = False
current_gear = 6  # Start with Gear 6 (full speed) by default
last_gear = -1
clutch_was_engaged = False

# ===== MAIN LOOP =====
while True:

    pygame.event.pump()

    # ===== READ CLUTCH =====
    clutch_raw = js.get_axis(CLUTCH_AXIS)       # +1 released, -1 fully pressed
    clutch_pressed = (1 - clutch_raw) / 2       # normalize to 0..1
    clutch_engaged = clutch_pressed >= CLUTCH_THRESHOLD

    # ===== READ GEAR =====
    pressed_gear = None
    for i in range(12, 18):
        if js.get_button(i):
            pressed_gear = i - 11  # Button 12 -> Gear 1, ..., Button 17 -> Gear 6
            break

    # Gear only changes while clutch is held
    if clutch_engaged:
        if pressed_gear is not None:
            shifter_detected = True
            current_gear = pressed_gear
        elif shifter_detected:
            current_gear = 0  # Neutral
    # If clutch released, gear stays locked to whatever was last selected

    if not shifter_detected and not clutch_was_engaged:
        current_gear = 6  # Default full speed if shifter never used

    if clutch_engaged != clutch_was_engaged:
        state = "ENGAGED" if clutch_engaged else "RELEASED"
        print(f"Clutch: {state}")
        clutch_was_engaged = clutch_engaged

    if current_gear != last_gear:
        gear_name = "Neutral" if current_gear == 0 else f"Gear {current_gear}"
        limit_pct = int(GEAR_LIMITS[current_gear] * 100)
        print(f"Transmission: {gear_name} (Max Speed: {limit_pct}%)")
        last_gear = current_gear

    # ===== READ AXES =====
    steering = js.get_axis(0)
    accel = js.get_axis(1)
    brake = js.get_axis(2)

    # ===== DEADZONE =====
    steering = apply_deadzone(steering, DEADZONE)

    # ===== PEDAL NORMALIZATION =====
    # released = +1
    # pressed  = -1

    current_limit = GEAR_LIMITS[current_gear] * MAX_SPEED
    throttle = ((1 - accel) / 2) * current_limit
    reverse  = ((1 - brake) / 2) * current_limit

    speed = throttle - reverse

    # ===== STEERING SCALE =====
    steering = steering * STEERING_GAIN

    # ===== DIFFERENTIAL DRIVE MIX =====
    if current_gear == 0:
        left = 0
        right = 0
    else:
        left  = speed + steering
        right = speed - steering

    # ===== THROTTLE TRIM =====
    # Apply per-side, per-direction trim only when the motor is actually being driven
    if left > 0:
        left += TRIM_LEFT_FWD
    elif left < 0:
        left += TRIM_LEFT_REV

    if right > 0:
        right += TRIM_RIGHT_FWD
    elif right < 0:
        right += TRIM_RIGHT_REV

    left = clamp(left)
    right = clamp(right)

    send(left, -right)

    time.sleep(0.05)