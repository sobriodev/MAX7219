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

#define MAX7219_NO_OP_REG           0x00            /**< The No-Op register used for sending dummy data */
#define MAX7219_DECODE_MODE_REG     0x09            /**< Decode Mode register used for turning on/off Code B decoding */
#define MAX7219_INTENSITY_REG       0x0A            /**< Intensity register used for setting LEDs' brightness */
#define MAX7219_SCAN_LIMIT_REG      0x0B            /**< Scan Limit register used for setting maximum number of scanned digits */
#define MAX7219_SHOUTDOWN_REG       0x0C            /**< Shutdown register used for turning on/off the display */
#define MAX7219_DISPLAY_TEST_REG    0x0F            /**< Display Test register used for turning on/off test mode (all LEDs) */
#define MAX7219_DIGIT_REG(DIGIT)    ((DIGIT) + 1)   /**< Calculate proper digit register from 0 offset.
                                                         The Digit registers starts from 0x01 (1st digit) to 0x08 (8th digit) */
/**
 *  Macro for building MAX7219 frames. The frame format is:
 *  |DON'T CARE BITS (B12:B15)|ADDR (B8:B11)|DATA (B0:B7)|
 */
#define MAX7219_FRAME(ADDR, DATA)    ((uint16_t) (((ADDR) << 8) | ((DATA) & 0xFF)))

#define LOW     0   /**<  Low state of a pin indicates presence of data */
#define HIGH    1   /**<  High state of a pin latches data in all MAX7219 chips */

/**
 * @brief MAX7219 configuration structure
 * @note
 * SSP have to be configured as SPI master with 16 bit frame and clock mode 0
 * The clock rate should not exceed 10MHz
 * @note SSEL pin have to be configured as a GPIO output pin. See GPIO byte pin registers for passing base address
 */
typedef struct {
    LPC_SSP_T *ssp;         /**< The base of SSP used */
    __IO uint8_t *ssel;     /**< The base of GPIO pin used */
    size_t numOfDisplays;   /**< The number of chained MAX7219s (number of displays) */
} MAX7219_Config;

/**
 * @brief Helper function for calculating intensity frame
 * @param intensity : Desired intensity
 * @return Calculated frame
 * @note Only lower nibble of the value is recognized. Values from 0x0 (minimum intensity) to 0xF (maximum intensity) are acceptable
 */
static inline uint16_t MAX7219_IntensityFrame(uint8_t intensity) {
    return MAX7219_FRAME(MAX7219_INTENSITY_REG, intensity);
}

/**
 * @brief Helper function for calculating shutdown frame
 * @param mode : Shutdown mode. Passing true turns off the display, false turns it on
 * @return Calculated frame
 */
static inline uint16_t MAX7219_ShutdownFrame(bool mode) {
    return MAX7219_FRAME(MAX7219_SHOUTDOWN_REG, !mode);
}

/**
 * @brief Helper function for calculating test mode frame
 * @param mode : Test mode. Passing true turns on test mode, false turns it off
 * @return Calculated frame
 */
static inline uint16_t MAX7219_TestFrame(bool mode) {
    return MAX7219_FRAME(MAX7219_DISPLAY_TEST_REG, mode);
}

/**
 * @brief Set all necessary settings and configure MAX7219s' blocks
 * @param config : An MAX7219_Config entity
 * @return Nothing
 */
void MAX7219_SetConfiguration(MAX7219_Config config);

/**
 * @brief Update the registry value of the desired display
 * @param offset : Display offset starting from 0
 * @param frame  : Requested frame
 * @return Nothing
 */
void MAX7219_UpdateDisplayReg(size_t offset, uint16_t frame);

/**
 * @brief Update the registry value of all displays
 * @param frame : Requested frame
 * @return Nothing
 */
void MAX7219_UpdateDisplaysReg(uint16_t frame);

/**
 * @}
 */

#endif /* MAX7219_H_ */
