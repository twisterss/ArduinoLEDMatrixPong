#include "Arduino.h"
#include "led_matrix.h"

LEDMatrix::LEDMatrix(byte data_out, byte clk_out, byte wrclk_out, byte cs_out):
  buffer(),
  data(data_out),
  clk(clk_out),
  wrclk(wrclk_out),
  cs(cs_out)
{
  int i,j;

  // Initialize the buffer
  for(i=0; i<CELLS; i++)
    for(j=0; j<CHIPS; j++)
      buffer[i][j] = 1;
  
  // Initialize the outputs
  pinMode(cs, OUTPUT);
  pinMode(wrclk, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(clk, OUTPUT);
  
  // Send initial commands
  for (i=1; i<=CHIPS; i++) {
    sendCmd(i, HT1632_CMD_SYSDIS); // Disable system
    sendCmd(i, HT1632_CMD_COMS00); // 16*32, PMOS drivers
    sendCmd(i, HT1632_CMD_MSTMD);  // MASTER MODE - Use on-chip clock
    sendCmd(i, HT1632_CMD_SYSON);  // System on - Enable system oscil
    sendCmd(i, HT1632_CMD_LEDON);  // LEDs on 
  }
  
  // Clear the display
  clear(); 
  commit();
}


byte LEDMatrix::ledColorActive(Color color, byte ledColor) {
  // We use color order in the enum to select which colors should be on
  byte byteColor = (byte) color;
  return byteColor & (1 << ledColor);
}

void LEDMatrix::chipSelect(int chip) {
  int tmp;
  if (chip<0) {
    digitalWrite(cs, LOW); 
    for (tmp=0; tmp<CHIPS; tmp++) {
      digitalWrite(clk, HIGH);
      digitalWrite(clk, LOW);
    }
  } else {
    digitalWrite(cs, HIGH);
    for(tmp=0; tmp<CHIPS; tmp++) {
      digitalWrite(clk, HIGH);
      digitalWrite(clk, LOW);
    }
    if (chip>0) {
      digitalWrite(cs, LOW);
      digitalWrite(clk, HIGH);
      digitalWrite(clk, LOW);
      digitalWrite(cs, HIGH);
      for (tmp=1 ; tmp<chip; tmp++) {
        digitalWrite(clk, HIGH);
        digitalWrite(clk, LOW);
      }
    }
  }
    
}

void LEDMatrix::clear() {
  int i, j;
  for(i=0; i<CELLS; i++)
    for(j=0; j<CHIPS; j++)
        writeToBuffer(i, j, 0);
}

void LEDMatrix::writeBits(byte bits, int length) {
  int i;
  for (i=length; i>=0; i--) {
    digitalWrite(wrclk, LOW);
    if (bits & 1<<i) {
      digitalWrite(data, HIGH);
    } else {
      digitalWrite(data, LOW);
    }
    digitalWrite(wrclk, HIGH);
  }
}

void LEDMatrix::sendCmd(byte chipNo, byte command) {
  chipSelect(chipNo);
  writeBits(HT1632_ID_CMD, 2);  // send 3 bits of id: COMMMAND
  writeBits(command, 7);        // send the actual command
  writeBits(0, 1); 	        /* one extra dont-care bit in commands. */
  chipSelect(0);
}

void LEDMatrix::plot(int x, int y, Color color) {
 // check the inputs value
 // in case of error return nothing
 if(x<0 || x>=WIDTH|| y<0 || y>HEIGHT)  
    return;
   
  // transform the X/Y coordinates to the number of the microchip
  // that controls the region (1,2,3 or 4) the LED you want to
  // light up
  int chip = x/(WIDTH/CHIPS_X) + CHIPS_X*(y/(HEIGHT/CHIPS_Y));
  // after the selection of the chip we need just the coordinate
  // for 1 of them, so we need to change the coordinate system
  int local_x = x % (WIDTH/CHIPS_X); // columns
  int local_y = y % (HEIGHT/CHIPS_Y);  // rows
  
  // Mask of the byte to change
  byte localMask = 1 << ((LEDS_PER_CELL - 1) - y % LEDS_PER_CELL);

  // Write in the buffer for each color
  int address = local_x * HEIGHT / (CHIPS_Y * LEDS_PER_CELL) + local_y / LEDS_PER_CELL;
  byte localBuffer;
  for (int ledColor = 0; ledColor < COLORS; ledColor++) {
    localBuffer = buffer[address][chip];
    if (ledColorActive(color, ledColor))
      localBuffer|= localMask;
    else
      localBuffer&= ~localMask;
    writeToBuffer(address, chip, localBuffer);
    address+= CELLS / COLORS;
  }
}

void LEDMatrix::writeToBuffer(int address, int chip, byte localBuffer) {
  // A local buffer is only 4 bits
  localBuffer&= CELL_MASK;
  // Set byte 7 to 1 to indicate a change
  if ((buffer[address][chip] & CELL_MASK) != localBuffer)
    buffer[address][chip] = localBuffer | CELL_MODIFIED_MASK;
}

void LEDMatrix::commit() {
  int address;
  byte localBuffer;
  int chip, cell, color;
  for (chip=0; chip<CHIPS; chip++) {
    for (cell=0; cell<CELLS/2; cell++) {
      for (color = 0; color < 2; color++) {
        address = cell + 32 * color;
        localBuffer = buffer[address][chip];
        // Has the buffer been modified?
        if (localBuffer & CELL_MODIFIED_MASK) {
          // Select the chip
          chipSelect(chip+1);
          // Write the buffer to the screen
          writeBits(HT1632_ID_WR, 2);  		   // send ID: WRITE to RAM
          writeBits(address, 6);       		   // Send address
          writeBits(localBuffer, LEDS_PER_CELL-1);   // send 4 bits of data
          // Removes the modified bit from the buffer
          buffer[address][chip]&= CELL_MASK;
        }
      }
    }
  }
  //Serial.println(count);
}
