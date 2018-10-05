/*
 * max7219.c
 *
 *  Created on: 24 Sep 2018
 *      Author: sobriodev
 */

#include "max7219.h"
#include "stdlib.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static MAX7219_Config conf;     /**< Stores all necessary configuration parameters */
static uint8_t *buffer = NULL;  /**< Buffer for storing displays data */


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/**
 * @brief Wait until the TNF bit is 1 indicating that the TX FIFO is not full
 * @return Nothing
 */
static inline void TNFWait(void) {
    while (!(conf.ssp->SR & SSP_STAT_TNF));
}

/**
 * @brief Wait until the BSY bit is 0 indicating that the SSP controller is idle
 * @return Nothing
 */
static inline void BSYWait(void) {
    while (conf.ssp->SR & SSP_STAT_BSY);
}

/**
 * @brief Send 16-bit SPI frame
 * @param frame : Requested frame
 */
static void sendSPIFrame(uint16_t frame) {
    TNFWait(); /* Make sure that TX FIFO will handle the frame */
    conf.ssp->DR = frame;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

MAX7219_ReturnCodes MAX7219_SetConfiguration(MAX7219_Config config) {
    conf = config;
    if (conf.numOfDisplays == 0) {
        conf.numOfDisplays = 1; /* Change the number of displays to one if zero is passed */
    }

    buffer = (uint8_t *) malloc(sizeof(uint8_t) * conf.numOfDisplays);
    if (buffer == NULL) {
        return BUFFER_ERROR; /* Memory cannot be allocated. Return error code */
    }

    *(conf.ssel) = HIGH; /* The SSEL pin have to be high when inactive */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_DECODE_MODE_REG, 0x00)); /* Disable Code B */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_SCAN_LIMIT_REG, 0x07)); /* Set scan limit to 8 digits */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_DISPLAY_TEST_REG, 0x00)); /* Disable test mode */

    return OP_SUCCESS;
}

MAX7219_ReturnCodes MAX7219_UpdateDisplayReg(size_t dispOffset, uint16_t frame) {
    if (dispOffset > MAX_DISP_OFFSET(conf.numOfDisplays)) { /* If the display offset is greater than the total amount of displays do nothing */
        return OFFSET_ERROR;
    }

    BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
    *(conf.ssel) = LOW;
    for (size_t i = 1; i <= conf.numOfDisplays; i++) { /* Send frames in reverse order. The first frame will arrive to the last MAX7219 chip */
        if (dispOffset == (conf.numOfDisplays - i)) {
            sendSPIFrame(frame); /* Send desired data */
        }
        else {
            sendSPIFrame(MAX7219_FRAME(MAX7219_NO_OP_REG, 0x00)); /* Send dummy data */
        }
    }
    BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
    *(conf.ssel) = HIGH;

    return OP_SUCCESS;
}

void MAX7219_UpdateDisplaysReg(uint16_t frame) {
    BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
    *(conf.ssel) = LOW;
    for (size_t i = 0; i < conf.numOfDisplays; i++) {
        sendSPIFrame(frame);
    }
    BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
    *(conf.ssel) = HIGH;
}

MAX7219_ReturnCodes MAX7219_UpdateBuffer(size_t colOffset, const uint8_t *data, size_t bytes, bool singleValue) {
    /* Parameters validation */
    if (colOffset > MAX_COL_OFFSET(conf.numOfDisplays) ||
        bytes == 0 ||
        NEW_COL_OFFSET(colOffset, bytes) > MAX_COL_OFFSET(conf.numOfDisplays))
    {
        return OFFSET_ERROR;
    }

    if (singleValue) { /* If single value is passed just copy it N times to the buffer */
        for (size_t i = colOffset; i <= NEW_COL_OFFSET(colOffset, bytes); i++) {
            buffer[i] = *data;
        }
    } else { /* Otherwise copy all the data to the buffer */
        for (size_t i = colOffset, j = 0; i <= NEW_COL_OFFSET(colOffset, bytes); i++, j++) {
            buffer[i] = data[j];
        }
    }

    return OP_SUCCESS;
}

void MAX7219_RefreshDisp(void) {
    for (size_t i = 0; i < 8; i++) {
        BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
        *(conf.ssel) = LOW;

        for (size_t j = 1; j <= conf.numOfDisplays; j++) {
            sendSPIFrame(MAX7219_FRAME(MAX7219_DIGIT_REG(i), buffer[(conf.numOfDisplays - j) * 8 + i]));
        }

        BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
        *(conf.ssel) = HIGH;
    }
}

void MAX7219_UnsetConfiguration(void) {
    free(buffer);
    buffer = NULL;
}

