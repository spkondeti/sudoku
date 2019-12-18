# Sudoku Game on STM32 (Embedded C)
This is a Sudoku game created on STM32F051R8T6 microcontroller using LED matrix for output, keypad and lcd display for input 
### Tech
This project requires the following technologies to work properly:
* System Workbench for STM32 Software
* STM32F051R8T6 board
* CFAL1602 OLED LCD Display
* 4x4 0-9, a-d keypad (4 pins for row, 4 pins for columns)
* Adafruit 64x64 LED matrix (https://www.adafruit.com/product/3649)
* Speaker (If you want audio for the game)
* FTDI232 USB to Serial Device (If you want debugging during development)
### Wiring for LED matrix
 * PA0,OE 
 * PC0,R0 
 * PC1,G0 
 * PC2,B0 
 * PC3,R1 
 * PC4,G1
 * PC5,B1 
 * PC6,A0
 * PC7,A1
 * PC8,A2 
 * PC9,A3 
 * PC10,A4 
 * PC11,CLK 
 * PC12,LATCH 
 * (optional) Audio Output: PB14
 ![LED MATRIX WIRING](https://github.com/spkondeti/sudoku/blob/master/images/keypad.PNG)
 ![KEYPAD AND LCD DISPLAY WIRING](https://github.com/spkondeti/sudoku/blob/master/images/wiring.PNG)

### Gameplay
* To start giving input, Press '*' on the keypad (Input won't be taken, otherwise)
* To enter a number, mode is 'C'. To delete a number, mode is 'D'
* Mode, Num and Row, Col: Both of these should always be seperated by a B ('|') on the keypad
* Press '#' anytime during the game to reset the input 
### Instructions
* Power the STM32 and LED Matrix individually
* To change the colors, Change the RGB values in the update_board function
* If you want to change the number of blocks to remove depending on the level user selected, change the blocks_to_remove variable in game.c file

### Development
Want to contribute? Great!
Send commit requests or email me directly at skondeti@purdue.edu or suryapkondeti@gmail.com
