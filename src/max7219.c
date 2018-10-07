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
static uint8_t *buffer = NULL;  /**< Buffer for storing output data */

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
static inline void SSP_TNFWait(void) {
    while (!(conf.ssp->SR & SSP_STAT_TNF));
}

/**
 * @brief Wait until the BSY bit is 0 indicating that the SSP controller is idle
 * @return Nothing
 */
static inline void SSP_BSYWait(void) {
    while (conf.ssp->SR & SSP_STAT_BSY);
}

/**
 * @brief Send 16-bit SPI frame
 * @param frame : Requested frame
 */
static void SSP_SendFrame(uint16_t frame) {
    SSP_TNFWait(); /* Make sure that TX FIFO will handle the frame */
    Chip_SSP_SendFrame(conf.ssp, frame);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

MAX7219_ReturnCodes MAX7219_SetConf(MAX7219_Config config) {
    conf = config;
    if (conf.numOfDisp == 0) {
        conf.numOfDisp = 1; /* Change the number of displays to one if zero is passed */
    }

    buffer = (uint8_t *) malloc(sizeof(uint8_t) * conf.numOfDisp);
    if (buffer == NULL) {
        return BUFFER_ERROR; /* Memory cannot be allocated. Return error code */
    }

    *(conf.ssel) = HIGH; /* The SSEL pin have to be high when inactive */
    MAX7219_UpdateDisplaysReg(MAX7219_Frame(MAX7219_DECODE_MODE_REG, 0x00)); /* Disable Code B */
    MAX7219_UpdateDisplaysReg(MAX7219_Frame(MAX7219_SCAN_LIMIT_REG, 0x07)); /* Set scan limit to 8 digits */
    MAX7219_UpdateDisplaysReg(MAX7219_Frame(MAX7219_DISPLAY_TEST_REG, 0x00)); /* Disable test mode */

    return OP_SUCCESS;
}

MAX7219_ReturnCodes MAX7219_UpdateDisplayReg(uint8_t dispOffset, uint16_t frame) {
    if (dispOffset > MAX_DISP_OFFSET(conf.numOfDisp)) { /* If the display offset is greater than the total amount of displays do nothing */
        return OFFSET_ERROR;
    }

    SSP_BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
    *(conf.ssel) = LOW;
    for (uint8_t i = 1; i <= conf.numOfDisp; i++) { /* Send frames in reverse order. The first frame will arrive to the last MAX7219 chip */
        if (dispOffset == (conf.numOfDisp - i)) {
            SSP_SendFrame(frame); /* Send desired data */
        }
        else {
            SSP_SendFrame(MAX7219_Frame(MAX7219_NO_OP_REG, 0x00)); /* Send dummy data */
        }
    }
    SSP_BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
    *(conf.ssel) = HIGH;

    return OP_SUCCESS;
}

void MAX7219_UpdateDisplaysReg(uint16_t frame) {
    SSP_BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
    *(conf.ssel) = LOW;
    for (uint8_t i = 0; i < conf.numOfDisp; i++) {
        SSP_SendFrame(frame);
    }
    SSP_BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
    *(conf.ssel) = HIGH;
}

MAX7219_ReturnCodes MAX7219_UpdateBuffer(uint16_t colOffset, const uint8_t *data, uint16_t bytes, MAX7219_BufferUpdateModes mode) {
    /* Basic validation. Cast the offset to a 32 bit value to prevent overflow while calculating end column offset */
    if (bytes == 0 || END_COL_OFFSET((uint32_t) colOffset, bytes) > MAX_COL_OFFSET(conf.numOfDisp)) {
        return OFFSET_ERROR;
    }

    if (mode == VAL_UPDATE) { /* If VAL_UPDATE is passed just copy the value N times to the buffer */
        for (uint16_t i = colOffset; i <= END_COL_OFFSET(colOffset, bytes); i++) {
            buffer[i] = *data;
        }
    } else if (mode == SEQ_UPDATE) { /* Otherwise copy all the data to the buffer */
        for (uint16_t i = colOffset, j = 0; i <= END_COL_OFFSET(colOffset, bytes); i++, j++) {
            buffer[i] = data[j];
        }
    }

    return OP_SUCCESS;
}

void MAX7219_RefreshDisp(void) {
    for (int8_t i = 0; i < COLS_PER_DISP; i++) {
        SSP_BSYWait(); /* Make sure that the SSP controller is idle and drive pin state low */
        *(conf.ssel) = LOW;

        for (uint8_t j = 1; j <= conf.numOfDisp; j++) {
            SSP_SendFrame(MAX7219_Frame(MAX7219_DIG_REG(i), buffer[COL_OFFSET(conf.numOfDisp - j, i)]));
        }

        SSP_BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
        *(conf.ssel) = HIGH;
    }
}

void MAX7219_UnsetConf(void) {
    free(buffer);
    buffer = NULL;
}

