/* SCIc UART driver for Renesas RX series chips
 * Designed to work with the AbstractWiring framework
 */

// TODO: Implement RTS/CTS

#ifndef SCIC_UART_H
#define SCIC_UART_H

#include <iodefine.h>
#include <AbstractWiring.h>
#include <AbstractSerial.h>
#include <RXGPIO.h>

#ifndef PCLK_CPU
// default RX210
#define PCLK_CPU 50000000UL
#endif

template <
	volatile struct st_sci0 & scicdrv,
	typename RXD_GPIO,  // Type RXGPIO_PIN<RXGPIO_PORT<RX_PORTn>, x> where n,x = Pn.x
	size_t tx_buffer_size,
	size_t rx_buffer_size
	>
class SCIC_UART : public AbstractSerial {
	private:
		volatile uint8_t txbuf[tx_buffer_size], rxbuf[rx_buffer_size];
		volatile size_t tx_head, tx_tail, rx_head, rx_tail;
		SERIAL_BREAK_CALLBACK _breakcb;

	public:
		__noinline
		SCIC_UART() {
			tx_head = 0;
			tx_tail = 0;
			rx_head = 0;
			rx_tail = 0;
			_breakcb = NULL;
		};

		__noinline
		void begin(unsigned long bitrate) {
			// Init SCI
			scicdrv.SCR.BYTE = 0x00;
			scicdrv.SIMR1.BIT.IICM = 0;  // Not I2C mode
			scicdrv.SPMR.BYTE = 0x00;  // CTS disabled, CKPH=0, CKPOL=0
			scicdrv.SMR.BYTE = 0x00;  // Async 8-bit, 1 stopbit, no parity, no multiproc, CKS=PCLK/1
			scicdrv.SCMR.BYTE = 0x00;  // Serial (not SmartCard), LSB-first
			scicdrv.SEMR.BYTE = BIT4|BIT5;  // 8 base clock cycles per 1-bit, Noise Cancellation on RXD
			// Set bitrate
			configClock(bitrate);  // This may modify BRR and SMR.CKS
			scicdrv.SCR.BYTE = BIT4 | BIT6;  // RX enabled & RXIE enabled - we're now live
		};

		__noinline
		void configClock(unsigned long bitrate) {
			// Requires PCLK_CPU definition
			uint8_t scr = scicdrv.SCR.BYTE;
			scicdrv.SCR.BYTE &= ~(BIT2 | BIT4 | BIT5 | BIT6 | BIT7);
			scicdrv.SEMR.BIT.ABCS = 1;  // 8 clocks per bit for high bitrates

			int brr;
			// Attempt #1 - PCLK/1
			brr = (PCLK_CPU / (16 * bitrate)) - 1; // 2^(2n-1) = 0.5
			if (brr < 256) {
				scicdrv.SMR.BIT.CKS = 0;
				scicdrv.BRR = brr;
				scicdrv.SCR.BYTE = scr;
				return;
			}
			// Attempt #2 - PCLK/4
			brr = (PCLK_CPU / (32 * 2 * bitrate)) - 1; // 2^(2n-1) = 2
			if (brr < 256) {
				scicdrv.SMR.BIT.CKS = 1;
				scicdrv.BRR = brr;
				scicdrv.SCR.BYTE = scr;
				return;
			}
			// Attempt #3 - PCLK/16
			brr = (PCLK_CPU / (32 * 8 * bitrate)) - 1; // 2^(2n-1) = 8
			if (brr < 256) {
				scicdrv.SMR.BIT.CKS = 2;
				scicdrv.BRR = brr;
				scicdrv.SCR.BYTE = scr;
				return;
			}
			// Attempt #4 - PCLK/64
			brr = (PCLK_CPU / (32 * 32 * bitrate)) - 1; // 2^(2n-1) = 32
			if (brr < 256) {
				scicdrv.SMR.BIT.CKS = 3;
				scicdrv.BRR = brr;
				scicdrv.SCR.BYTE = scr;
				return;
			}
			// Else .... Can't find a valid BRR/CKS config.
			scicdrv.SCR.BYTE = scr;
		};

		__noinline
		void end(void) {
			scicdrv.SCR.BYTE = 0x00;  // Disable SCI
		};

		int available(void) { return (rx_buffer_size + rx_head - rx_tail) % rx_buffer_size; };
		int peek(void) { if (rx_head == rx_tail) return -1; return rxbuf[rx_tail]; };
		void flush(void) { while (tx_head != tx_tail); };

		__noinline
		int read(void) {
			if (rx_head == rx_tail)
				return -1;

			uint8_t c = rxbuf[rx_tail];
			rx_tail = (size_t)(rx_tail + 1) % rx_buffer_size;
			return c;
		};

