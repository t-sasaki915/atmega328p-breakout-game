TARGET = Main

MCU = atmega328p

PROGRAMMER = usbasp

F_CPU = 1000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

CFLAGS = -Wall -Wextra -Wpedantic -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)

all: $(TARGET).hex $(TARGET).S

$(TARGET).elf: $(TARGET).S
	$(CC) $(CFLAGS) -o $(TARGET).elf -nostartfiles $(TARGET).S

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U flash:w:$(TARGET).hex:i

clean:
	del $(TARGET).elf $(TARGET).hex
