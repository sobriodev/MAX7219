/**
 * max7219.h
 *
 *  Created on: 24 Sep 2018
 *      Author: sobriodev
 */

#ifndef MAX7219_H_
#define MAX7219_H_

#include "board.h"

/**
 * @defgroup MAX7219 MAX7219 driver
 * @{
 */

#define NO_OP_REG           0x00    		/**< The No-Op register used for sending dummy data */
#define DECODE_MODE_REG     0x09 			/**< Decode Mode register used for turning on/off Code B decoding */
#define INTENSITY_REG       0x0A 			/**< Intensity register used for setting LEDs' brightness */
#define SCAN_LIMIT_REG      0x0B 			/**< Scan Limit register used for setting maximum number of scanned digits */
#define SHOUTDOWN_REG       0x0C 			/**< Shutdown register used for turning on/off the display */
#define DISPLAY_TEST_REG    0x0F 			/**< Display Test register used for turning on/off test mode (all LEDs) */
#define DIGIT_REG(DIGIT)    ((DIGIT) + 1)	/**< Calculate proper digit register from 0 offset.
										     The Digit registers starts from 0x01 (1st digit) to 0x08 (8th digit) */

/**
 *  Macro for building MAX7219 frames. The frame format is:
 *  | DON'T CARE BITS (B12:B15) | ADDR (B8:B11) | DATA (B0:B7) |
 */
#define MAX7219_FRAME(ADDR, DATA)	((uint16_t) (((ADDR) << 8) | (DATA)))

/**
 * @brief MAX7219 configuration structure
 * @note
 * SSP have to be configured as SPI master with 16 bit frame and CPOL, CPHA bits set to zero.
 * The clock rate should not exceed 10MHz
 * @note SSEL pin have to be configured as a GPIO output pin. See GPIO byte pin registers for passing base address
 */
typedef struct {
    LPC_SSP_T *ssp; 		/**< The base of SSP used */
    __IO uint8_t *ssel; 	/**< The base of GPIO pin used */
    size_t numOfDisplays;	/**< The number of chained MAX7219s used (number of displays) */
} MAX7219Config;

/**
 * @}
 */

#endif /* MAX7219_H_ */
