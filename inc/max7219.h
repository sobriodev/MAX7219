/**
 * max7219.h
 *
 *  Created on: 24 Sep 2018
 *      Author: sobriodev
 */

#ifndef MAX7219_H_
#define MAX7219_H_

/* Register address map */
#define NO_OP_REG           0x00            /* The No-Op register used for sending dummy data */
#define DECODE_MODE_REG     0x09            /* The Decode Mode register used for turning on/off Code B decoding */
#define INTENSITY_REG       0x0A            /* The Intensity register used for setting LEDs' brightness */
#define SCAN_LIMIT_REG      0x0B            /* The Scan Limit register used for setting maximum number of scanned digits */
#define SHOUTDOWN_REG       0x0C            /* The Shutdown register used for turning on/off the display */
#define DISPLAY_TEST_REG    0x0F            /* The Display Test register used for turning on/off test mode (turn on all display's LEDs) */
#define DIGIT_REG(DIGIT)    ((DIGIT) + 1)   /* Calculate proper digit register from 0 offset. The Digit registers starts from 0x01 (1st digit) to 0x08 (8th digit) */

/* Build MAX7219 SPI frame. The frame format is:
 * ---------------------------------------------------------------------------------------
 * | B15 | B14 | B13 | B12 | B11 | B10 | B9 | B8 | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
 * ---------------------------------------------------------------------------------------
 * |  X  |  X  |  X  |  X  |       ADDRESS       |                  DATA                 |
 * ---------------------------------------------------------------------------------------
 * B12:B15 are meaningless bits
 */
#define MAX7219_FRAME(ADDR, DATA)   ((uint16_t) (((ADDR) << 8) | (DATA)))

#endif /* MAX7219_H_ */
