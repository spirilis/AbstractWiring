# Makefile to do a test compile using msp430-elf-gcc
#

TARGETMCU	?= msp430g2553

CROSS		:= msp430-elf-
CXX		:= $(CROSS)g++
CFLAGS		:= -Os -Wall -Werror -mmcu=$(TARGETMCU)
CFLAGS		+= -fdata-sections -ffunction-sections -Wl,--gc-sections
CFLAGS		+= -fno-exceptions -fno-rtti
CFLAGS		+= -I. -I.. -I../../../AbstractWiring/
LDFLAGS		:=

TEST		:= test
TESTFILES	:= test.cpp
UART		:= uart
UARTFILES	:= uart.cpp
SPI		:= spi
SPIFILES	:= spi.cpp
SPITRANS	:= spitrans
SPITRANSFILES	:= spitrans.cpp
SRCFILES	:= ../*.cpp ../../../AbstractWiring/*.cpp

all:		$(TEST).elf $(UART).elf $(SPI).elf $(SPITRANS).elf

$(TEST).elf:
	$(CXX) $(CFLAGS) -o $(TEST).elf $(SRCFILES) $(TESTFILES) $(LDFLAGS)
$(UART).elf:
	$(CXX) $(CFLAGS) -o $(UART).elf $(SRCFILES) $(UARTFILES) $(LDFLAGS)
$(SPI).elf:
	$(CXX) $(CFLAGS) -DSPI_ENABLE_EXTENDED_API -o $(SPI).elf $(SRCFILES) $(SPIFILES) $(LDFLAGS)
$(SPITRANS).elf:
	$(CXX) $(CFLAGS) -o $(SPITRANS).elf $(SRCFILES) $(SPITRANSFILES) $(LDFLAGS)

clean:
	rm -f *.elf
