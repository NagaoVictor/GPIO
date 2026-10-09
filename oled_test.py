python3 -c "
import board
import digitalio
import adafruit_ssd1306

i2c = board.I2C()  # Usa o barramento padrão do Raspberry Pi
# Tenta 0x3c, se falhar tente 0x3d
try:
    oled = adafruit_ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3C)
except:
    oled = adafruit_ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3D)

oled.fill(1)  # Acende todos os pixels da tela
oled.show()
print('Display aceso via Python!')
"