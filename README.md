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
 ![LED MATRIX WIRING](https://github.com/spkondeti/sudoku/blob/master/images/keypad.PNG)
 ![KEYPAD AND LCD DISPLAY WIRING](https://github.com/spkondeti/sudoku/tree/master/images/wiring.png)

### Running the application
Run the following command in the terminal:
```sh
python manage.py runserver
```

Verify the deployment by navigating to: (your domain name in deployment mode)
```sh
127.0.0.1:8000
```

### Development
Want to contribute? Great!
Send commit request or email me directly at skondeti@purdue.edu or suryapkondeti@gmail.com

**Free Software, Hell Yeah!**