		__noinline
		size_t write(uint8_t c) {
			size_t i = (tx_head + 1) % tx_buffer_size;

			// If the output buffer is full, we must wait.
			if (i == tx_tail) {
				// What to do next?
				// First: Is the peripheral suspended?
				if (!scicdrv.SCR.BIT.RE)
					return 0;  // Peripheral not running

				// Second, are interrupts disabled?
				uint32_t psw = __builtin_rx_mvfc(0);  // PSW
				if ( !(psw & 0x00010000) )
					return 0;  // Interrupts not enabled!
				// NOTE: We do not check to see whether the current PSW Interrupt Priority Level
				// is too high for the SCI interrupt to occur, since there's no easy, library-portable
				// way to ascertain what the current Interrupt Priority Level is for the peripheral.

				// Else... just wait
				while (i == tx_tail)
					;
			}

			txbuf[tx_head] = c;
			tx_head = i;
			scicdrv.SCR.BYTE |= BIT5 | BIT7;  // Activate TX, TIE
			return 1;
		};

		operator bool() {  // Is the peripheral enabled?
			if (scicdrv.SCR.BIT.RE)
				return true;
			return false;
		};

		__noinline
		void isr_send_char(void) {  // Utility function used by TX ISR
			if (tx_head == tx_tail) {
				// All done; disable TX/TIE
				scicdrv.SCR.BYTE &= ~(BIT5 | BIT7);
			}
			scicdrv.TDR = txbuf[tx_tail];
			tx_tail = (tx_tail + 1) % tx_buffer_size;
		};

		__noinline
		void isr_get_char(void) {   // Utility function used by RX ISR
			uint8_t c = scicdrv.RDR;

			if (rx_head == rx_tail) {
				return;
			}
			rxbuf[rx_head] = c;
			rx_head = (rx_head + 1) % rx_buffer_size;
		};

		__noinline
		void isr_check_break(void) {  // Framing error
			if (_breakcb != NULL) {
				if (RXD_GPIO::read() == 0)  // RXD=0 == BREAK condition
					_breakcb();
			}
		};

		boolean hasExtendedAPI(void) { return true; };

		__noinline
		void set7Bit(boolean yn) {
			uint8_t scr = scicdrv.SCR.BYTE;
			scicdrv.SCR.BYTE &= ~(BIT2 | BIT4 | BIT5 | BIT6 | BIT7);

			if (yn)
				scicdrv.SMR.BIT.CHR = 1;  // 7-bit
			else
				scicdrv.SMR.BIT.CHR = 0;  // 8-bit

			scicdrv.SCR.BYTE = scr;
		};

		__noinline
		void setStopBits(int s) {
			uint8_t scr = scicdrv.SCR.BYTE;
			scicdrv.SCR.BYTE &= ~(BIT2 | BIT4 | BIT5 | BIT6 | BIT7);

			if (s == 1)
				scicdrv.SMR.BIT.STOP = 0;
			if (s == 2)
				scicdrv.SMR.BIT.STOP = 1;
			// else, do nothing... since stop bits < 0 or > 2 are invalid
			scicdrv.SCR.BYTE = scr;
		};

		__noinline
		void setParity(enum SerialParity par) {
			uint8_t scr = scicdrv.SCR.BYTE;
			scicdrv.SCR.BYTE &= ~(BIT2 | BIT4 | BIT5 | BIT6 | BIT7);

			switch (par) {
				case PARITY_NONE:
					scicdrv.SMR.BIT.PE = 0;
					break;
				case PARITY_ODD:
					scicdrv.SMR.BIT.PM = 1;
					scicdrv.SMR.BIT.PE = 1;
					break;
				case PARITY_EVEN:
					scicdrv.SMR.BIT.PM = 0;
					scicdrv.SMR.BIT.PE = 1;
					break;
			}
			scicdrv.SCR.BYTE = scr;
		};

		__noinline
		void attachBreakInterrupt(SERIAL_BREAK_CALLBACK cb) {
			_breakcb = cb;
		};

		__noinline
		void detachBreakInterrupt(void) { _breakcb = NULL; };

		#define SERIAL_HAS_FLOW_CONTROL 1
		__noinline
		void setFlowControl(boolean yn) {
			uint8_t scr = scicdrv.SCR.BYTE;
			scicdrv.SCR.BYTE &= ~(BIT2 | BIT4 | BIT5 | BIT6 | BIT7);

			/* RTS is always enabled, but this will activate CTS function for TX
			 * Note it's up to the user to set the port modes correctly for this SCI peripheral's
			 * RTS and CTS pins.
			 */
			if (yn)
				scicdrv.SPMR.BIT.CTSE = 1;
			else
				scicdrv.SPMR.BIT.CTSE = 0;

			scicdrv.SCR.BYTE = scr;
		}
};



#endif /* SCIC_UART_H */
