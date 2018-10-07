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

#define MAX7219_NO_OP_REG           0x00    //!< No-Op register used for sending dummy data
#define MAX7219_DECODE_MODE_REG     0x09    //!< Decode Mode register used for turning on/off Code B decoding
#define MAX7219_INTENSITY_REG       0x0A    //!< Intensity register used for setting LEDs' brightness
#define MAX7219_SCAN_LIMIT_REG      0x0B    //!< Scan Limit register used for setting maximum number of scanned digits
#define MAX7219_SHOUTDOWN_REG       0x0C    //!< Shutdown register used for turning on/off the display
#define MAX7219_DISPLAY_TEST_REG    0x0F    //!< Display Test register used for turning on/off test mode (all LEDs)

/**< Calculate proper digit register from 0 offset. The Digit registers start from 0x01 (1st digit) and end at 0x08 (8th digit) */
#define MAX7219_DIG_REG(DIG)    ((DIG) + 1)

#define COLS_PER_DISP 8 //!< The number of columns per display

#define LOW     0   //!<  Low state of a pin indicates presence of data
#define HIGH    1   //!<  High state of a pin latches data in all MAX7219 chips

#define MAX_DISP_OFFSET(DISP)       ((DISP) - 1)            //!< Calculate maximum display offset
#define MAX_COL_OFFSET(COL)         ((COL) * 8 - 1)         //!< Calculate maximum column offset
#define END_COL_OFFSET(COL, LEN)    ((COL) + (LEN) - 1)     //!< Calculate end column offset based on current column and length of the data
#define COL_OFFSET(DISP, COL)       ((DISP) * 8 + (COL))    //!< Calculate absolute column offset based on a specified display column

/**
 * @brief MAX7219 configuration structure
 * @note
 * SSP have to be configured as SPI master with 16 bit frame and clock mode 0
 * The clock rate should not exceed 10MHz
 * @note SSEL pin have to be configured as a GPIO output pin. See GPIO byte pin registers for passing base address
 */
typedef struct {
    LPC_SSP_T *ssp;     //!< The base of SSP used
    __IO uint8_t *ssel; //!< The base of GPIO pin used
    uint8_t numOfDisp;  //!< The number of chained MAX7219s (number of displays)
} MAX7219_Config;

/**
 * @brief Return codes for library functions
 */
typedef enum {
    OP_SUCCESS = 0,     //!< Operation performed successfully
    BUFFER_ERROR = 1,   //!< Buffer allocation error
    OFFSET_ERROR = 2    //!< Given display/data offset exceeds correct range
} MAX7219_ReturnCodes;

/**
 * @brief Modes for updating display buffer
 */
typedef enum {
    VAL_UPDATE = 0, //!< Update multiple buffer bytes using single byte
    SEQ_UPDATE = 1  //!< Update the buffer with a sequence of bytes
} MAX7219_BufferUpdateModes;

/**
 * Build MAX7219 frame
 * @param addr : Register address
 * @param data : Specified data
 * @return Built frame
 * @note
 * The MAX7219 frame format is:
 * [ DON'T CARE BITS (B12:B15) | ADDR (B8:B11) | DATA (B0:B7) ]
 */
static inline uint16_t MAX7219_Frame(uint8_t addr, uint8_t data) {
    return ((uint16_t) (((addr) << 8) | ((data) & 0xFF)));
}

/**
 * @brief Helper function for calculating intensity frame
 * @param intensity : Desired intensity
 * @return Calculated frame
 * @note Only lower nibble of the value is recognized. Values from 0x0 (minimum intensity) to 0xF (maximum intensity) are acceptable
 */
static inline uint16_t MAX7219_Intensity(uint8_t intensity) {
    return MAX7219_Frame(MAX7219_INTENSITY_REG, intensity);
}

/**
 * @brief Helper function for calculating shutdown frame
 * @param mode : Shutdown mode. Passing true turns off the display, false turns it on
 * @return Calculated frame
 */
static inline uint16_t MAX7219_Shutdown(bool mode) {
    return MAX7219_Frame(MAX7219_SHOUTDOWN_REG, !mode);
}

