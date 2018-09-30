/*
 * max7219.c
 *
 *  Created on: 24 Sep 2018
 *      Author: sobriodev
 */

#include "max7219.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static MAX7219_Config conf;

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

void MAX7219_SetConfiguration(MAX7219_Config config) {
    conf = config;
    if (conf.numOfDisplays == 0) { /* Change the number of displays to one if zero is passed */
        conf.numOfDisplays = 1;
    }
    *(conf.ssel) = HIGH; /* The SSEL pin have to be high when inactive */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_DECODE_MODE_REG, 0x00)); /* Disable Code B */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_SCAN_LIMIT_REG, 0x07)); /* Set scan limit to 8 digits */
    MAX7219_UpdateDisplaysReg(MAX7219_FRAME(MAX7219_DISPLAY_TEST_REG, 0x00)); /* Disable test mode */
}

void MAX7219_UpdateDisplayReg(size_t offset, uint16_t frame) {
    if (offset > conf.numOfDisplays - 1) { /* If the display offset is greater than the total amount of displays do nothing */
        return;
    }
    BSYWait(); /* Make sure that SSP controller is idle and drive pin state low */
    *(conf.ssel) = LOW;
    for (size_t i = 1; i <= conf.numOfDisplays; i++) { /* Send frames in reverse order. The first frame will arrive to the last MAX7219 chip */
        if (offset == (conf.numOfDisplays - i)) {
            sendSPIFrame(frame); /* Send desired data */
        }
        else {
            sendSPIFrame(MAX7219_FRAME(MAX7219_NO_OP_REG, 0x00)); /* Send dummy data */
        }
    }
    BSYWait(); /* Wait until the SSP controller is idle to prevent latching data when TX FIFO is not empty and drive pin state high */
    *(conf.ssel) = HIGH;
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

