import pygame
import time

pygame.init()
pygame.joystick.init()

js = pygame.joystick.Joystick(0)
js.init()

print("Controller:", js.get_name())

while True:
    pygame.event.pump()

    for i in range(js.get_numaxes()):
        print(f"Axis {i}: {js.get_axis(i): .3f}")

    print("-------------------")
    time.sleep(0.5)