/**
 * @brief Helper function for calculating test mode frame
 * @param mode : Test mode. Passing true turns on test mode, false turns it off
 * @return Calculated frame
 */
static inline uint16_t MAX7219_Test(bool mode) {
    return MAX7219_Frame(MAX7219_DISPLAY_TEST_REG, mode);
}

/**
 * @brief Set all necessary settings, configure MAX7219 blocks and allocate buffer data
 * @param config : An MAX7219_Config entity.
 * @return The function can return:
 *           - BUFFER_ERROR when data buffer cannot be allocated
 *           - OP_SUCCESS when everything is set correctly
 * @note Only positive values for MAX7219_Config.numOfDisp are allowed. Passing 0 will cause change the variable value to 1
 * @note Always check the return code. Behavior of library functions is undefined if this function does not return OP_SUCCESS code
 */
MAX7219_ReturnCodes MAX7219_SetConf(MAX7219_Config config);

/**
 * @brief Update the registry value of the desired display
 * @param dispOffset : Display offset starting from 0
 * @param frame      : Requested frame
 * @return The function can return:
 *           - OFFSET_ERROR when given offset exceeds total numbers of displays
 *           - OP_SUCCESS otherwise
 */
MAX7219_ReturnCodes MAX7219_UpdateDisplayReg(uint8_t dispOffset, uint16_t frame);

/**
 * @brief Update the registry value of all displays
 * @param frame : Requested frame
 * @return Nothing
 */
void MAX7219_UpdateDisplaysReg(uint16_t frame);

/**
 * @brief Write bytes to the output buffer
 * @param colOffset : Column offset starting from 0
 * @param data      : Data base address
 * @param bytes     : The Number of bytes written to the buffer
 * @param opCode    : Operation code. Supported values:
 *                     - VAL_UPDATE : Update N bytes of the buffer with the same value. Only first byte of the data is recognized
 *                     - SEQ_UPDATE : Fill the buffer with N bytes of data
 * @return Multiple return values are supported:
 *          - OFFSET_ERROR when data will not fit into the buffer
 *          - OP_SUCCESS otherwise
 * @see MAX7219_UpdateBufferVal() and MAX7219_BufferSeq() functions for more convenient updating of the buffer
 */
MAX7219_ReturnCodes MAX7219_UpdateBuffer(uint16_t colOffset, const uint8_t *data, uint16_t bytes, MAX7219_BufferUpdateModes mode);

/**
 * @brief Write data to the output buffer
 * @param colOffset : Column offset starting from 0
 * @param value     : Data base address
 * @param bytes     : The number of bytes written to the buffer
 * @return Multiple return values are supported:
 *          - OFFSET_ERROR when data will not fit into the buffer
 *          - OP_SUCCESS otherwise
 * @note This is wrapper for the function MAX7219_UpdateBuffer()
 */
static inline MAX7219_ReturnCodes MAX7219_UpdateBufferSeq(uint16_t colOffset, const uint8_t *data, uint16_t bytes) {
    return MAX7219_UpdateBuffer(colOffset, data, bytes, SEQ_UPDATE);
}

/**
 * @brief Update output buffer columns with the same value
 * @param colOffset : Column offset starting from 0
 * @param data      : Data base address
 * @param bytes     : The number of bytes written to the buffer
 * @return Multiple return values are supported:
 *          - OFFSET_ERROR when data will not fit into the buffer
 *          - OP_SUCCESS otherwise
 * @note This is wrapper for the function MAX7219_UpdateBuffer()
 */
static inline MAX7219_ReturnCodes MAX7219_UpdateBufferVal(uint16_t colOffset, uint8_t value, uint16_t bytes) {
    return MAX7219_UpdateBuffer(colOffset, &value, bytes, VAL_UPDATE);
}

/**
 * @brief Send data from the buffer to the displays
 * @todo Modify the function to refresh only specified columns
 * @return Nothing
 */
void MAX7219_RefreshDisp(void);

/**
 * @brief Unset configuration and free allocated memory
 * @return Nothing
 */
void MAX7219_UnsetConf(void);

/**
 * @}
 */

#endif /* MAX7219_H_ */
