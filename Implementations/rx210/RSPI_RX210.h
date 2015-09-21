/*
 * RSPI_RX210.h
 *
 *  Created on: Sep 8, 2015
 *      Author: ebrundick
 */

#ifndef RSPI_RX210_H
#define RSPI_RX210_H

#include <SPI.h>
#include <iodefine.h>

#define F_CPU 50000000UL
#define PCLK_CPU (F_CPU / (1 << ((SYSTEM.SCKCR.LONG & 0x00000F00) >> 8)))

template <
	volatile struct st_rspi & rspidrv
>
class RSPI_RX210 : public SPIClass {
	private:
		SPISettings _settings, _settings_old;
		boolean _transaction_semaphore;
		int _mask_irq;

	public:
		__noinline
		void begin(void) {
			_mask_irq = 0;
			rspidrv.SPCR.BYTE = BIT0 | BIT3;  // 3-wire SPI, Full-Duplex, no interrupts, Disabled
			rspidrv.SPPCR.BYTE = BIT5;  // MOSI at rest = 0
			rspidrv.SPSCR.BYTE = 0x00;  // Repeat SPCMD0 continuously
			configClock(_settings._clock);
			rspidrv.SPDCR.BYTE = 0x00;  // Simple 1-frame buffer mode
			rspidrv.SPCKD.BYTE = 0x00;  // Minimal delay before SCK generation
			rspidrv.SPCR2.BYTE = 0x00;  // No parity
			configMode(_settings._bitorder, _settings._datamode);
			rspidrv.SPCR.BIT.SPE = 1;
			// Ready to roll!
		};

		__noinline
		void begin(SPISettings s) {
			_settings.copy(s);
			begin();
		}

		__noinline
		void configClock(uint32_t bitrate) {
			uint32_t i, thisbr;
			for (i = 0; i < 256; i++) {
				thisbr = PCLK_CPU / (2 * (i+1));
				if (thisbr <= bitrate) {
					rspidrv.SPBR = (uint8_t)i;
					return;
				}
			}
			rspidrv.SPBR = 255;  // SPBR happens not to be bitfielded.
		};

		__noinline
		void configMode(uint8_t bitorder, uint8_t mode) {
			uint16_t cmd = 0x0000;
			if (bitorder != MSBFIRST)
				cmd |= BITC;  // RSPI LSB First
			switch (mode) {
				case SPI_MODE1:
					cmd |= BIT0;  // CPOL=0, CPHA=1
					break;
				case SPI_MODE2:
					cmd |= BIT1;  // CPOL=1, CPHA=0
					break;
				case SPI_MODE3:
					cmd |= BIT0 | BIT1;  // CPOL=1, CPHA=1
					break;
				default: // SPI mode0 = CPOL=0, CPHA=0
					break;
			}
			rspidrv.SPCMD0.WORD = cmd;
		};

		__noinline
		void end(void) {
			rspidrv.SPCR.BYTE = 0x00;  // Disable
		};

		__noinline
		void setBitOrder(unsigned int msblsb) {
			_settings._bitorder = msblsb;
			rspidrv.SPCR.BIT.SPE = 0;
			configMode(_settings._bitorder, _settings._datamode);
			rspidrv.SPCR.BIT.SPE = 1;
		};

		__noinline
		void setDataMode(unsigned int mode) {
			_settings._datamode = mode;
			rspidrv.SPCR.BIT.SPE = 0;
			configMode(_settings._bitorder, _settings._datamode);
			rspidrv.SPCR.BIT.SPE = 1;
		}

		__noinline
		void setClockDivider(int clockDiv) {
			_settings._clock = PCLK_CPU / clockDiv;
			configClock(_settings._clock);
		};

		// Data transfer
		#define RSPI_SPCMD_FRAMESIZE_BITS (BIT8 | BIT9 | BITA | BITB)
		__noinline
		uint8_t transfer(uint8_t inb) {
			uint16_t cmd = rspidrv.SPCMD0.WORD;
			if ( (cmd & RSPI_SPCMD_FRAMESIZE_BITS) != BITA ) {
				// SPI currently not set to 8-bit mode; change this
				cmd &= ~RSPI_SPCMD_FRAMESIZE_BITS;
				cmd |= BITA;  // 8 bits per transfer
				rspidrv.SPCMD0.WORD = cmd;
			}
			rspidrv.SPDR.WORD.H = (uint16_t)inb;
			while (rspidrv.SPSR.BIT.IDLNF)
				;  // Waiting for RSPI to go Idle...
			return (uint8_t)rspidrv.SPDR.WORD.H;
		};

		__noinline
		uint16_t transfer9(uint16_t inw) {
			uint16_t cmd = rspidrv.SPCMD0.WORD;
			if ( (cmd & RSPI_SPCMD_FRAMESIZE_BITS) != BITB ) {
				// SPI currently not set to 9-bit mode; change this
				cmd &= ~RSPI_SPCMD_FRAMESIZE_BITS;
				cmd |= BITB;  // 9 bits per transfer
				rspidrv.SPCMD0.WORD = cmd;
			}
			rspidrv.SPDR.WORD.H = (uint16_t)inw;
			while (rspidrv.SPSR.BIT.IDLNF)
				;  // Waiting for RSPI to go Idle...
			return (uint16_t)rspidrv.SPDR.WORD.H;
		};

		__noinline
		uint16_t transfer16(uint16_t inw) {
			uint16_t cmd = rspidrv.SPCMD0.WORD;
			if ( (cmd & RSPI_SPCMD_FRAMESIZE_BITS) != RSPI_SPCMD_FRAMESIZE_BITS ) {
				// SPI currently not set to 9-bit mode; change this
				cmd |= RSPI_SPCMD_FRAMESIZE_BITS;  // 16 bits per transfer
				rspidrv.SPCMD0.WORD = cmd;
			}
			rspidrv.SPDR.WORD.H = (uint16_t)inw;
			while (rspidrv.SPSR.BIT.IDLNF)
				;  // Waiting for RSPI to go Idle...
			return (uint16_t)rspidrv.SPDR.WORD.H;
		};

		bool hasExtendedAPI(void) { return true; };

		__noinline
		bool beginTransaction(SPISettings settings) {
			// for atomicity in semaphore check
			__builtin_rx_clrpsw(8);  // Disable Global Interrupts
			if (_transaction_semaphore) {
				__builtin_rx_setpsw(8);  // Enable interrupts
				return false;
			}
			_transaction_semaphore = true;
			// TODO: Disable masked IRQ if _mask_irq is set to something valid
			if (_mask_irq != 255)
				__builtin_rx_setpsw(8);  // Enable interrupts

			_settings_old.copy(_settings);
			_settings.copy(settings);
			configClock(_settings._clock);
			configMode(_settings._bitorder, _settings._datamode);
			return true;
		};

		__noinline
		void endTransaction(void) {
			__builtin_rx_clrpsw(8);  // Disable Global Interrupts
			_transaction_semaphore = false;
			// TODO: Unmask gpio IRQ

			// Restore SPI settings under PSW(8)=CLR protection in case an IRQ fires which
			// uses SPI and expects the default SPISettings to be intact.
			_settings.copy(_settings_old);
			configClock(_settings._clock);
			configMode(_settings._bitorder, _settings._datamode);
			__builtin_rx_setpsw(8);  // Enable interrupts
		};

		__noinline
		void usingInterrupt(int pin) { ; }; // TODO
};



#endif /* RSPI_RX210_H */
