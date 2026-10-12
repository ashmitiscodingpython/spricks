from machine import Pin, SPI

led1 = Pin(28, Pin.OUT)
led1.off()

from ST7735 import TFT, TFTColor

lite = Pin(17, Pin.OUT)
lite.on()

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

display.initg()
display.rgb(True)
display.fill(TFT.GRAY)
display.fillrect([39, 55], (50, 50), TFT.WHITE)
display.fillcircle([64, 80], 20, TFT.RED)

# led2 = Pin(4, Pin.OUT)
# button_pins = [5, 6, 7, 8, 12, 13, 14, 15]
# buttons = []
# btn_state = [False, False, False, False, False, False, False, False]
# buttons_ = ["W", "A","S", "D", "I", "J", "K", "L"]
# for p in button_pins:
#     buttons.append(Pin(p, Pin.IN, Pin.PULL_UP))
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