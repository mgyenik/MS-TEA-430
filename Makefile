TARGET = main
MCU = msp430g2231
SOURCES = main.c state.c gpio.c
CFLAGS = -mmcu=$(MCU) -g -Os -Wall -Wunused $(INCLUDES)
ASFLAGS = -mmcu=$(MCU) -x assembler-with-cpp -Wa,-gstabs
LDFLAGS = -mmcu=$(MCU)

CC      	= msp430-gcc
LD      	= msp430-ld
AR      	= msp430-ar
AS      	= msp430-gcc
GASP    	= msp430-gasp
NM      	= msp430-nm
OBJCOPY 	= msp430-objcopy
RM      	= rm -f
MKDIR		= mkdir -p

# file that includes all dependencies
DEPEND = $(SOURCES:.c=.d)

OBJECTS = $(notdir $(SOURCES:.c=.o))

all: $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-$(RM) *.o *.elf *.hex

flash: $(TARGET).elf
	mspdebug rf2500 "prog $(TARGET).elf"

.PHONY: all clean flash
