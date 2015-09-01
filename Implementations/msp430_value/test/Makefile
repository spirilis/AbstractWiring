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
SRCFILES	:= ../*.cpp ../../../AbstractWiring/*.cpp

all:		$(TEST).elf $(UART).elf

$(TEST).elf:
	$(CXX) $(CFLAGS) -o $(TEST).elf $(SRCFILES) $(TESTFILES) $(LDFLAGS)
$(UART).elf:
	$(CXX) $(CFLAGS) -o $(UART).elf $(SRCFILES) $(UARTFILES) $(LDFLAGS)

clean:
	rm -f *.elf
