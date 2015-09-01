# Makefile to do a test compile using msp430-elf-gcc
#

TARGETMCU	?= msp430g2553

CROSS		:= msp430-elf-
CXX		:= $(CROSS)g++
CFLAGS		:= -Os -Wall -Werror -g -mmcu=$(TARGETMCU)
CFLAGS		+= -fdata-sections -ffunction-sections -Wl,--gc-sections
CFLAGS		+= -fno-exceptions -fno-rtti
CFLAGS		+= -I. -I.. -I../../../AbstractWiring/
LDFLAGS		:=

PROG		:= test
SRCFILES	:= *.cpp ../*.cpp ../../../AbstractWiring/*.cpp

all:		$(PROG).elf

$(PROG).elf:
	$(CXX) $(CFLAGS) -o $(PROG).elf $(SRCFILES) $(LDFLAGS)

clean:
	rm -f $(PROG).elf
