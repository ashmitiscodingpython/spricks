from machine import Pin, SPI, PWM
import time
from ST7735 import TFT, TFTColor
import framebuf

spi = SPI(
    0,
    baudrate=20_000_000,
    polarity=0,
    phase=0,
    sck=Pin(18),
    mosi=Pin(19)
)

display = TFT(
    spi,
    aReset=Pin(26),
    aCS=Pin(20),
    aDC=Pin(22)
)

lite = PWM(Pin(17, Pin.OUT))
lite.freq(1000)
lite.duty_u16(32768)

WIDTH = 128
HEIGHT = 160

buffer = bytearray(160 * 128 * 2)

canvas = framebuf.FrameBuffer(
    buffer, 128, 160, framebuf.RGB565
)



canvas.fill(0)
canvas.pixel(0, 0, 0xF800)
print(list(buffer[:2]))

display.initg()
display.rgb(True)
display.fill(display.WHITE)

def update_display():
    converted = bytearray(len(buffer))

    for i in range(0, len(buffer), 2):
        converted[i] = buffer[i + 1]
        converted[i + 1] = buffer[i]

    display.image(0, 0, WIDTH - 1, HEIGHT - 1, converted)

def draw_icon(name, ys=0, xs=0):
    exec(f"from generated import {name}")
    WIDTH: int = eval(f"{name}.WIDTH")
    HEIGHT: int = eval(f"{name}.HEIGHT")
    PIXELS: bytearray = eval(f"{name}.PIXELS")
    MASK: bytearray = eval(f"{name}.MASK")
    ys -= 12
    xs -= 12
    for y in range(HEIGHT):
        for x in range(WIDTH):
            i = y * WIDTH + x
            if MASK[i]:
                color = (PIXELS[2 * i + 1] << 8) | PIXELS[2 * i]
                canvas.pixel(ys + (HEIGHT - 1 - y), xs + x, color)

led1 = Pin(28, Pin.OUT)
led2 = Pin(4, Pin.OUT)
led1.off()

button_pins = [5, 6, 7, 8, 12, 13, 14, 15]
buttons = []
btn_state = [False, False, False, False, False, False, False, False]
buttons_ = ["W", "A", "S", "D", "I", "J", "K", "L"]
for p in button_pins:
    buttons.append(Pin(p, Pin.IN, Pin.PULL_UP))

while True:
    canvas.fill(0)
    if buttons[1].value():
        draw_icon("a_unpressed", 80, 64)
    else:
        draw_icon("a_pressed", 80, 64)
    print("tick")
    update_display()

# while True:
#     for i, button in enumerate(buttons):
#         pressed = not button.value()
#         btn_state[i] = pressed
#         if pressed:
#             print(buttons_[i])
#     if any(btn_state):
#         led2.on()
#     else:
#         led2.off()