/**
 * 32x16 Sure Electronics bi-color LED matrix library.
 * Written in september 2012 by Tristan Groleat.
 * Based on the code written by dataTeam in August 2011 for pong.
 * The library is very simple to use:
 * * initialize the screen by creating a new LEDMatrix object (parameters are the numbers of the pins you connected the screen to)
 * * use member functions plot and clear to change the local buffer as you want
 * * use member function render to send the changes to the screen
 */

#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "Arduino.h"

#define HT1632_ID_CMD 4		/* ID = 100 - Commands */
#define HT1632_ID_RD  6		/* ID = 110 - Read RAM */
#define HT1632_ID_WR  5		/* ID = 101 - Write RAM */

#define HT1632_CMD_SYSDIS 0x00	/* CMD= 0000-0000-x Turn off oscil */
#define HT1632_CMD_SYSON  0x01	/* CMD= 0000-0001-x Enable system oscil */
#define HT1632_CMD_LEDOFF 0x02	/* CMD= 0000-0010-x LED duty cycle gen off */
#define HT1632_CMD_LEDON  0x03	/* CMD= 0000-0011-x LEDs ON */
#define HT1632_CMD_BLOFF  0x08	/* CMD= 0000-1000-x Blink ON */
#define HT1632_CMD_BLON   0x09	/* CMD= 0000-1001-x Blink Off */
#define HT1632_CMD_SLVMD  0x10	/* CMD= 0001-0xxx-x Slave Mode */
#define HT1632_CMD_MSTMD  0x18	/* CMD= 0001-10xx-x Use on-chip clock */
#define HT1632_CMD_EXTCLK 0x1C	/* CMD= 0001-11xx-x Use external clock */
#define HT1632_CMD_COMS00 0x20	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS01 0x24	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS10 0x28	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_COMS11 0x2C	/* CMD= 0010-ABxx-x commons options */
#define HT1632_CMD_PWM    0xA0	/* CMD= 101x-PPPP-x PWM duty cycle */

#define WIDTH 32 // Width of the matrix (in LEDs)
#define HEIGHT 16// Height of the matrix (in LEDs)
#define CHIPS_X 2  // Number of chips horizontally
#define CHIPS_Y 2 // Number of chips vertically
#define CHIPS (CHIPS_X*CHIPS_Y) // Number of HT1632C Chips
#define COLORS 2 // Number of colors supported
#define LEDS_PER_CELL 4 // Number of LEDs adressed at the same time
#define CELLS (WIDTH * HEIGHT * COLORS / (LEDS_PER_CELL * CHIPS))  // Number of adressable cells on one chip
#define CELL_MASK ((1<<LEDS_PER_CELL) - 1) // Mask to get the LED data in one cell
#define CELL_MODIFIED_MASK (1<<7) // Mask to get the modified bit in one cell

/**
 * Colors
 * The order of the definition is important:
 * the index of the color (0 for the first, etc) is used as bit mask
 * to determine which colors should be on
 */
typedef enum Color {
  BLACK,      // both LEDs off
  GREEN,      // first LED on
  RED,        // second LED on
  ORANGE      // both LEDs on
} Color;


class LEDMatrix {

protected:

	/**
	 * Buffer which contains the screen data
	 * before committing
	 */
	byte buffer[CELLS][CHIPS];

	/**
	 * Numbers of the Arduino output pins
	 */
	byte data, clk, wrclk, cs;

public:

	/** 
	 * Initialize the matrix
	 */
	LEDMatrix(byte data_out, byte clk_out, byte wrclk_out, byte cs_out);

	/**
	 * Plot a pixel on the local buffer.
         * You will need to commit to actually display the change.
	 */
	void plot(int x, int y, Color color);

	/**
	 * Set the local bufer to all black.
         * You will need to commit to actually display the change.
	 */
	void clear();

	/**
	 * Render the current local buffer to the screen.
         * Only parts of the buffer that have been changed are sent to the screen
         */
	void render();

protected:

        /**
         * Check if the LED of color ledColor should
         * be active for this color
         */
        byte ledColorActive(Color color, byte ledColor);

	/**
	 * Select a chip (from 1 to 4)
	 * on the matrix
	 */
	void chipSelect(int chip);

	/**
	 * Send bits to the matrix
	 * sequentially
	 */
	void writeBits(byte bits, int length);

	/**
	 * Send a command to a chip on the matrix
	 */
	void sendCmd(byte chipNo, byte command);

	/**
	 * Write a buffer locally.
	 * And remember the changes to commit.
	 */
	void writeToBuffer(int address, int chip, byte localBuffer);
};

#endif
