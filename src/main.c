#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

int is_valid(int row, int col, int num);
void (*cmd)(char b) = 0;
void (*data)(char b) = 0;
void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;
//void init_lcd(void);
int display[9][9];
int hash_pressed;
int given_display[9][9];
char line1[16] = { "Mode|Num:" };
char line2[16] = { "Row|Col:" };
char line3[16] = { "Choose Level:" };
char line4[16] = { "GAME  FINISHED" };
char line_clear[16] = { "  PRESS RESET  " };
int col = 0;
long long timer_random = 0;
int level;
int delay_time = 10000;
int8_t history[16] = { 0 };
int8_t lookup[16] =
        { 1, 4, 7, 0xe, 2, 5, 8, 0, 3, 6, 9, 0xf, 0xa, 0xb, 0xc, 0xd };
char char_lookup[16] = { '1', '4', '7', '*', '2', '5', '8', '0', '3', '6', '9',
        '#', 'A', 'B', 'C', 'D' };
char input1[3] = { ' ', ' ', ' ' };
char input2[3] = { ' ', ' ', ' ' };
int row_counter;
int col_counter;
int pos = 0;
char key_being_pressed = '\0';
uint8_t key_press_ack = 0;
uint8_t pixels[64][64];
enum phase {
    WAIT_FOR_DIFFICULTY,
    WAIT_FOR_INPUT_TYPE,
    WAIT_FOR_MODE,
    GET_ROW_COL,
    ADJUSTING_BOARD,
    GAME_FINISHED,
    GAME_OFF
};

//No bigger than MAX_INT
#define WAVE_DEFINITION 25

#define SIXTEENTH 50000000
#define AFT_NOTE 30000000

#define OFF     10.0
#define C3      130.81
#define Db3     138.59
#define D3      146.83
#define Eb3     155.56
#define E3      164.81
#define F3      174.61
#define Gb3     185.00
#define G3      196.00
#define Ab3     207.65
#define A3      220.00
#define Bb3     233.08
#define B3      246.94
#define C4      261.63
#define Db4     277.18
#define D4      293.66
#define Eb4     311.13
#define E4      329.63
#define F4      349.23
#define Gb4     369.99
#define G4      392.00
#define Ab4     415.30
#define A4      440.00
#define Bb4     466.16
#define B4      493.88
#define C5      523.25
#define Db5     554.37
#define D5      587.33
#define Eb5     622.25
#define E5      659.25
#define F5      698.46
#define Gb5     739.99
#define G5      783.99
#define Ab5     830.61
#define A5      880.00
#define Bb5     932.33
#define B5      987.77
#define C6      1046.50

float tetris_notes[23] = { OFF, E5, B4, C5, D5, C5, B4, A4, A4, C5, E5, D5, C5, B4,
B4, B4, C5, D5, E5, C5, A4, A4, OFF };
float happy_notes[5] = { OFF, C5, C5, G5, OFF };
float sad_notes[5] = { OFF, G5, G5, C5, OFF };
float tri_notes[10] = { OFF, F5, B5, F5, B5, F5, B5, F5, B5, OFF};
int tetris[23] = {4, 8, 4, 4, 8, 4, 4, 8, 4, 4, 8, 4, 4, 8, 2, 2, 2, 8, 8, 8, 8,
        16, 4};
int happy[5] = {4, 2, 2, 8, 4 };
int sad[5] = {4, 2, 2, 8, 4};
int tri[10] = {4, 1, 1, 1, 1, 1, 1, 1, 1, 4};
int note_num = 0;
enum tone {
    NONE, TETRIS, HAPPY, SAD, TRI
};
enum tone song = NONE;
double held_for = 0;
double current_note;

//TEMPORARY
#define VOLUME 50

uint16_t wavetable[WAVE_DEFINITION] = { 0 };
int count;

void init_wavetable(void) {
    int x;
    for (x = 0; x < sizeof wavetable / sizeof wavetable[0]; x += 1)
        wavetable[x] = (VOLUME * sin(x * 2 * M_PI / WAVE_DEFINITION) + VOLUME);
}

char get_char_key() {
    int index = get_key_pressed();
    return char_lookup[index];
}

void setup_gpio() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER4_1;
    GPIOA->MODER &= ~GPIO_MODER_MODER4_0;
    GPIOA->AFR[0] &= ~(0xf << (4 * (4)));
    GPIOA->AFR[0] |= 0x4 << (4 * (4));

}

void setup_PWM_TIM14() {
    //Enable TIM14 Peripheral
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;

    //Prescaler and Auto Reload Register
    TIM14->PSC = 0;
    TIM14->ARR = 100;

    //Alternate Function Setting
    TIM14->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM14->CCMR1 &= ~TIM_CCMR1_OC1M_0;

    //Output compare preload enable
    TIM14->CCMR1 |= TIM_CCMR1_OC1PE;

    //Capture and Compare Output Enable
    TIM14->CCER |= TIM_CCER_CC1E;

    TIM14->CCR1 = 50;

    //Main Output Enable
    TIM14->BDTR |= TIM_BDTR_MOE; //MOE BDTR

    //Turn on timer 1
    //TIM14->CR1 |= TIM_CR1_CEN;

    TIM14->EGR |= TIM_EGR_UG;
}

void update_freq(int freq) {
    double psc;
    current_note = freq;
    psc = (48000000.0 / (WAVE_DEFINITION * freq * 2)) - 1;

    TIM1->PSC = psc;
}

void setup_TIM1() {
    //Enable TIM1Peripheral
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    //Prescaler and Auto Reload Register
    TIM1->PSC = 8000;
    TIM1->ARR = 1;
    //Turn on Interrupt
    TIM1->DIER |= TIM_DIER_UIE;
    //Turn on Interrupt
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    //Turn on timer 1
    //TIM1->CR1 |= TIM_CR1_CEN;

}

void TIM1_BRK_UP_TRG_COM_IRQHandler() {
    //Acknowledge interrupt
    TIM14->CCR1 = wavetable[count];
    TIM1->SR &= ~TIM_SR_UIF;
    if (song == NONE)
        return;
    count += 1;
    if (count >= WAVE_DEFINITION) {
        count = 0;
        held_for += (double)1 / (double)(48000000 / ((TIM1 -> PSC + 1) * (TIM1 -> ARR + 1)));
    }
    if (song == TETRIS) {
        if (held_for > ((float) tetris[note_num] * 0.002)) {
            note_num += 1;
            held_for = 0;
            if (note_num > 23)
                note_num = 0;
            update_freq(tetris_notes[note_num]);
        } else
            return;
    } else if (song == HAPPY) {
        if (held_for > ((float) happy[note_num] * 0.002)) {
            note_num += 1;
            held_for = 0;
            if (note_num > 5) {
                song = TETRIS;
                note_num = 0;
            }
            update_freq(happy_notes[note_num]);
        } else
            return;
    } else if (song == SAD) {
        if (held_for > ((float) sad[note_num] * 0.002)) {
            note_num += 1;
            held_for = 0;
            if (note_num > 5) {
                song = TETRIS;
                note_num = 0;
            }
            update_freq(sad_notes[note_num]);
        } else
            return;
    } else if (song == TRI) {
        if (held_for > ((float) tri[note_num] * 0.002)) {
            note_num += 1;
            held_for = 0;
            if (note_num > 10) {
                song = TETRIS;
                note_num = 0;
            }
            update_freq(tri_notes[note_num]);
        } else
            return;
    }


}

void on_PWM() {
    TIM14->CR1 |= TIM_CR1_CEN;
    TIM1->CR1 |= TIM_CR1_CEN;
}
void off_PWM() {
    TIM14->CR1 &= ~TIM_CR1_CEN;
    TIM1->CR1 &= ~TIM_CR1_CEN;
}
void toggle_PWM() {
    TIM14->CR1 = (TIM14->CR1 ^ TIM_CR1_CEN);
}

void rest() {
    TIM1->PSC = 62000;
}

void outTone(float freq, int sixteenths) {
    update_freq(freq);
    nano_wait(sixteenths * SIXTEENTH - AFT_NOTE);
    rest();
    nano_wait(AFT_NOTE);
}

void fastOutTone(float freq, int sixteenths) {
    update_freq(freq);
    nano_wait(sixteenths * (SIXTEENTH / 2) - (AFT_NOTE / 4));
    rest();
    nano_wait(AFT_NOTE / 4);
}
/*
void tetris() {
    on_PWM();

    outTone(E5, 8);

    outTone(B4, 4);

    outTone(C5, 4);

    outTone(D5, 8);

    outTone(C5, 4);

    outTone(B4, 4);

    outTone(A4, 8);

    outTone(A4, 4);

    outTone(C5, 4);

    outTone(E5, 8);

    outTone(D5, 4);

    outTone(C5, 4);

    outTone(B4, 8);

    outTone(B4, 2);

    outTone(B4, 2);

    outTone(C5, 2);

    outTone(D5, 8);

    outTone(E5, 8);

    outTone(C5, 8);

    outTone(A4, 8);

    outTone(A4, 16);

    off_PWM();
}

void happy() {
    on_PWM();

    outTone(C5, 2);

    outTone(C5, 2);

    outTone(G5, 8);

    off_PWM();
}

void sad() {
    on_PWM();

    outTone(G5, 2);

    outTone(G5, 2);

    outTone(C5, 8);

    off_PWM();
}

void tri() {
    on_PWM();

    fastOutTone(F5, 1);

    fastOutTone(B5, 1);

    off_PWM();
}*/

/*
 * PA0,OE (purple)
 * PC0,R0 (red)
 * PC1,G0 (green)
 * PC2,B0 (blue)
 * PC3,R1 (red)
 * PC4,G1 (green)
 * PC5,B1 (blue)
 * PC6,A0 (white)
 * PC7,A1 (white)
 * PC8,A2 (yellow)
 * PC9,A3 (yellow)
 * PC10,A4 (orange)
 * PC11,CLK (orange)
 * PC12,LATCH (brown)
 * ground (black)
 */

void setup_gpio_led() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= 0x01555555;
    GPIOC->MODER &= 0xfd555555;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x00010000;
    GPIOA->MODER &= 0xfffdffff;
    GPIOC->OSPEEDR |= 0x003fffff;
    GPIOA->OSPEEDR |= 0x00030000;
}
void setup_timer15() {
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->ARR |= 19;
    TIM15->PSC |= 1;
    TIM15->DIER |= TIM_DIER_UIE;
    row_counter = 0;
    TIM15->CR1 |= TIM_CR1_CEN;
    NVIC_SetPriority(TIM15_IRQn, 1);
    NVIC->ISER[0] |= (1 << TIM15_IRQn);
}

//TIM15_IRQHandler
void update_board(int row_counter) {
    //Clears TIM15 UIF
    /*TIM6->SR &= ~TIM_SR_UIF;
     if (row_counter > 31) {
     row_counter = 0;
     }*/
    //row_counter = 31;
    GPIOA->ODR &= ~0x0100; //oe low

    for (int col_counter = 0; col_counter < 64; col_counter++) {
        GPIOC->ODR &= 0xffc0; //rgb0 100
        if (pixels[row_counter][col_counter]
                && (row_counter % 7 == 0 || col_counter % 7 == 0)) {
            GPIOC->ODR |= 0x0006; //rgb0 100
            GPIOC->ODR &= ~0x0001; //rgb0 100
        } else if (pixels[row_counter][col_counter]) {
            if (!given_display[row_counter / 7][col_counter / 7]) {
                GPIOC->ODR |= 0x0003; //rgb0 100
                GPIOC->ODR &= ~0x0004; //rgb0 100
            } else {
                GPIOC->ODR |= 0x0001; //rgb0 100
                GPIOC->ODR &= ~0x0006; //rgb0 100
            }
        }

        if (pixels[row_counter + 32][col_counter]
                && ((row_counter + 32) % 7 == 0 || (col_counter) % 7 == 0)) {
            GPIOC->ODR |= 0x0030; //rgb0 100
            GPIOC->ODR &= ~0x0008; //rgb0 100
        } else if (pixels[row_counter + 32][col_counter]) {
            if (!given_display[(row_counter + 32) / 7][col_counter / 7]) {
                GPIOC->ODR |= 0x0018; //rgb0 100
                GPIOC->ODR &= ~0x0020; //rgb0 100
            } else {
                GPIOC->ODR |= 0x0008; //rgb0 100
                GPIOC->ODR &= ~0x0030; //rgb0 100
            }
        }
        GPIOC->ODR |= 0x0800; //clk high
        GPIOC->ODR &= ~0x0800; //clk low
    }
    GPIOA->ODR |= 0x0100; //oe high
    GPIOC->ODR |= 0x1000; //lat high
    GPIOC->ODR &= 0xefff; //lat low

    int temp_ODR = GPIOC->ODR;
    temp_ODR &= ~0x07c0;
    temp_ODR |= row_counter << 6;
    GPIOC->ODR = temp_ODR;

    GPIOA->ODR &= ~0x0100; //oe low
    GPIOC->ODR &= 0xefff; //lat low
}
void setup_border() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            pixels[i][j] = 0;
        }
    }
    for (int i = 0; i < 64; i++) {
        pixels[0][i] = 1;
        pixels[7][i] = 1;
        pixels[14][i] = 1;
        pixels[21][i] = 1;
        pixels[28][i] = 1;
        pixels[35][i] = 1;
        pixels[42][i] = 1;
        pixels[49][i] = 1;
        pixels[56][i] = 1;
        pixels[63][i] = 1;
    }
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j += 7) {
            pixels[i][j] = 1;
        }
    }
}
void clean_block(int row, int col) {
    row = row * 7 + 1;
    col = col * 7 + 1;
    for (int i = row; i < row + 6; i++) {
        for (int j = col; j < col + 6; j++) {
            pixels[i][j] = 0;
        }
    }
}
void set_number(int row, int col, int num) {
    int mode = 1;
    clean_block(row, col);
    row = row * 7 + 1;
    col = col * 7 + 1;
    if (num == 0) {
        pixels[row][col] = 0;
        pixels[row][col + 1] = 0;
        pixels[row][col + 2] = 0;
        pixels[row][col + 3] = 0;
        pixels[row][col + 4] = 0;
        pixels[row][col + 5] = 0;

        pixels[row + 1][col] = 0;
        pixels[row + 1][col + 1] = 0;
        pixels[row + 1][col + 2] = 0;
        pixels[row + 1][col + 3] = 0;
        pixels[row + 1][col + 4] = 0;
        pixels[row + 1][col + 5] = 0;

        pixels[row + 2][col] = 0;
        pixels[row + 2][col + 1] = 0;
        pixels[row + 2][col + 2] = 0;
        pixels[row + 2][col + 3] = 0;
        pixels[row + 2][col + 4] = 0;
        pixels[row + 2][col + 5] = 0;

        pixels[row + 3][col] = 0;
        pixels[row + 3][col + 1] = 0;
        pixels[row + 3][col + 2] = 0;
        pixels[row + 3][col + 3] = 0;
        pixels[row + 3][col + 4] = 0;
        pixels[row + 3][col + 5] = 0;

        pixels[row + 4][col] = 0;
        pixels[row + 4][col + 1] = 0;
        pixels[row + 4][col + 2] = 0;
        pixels[row + 4][col + 3] = 0;
        pixels[row + 4][col + 4] = 0;
        pixels[row + 4][col + 5] = 0;

        pixels[row + 5][col] = 0;
        pixels[row + 5][col + 1] = 0;
        pixels[row + 5][col + 2] = 0;
        pixels[row + 5][col + 3] = 0;
        pixels[row + 5][col + 4] = 0;
        pixels[row + 5][col + 5] = 0;
    }

    if (num == 1) {
        pixels[row][col + 2] = mode;
        pixels[row + 1][col + 1] = mode;
        pixels[row + 1][col + 2] = mode;
        pixels[row + 2][col] = mode;
        pixels[row + 2][col + 2] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 4][col + 2] = mode;
        pixels[row + 5][col] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;
        pixels[row + 5][col + 5] = mode;
    } else if (num == 2) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 2][col + 2] = mode;
        pixels[row + 2][col + 3] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 4][col + 1] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;
    } else if (num == 3) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 2][col + 2] = mode;
        pixels[row + 2][col + 3] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;
    } else if (num == 4) {
        pixels[row + 1][col + 1] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 3][col + 3] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 4] = mode;

    } else if (num == 5) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 1] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 3][col + 3] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;
    } else if (num == 6) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 1] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 3][col + 3] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 1] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;

    } else if (num == 7) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 4] = mode;
    } else if (num == 8) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 1] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 3][col + 3] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 1] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 1] = mode;
        pixels[row + 5][col + 2] = mode;
        pixels[row + 5][col + 3] = mode;
        pixels[row + 5][col + 4] = mode;
    } else if (num == 9) {
        pixels[row][col + 1] = mode;
        pixels[row][col + 2] = mode;
        pixels[row][col + 3] = mode;
        pixels[row][col + 4] = mode;
        pixels[row + 1][col + 1] = mode;
        pixels[row + 1][col + 4] = mode;
        pixels[row + 2][col + 1] = mode;
        pixels[row + 2][col + 4] = mode;
        pixels[row + 3][col + 1] = mode;
        pixels[row + 3][col + 2] = mode;
        pixels[row + 3][col + 3] = mode;
        pixels[row + 3][col + 4] = mode;
        pixels[row + 4][col + 4] = mode;
        pixels[row + 5][col + 4] = mode;
    }
}

void update_display() {
    setup_border();
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            set_number(i, j, display[i][j]);
        }
    }
}

void spi_cmd(char b) {
    while ((SPI2->SR & SPI_SR_TXE) == 0)
        ;
    SPI2->DR = b;
}

void spi_data(char b) {
    while ((SPI2->SR & SPI_SR_TXE) == 0)
        ;
    SPI2->DR = b | 0x200;
}

void spi_init_lcd(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0x30ffffff;
    GPIOB->MODER |= 0x8a000000;
    GPIOB->AFR[1] &= 0x0f00ffff;
    //////////////////////////////////
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 |= 0xc000;
    SPI2->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0 | SPI_CR1_BR_2;
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR1 &= ~(SPI_CR1_CPOL);
    SPI2->CR1 &= ~(SPI_CR1_CPHA);
    SPI2->CR2 = 0x090c;
    SPI2->CR1 |= SPI_CR1_SPE;
    generic_lcd_startup();
}

void tty_init(void) {
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);
    GPIOA->MODER |= 2 << (2 * 9) | 2 << (2 * 10);
    GPIOA->AFR[1] |= 0x00000110;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    USART1->CR1 &= 0xfffffffe; //Disable UE
    USART1->CR1 &= 0xefffffff; //Set 1 in M[1]
    USART1->CR1 &= 0xffffefff; //Set 1 in M[0]
    USART1->CR1 &= ~USART_CR1_PCE;  //No Parity
    USART1->CR1 &= ~USART_CR1_OVER8; //OVER8=0
    USART1->BRR |= 0x01A1;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
    while ((USART1->ISR & USART_ISR_REACK) == 0)
        ;
    while ((USART1->ISR & USART_ISR_TEACK) == 0)
        ;
}
static int putchar_nonirq(int ch) {
    if (ch == '\n') {
        while ((USART1->ISR & USART_ISR_TC) == 0)
            ;
        USART1->TDR = '\r';
    }
    while ((USART1->ISR & USART_ISR_TC) == 0)
        ;
    USART1->TDR = ch;
    return ch;
}
int __io_putchar(int ch) {
    return putchar_nonirq(ch);
}
int validate_input1() {
    int num_to_return = 1;
    if (!(input1[1] == '|' || input1[1] == '-')) {
        //printf("Error: Second Character should be '|'(B on keypad)\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }
    if (!(input1[2] == '1' || input1[2] == '2' || input1[2] == '3'
            || input1[2] == '4' || input1[2] == '5' || input1[2] == '6'
            || input1[2] == '7' || input1[2] == '8' || input1[2] == '9'
            || input1[2] == '-')) {
        //printf("Error: Num should be between 1 and 9\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }
    if (!(input1[0] == 'C' || input1[0] == 'D')) {
        //printf("Error: Mode can only be either 'C' or 'D'\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }
    return num_to_return;
}
int validate_input2() {
    int num_to_return = 1;
    if (!(input2[1] == '|')) {
        //printf("Error: Second Character should be '|'(B on keypad)\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }
    if (!(input2[0] == '1' || input2[0] == '2' || input2[0] == '3'
            || input2[0] == '4' || input2[0] == '5' || input2[0] == '6'
            || input2[0] == '7' || input2[0] == '8' || input2[0] == '9')) {
        //printf("Error: Row should be between 1 and 9\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }

    if (!(input2[2] == '1' || input2[2] == '2' || input2[2] == '3'
            || input2[2] == '4' || input2[2] == '5' || input2[2] == '6'
            || input2[2] == '7' || input2[2] == '8' || input2[2] == '9')) {
        //printf("Error: Column should be between 1 and 9\n");
        //micro_wait(delay_time);
        num_to_return = 0;
    }

    return num_to_return;
}

void clear_line1() {
    input1[0] = ' ';
    input1[1] = ' ';
    input1[2] = ' ';
    line1[9] = ' ';
    line1[10] = ' ';
    line1[11] = ' ';
    line1[12] = ' ';
    line1[13] = ' ';
    line1[14] = ' ';
    line1[15] = ' ';
    display1(line1);
}

void clear_line2() {
    input2[0] = ' ';
    input2[1] = ' ';
    input2[2] = ' ';
    line2[8] = ' ';
    line2[9] = ' ';
    line2[10] = ' ';
    line2[11] = ' ';
    line2[12] = ' ';
    line2[13] = ' ';
    line2[14] = ' ';
    line1[15] = ' ';
    display2(line2);
}

int get_mode_num() {
    int valid_flag = 0;
    char key = key_being_pressed;
    if (key == '0') {
        clear_line1();
    }
    pos++;
    if (key != 'B') {
        line1[8 + pos] = key;
        input1[pos - 1] = key;
    } else {
        line1[8 + pos] = '|';
        input1[pos - 1] = '|';
    }
    if (line1[9] == 'D') {
        line1[10] = 'E';
        line1[11] = 'L';
        input1[1] = '-';
        input1[2] = '-';
        pos = 3;
    }
    display1(line1);
    if (pos == 3) {
        valid_flag = validate_input1();
    }
    return valid_flag;
}
int get_row_col() {
    int valid_flag = 0;
    char key = key_being_pressed;
    if (key == '0') {
        clear_line2();
    }
    pos++;
    if (key != 'B') {
        line2[7 + pos] = key;
        input2[pos - 1] = key;
    } else {
        line2[7 + pos] = '|';
        input2[pos - 1] = '|';
    }
    display2(line2);
    if (pos == 3) {
        valid_flag = validate_input2();
    }
    return valid_flag;
}

void init_keypad() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= 0xffffff00;
    GPIOA->MODER |= 0x00000055;
    GPIOB->MODER &= 0xffff00ff;
    GPIOB->PUPDR |= 0x0000aa00;
}

void setup_timer6() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->ARR |= 1;
    TIM6->PSC |= 1;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
    NVIC_SetPriority(TIM6_DAC_IRQn, 0);
    NVIC->ISER[0] |= (1 << TIM6_DAC_IRQn);
}
//TIM6_DAC_IRQHandler
void TIM15_IRQHandler() {
    timer_random++;
    TIM15->SR &= ~TIM_SR_UIF;
    int row = ((GPIOB->IDR) >> 4) & 0xf;
    int index = col << 2;
    history[index] = (history[index] << 1);
    history[index] |= (row & 0x1);
    history[index + 1] = (history[index + 1] << 1);
    history[index + 1] |= ((row >> 1) & 0x1);
    history[index + 2] = (history[index + 2] << 1);
    history[index + 2] |= ((row >> 2) & 0x1);
    history[index + 3] = (history[index + 3] << 1);
    history[index + 3] |= ((row >> 3) & 0x1);
    col++;
    if (col > 3) {
        col = 0;
    }
    GPIOA->ODR = (1 << col);

    //int8_t temp_history[16];

    int i;
    if (key_being_pressed == '\0') {
        for (int i = 0; i < 16; i++) {
            if (history[i] == 1) {
                key_being_pressed = char_lookup[i];
                key_press_ack = 0;
            }
        }
    } else if (key_press_ack == 1) {
        for (int i = 0; i < 16; i++) {
            if (history[i] == -2) {
                key_being_pressed = '\0';
                key_press_ack = 0;
            }
        }
    }
}

int choose_difficulty() {
    int pos = 0;
    char key = key_being_pressed;
    if (key == '0' || key == '#') {
        clear_line1();
        key_press_ack = 1;
    }
    if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5') {
        line3[13 + pos] = key;
        level = (int) key - 48;
        pos = 1;
        key_press_ack = 1;
    } else {
        //printf("\nError: Please enter a number between 1 and 5\n");
        //printf("Please Choose Difficulty: ");
        key_press_ack = 1;
        note_num = 0;
        song = SAD;
    }
    display1(line3);
    if (pos == 1) {
        return 1;
    } else {
        return 0;
    }
}
int is_valid(int row, int col, int num) {
    if (rowCheck(row, col, num, 1) && colCheck(row, col, num, 1)
            && gridCheck(row, col, num, 1)) {
        return 1;
    }
    return 0;
}
int main(void) {
    step1();
    int temp1, temp2;
    int game_flag = 1;
    setup_gpio_led();
    init_keypad();
    setup_timer15();
    init_wavetable();
    setup_gpio();
    setup_PWM_TIM14();
    setup_TIM1();
    on_PWM();
    song = TETRIS; //startup tone
    update_freq(E5);
    int out, in;
    /*tty_init();
     //printf("\n------------------\n");
     //printf("New Game Started\n");
     //printf("------------------\n");
     //printf("Please Choose Difficulty: ");*/
    display1(line3);
    display2("(b/w 1 & 5)");
    update_display();

    enum phase game_phase = WAIT_FOR_DIFFICULTY;
    int x = 0;
    while (1) {
        /*for (row_counter = 0; row_counter < 32; row_counter++) {
         update_board(row_counter);
         }*/
        if (x > 31) {
            x = 0;
        }
        update_board(x);
        x++;
        if (key_being_pressed != '\0' && key_press_ack == 0) {
            if (game_phase == GAME_OFF)
                song = NONE;
            if (game_phase == WAIT_FOR_DIFFICULTY) {
                level = 0;
                if (choose_difficulty() == 1) {
                    note_num = 0;
                    song = HAPPY;


                    //printf("%d\n", level);
                    micro_wait(750000);
                    generate_game(timer_random);
                    for (int i = 0; i < 9; i++) {
                        //printf("%d--", i + 1);
                        for (int j = 0; j < 9; j++) {
                            //printf("%d.", display[i][j]);
                            if (display[i][j]) {
                                given_display[i][j] = 1;
                            } else {
                                given_display[i][j] = 0;
                            }
                        }
                        //printf("\n");
                    }
                    /*setup_timer15();
                     ////////Update Initial Display*/
                    update_display();
                    /*int x = 0;
                     //setup_timer15();
                     while (1) {
                     test_fun(x);
                     x++;
                     if (x > 31) {
                     x = 0;
                     }
                     }
                     //Disabled to test TIM15_IRQHandler*/
                    display1(line1);
                    display2(line2);
                    //setup_timer6();

                    int game_flag = 1;
                    int out, in;
                    //printf("--------------------------------------------\n");
                    //printf("Press * to give input\n");
                    //printf("Press 0 anytime to reset current line\n");
                    //printf("Press # anytime to reset the whole input\n");
                    //printf("--------------------------------------------\n");
                    game_phase = WAIT_FOR_INPUT_TYPE;
                    key_press_ack = 1;
                }
            } else if (game_phase == WAIT_FOR_INPUT_TYPE) {
                char key = key_being_pressed;
                if (key == '#') {
                    clear_line1();
                    clear_line2();
                    //printf("Success: Input Reset\n");
                    key_press_ack = 1;
                    pos = 0;
                } else if (key == '*') {
                    game_phase = WAIT_FOR_MODE;
                    key_press_ack = 1;
                    note_num = 0;
                    song = TRI;
                    //printf("Please choose the mode and number to insert/delete\n");
                }
            }
            //clear_line1();
            //clear_line2();
            else if (game_phase == WAIT_FOR_MODE) {
                char key = key_being_pressed;
                if (key == '#') {
                    clear_line1();
                    clear_line2();
                    ////printf("Success: Input Reset\n");
                    key_press_ack = 1;
                    pos = 0;

                } else {
                    if (get_mode_num() == 1) {
                        game_phase = GET_ROW_COL;
                        pos = 0;
                        ////printf("Please choose a row and a column\n");
                    }
                    key_press_ack = 1;
                }
            } else if (game_phase == GET_ROW_COL) {
                char key = key_being_pressed;
                if (key == '#') {
                    clear_line1();
                    clear_line2();
                    ////printf("Success: Input Reset\n");
                    key_press_ack = 1;
                    pos = 0;
                    game_phase = WAIT_FOR_MODE;
                } else {
                    if (get_row_col() == 1) {
                        game_phase = ADJUSTING_BOARD;
                        pos = 0;
                    }
                    key_press_ack = 1;
                }
            }
            /*if (hash_pressed) {
             hash_pressed = 0;
             //printf("Success: Input Reset\n");
             continue;
             }*/
            /*if (hash_pressed) {
             hash_pressed = 0;
             //printf("Success: Input Reset\n");
             continue;
             }*/

            if (game_phase == ADJUSTING_BOARD) {
                game_flag = 1;
                char key = key_being_pressed;
                if (key == '#') {
                    clear_line1();
                    clear_line2();
                    ////printf("Success: Input Reset\n");
                    key_press_ack = 1;
                    pos = 0;
                    game_phase = WAIT_FOR_MODE;
                }
                if (input1[0] == 'C') {
                    game_phase = WAIT_FOR_MODE;
                    if (!given_display[(int) input2[0] - 49][(int) input2[2]
                            - 49]) {
                        if (is_valid((int) input2[0] - 49, (int) input2[2] - 49,
                                (int) input1[2] - 48)) {
                            display[((int) input2[0] - 49)][((int) input2[2]
                                    - 49)] = (int) input1[2] - 48;
                            //printf("Success: Number Succesfully Inserted\n");
                            note_num = 0;
                            song = HAPPY;

                            /*for (int i = 0; i < 9; i++) {
                             //printf("%d--", i + 1);
                             for (int j = 0; j < 9; j++) {
                             //printf("%d.", display[i][j]);
                             }
                             //printf("\n");
                             }*/
                            update_display();
                        }
                    } else {
                        //printf("Error: You cannot replace a given number\n");
                        note_num = 0;
                        song = SAD;
                    }
                } else {
                    game_phase = WAIT_FOR_MODE;
                    if (!given_display[(int) input2[0] - 49][(int) input2[2]
                            - 49]) {
                        if (display[(int) input2[0] - 49][(int) input2[2] - 49]) {
                            display[(int) input2[0] - 49][(int) input2[2] - 49] =
                                    0;
                            //printf("Success: Number Succesfully Deleted at %dx%d\n", (int) input2[0] - 49, (int) input2[2] - 49);
                            note_num = 0;
                            song = HAPPY;
                            update_display();

                            /*for (int i = 0; i < 9; i++) {
                             //printf("%d--", i + 1);
                             for (int j = 0; j < 9; j++) {
                             //printf("%d.", display[i][j]);
                             }
                             //printf("\n");
                             }*/
                        } else {
                            //printf("Error: There is nothing to delete there\n");
                            note_num = 0;
                            song = SAD;
                        }
                    } else {
                        //printf("Error: You cannot delete a given number\n");
                        note_num = 0;
                        song = SAD;
                    }
                }

                //CHECKS FOR GAME OVER
                for (out = 0; out < 9; out++) {
                    for (in = 0; in < 9; in++) {
                        if (display[out][in] == 0) {
                            game_flag = 0;
                            break;
                        }
                    }
                }
                if (game_flag == 1)
                    game_phase = GAME_FINISHED;
            }
            //IF GAME_PHASE REACHES UNNATAINABLE VALUE
            /*
             //micro_wait(1000000);
             if (input1[1] != ' ') {
             input1[0] = ' ';
             input1[1] = ' ';
             }

             //clear_line1();
             if (input2[1] != ' ') {
             input2[0] = ' ';
             input2[1] = ' ';
             }
             */
            //clear_line2();
            else if (game_phase == GAME_OFF) {
                game_phase == WAIT_FOR_DIFFICULTY;
            }
        }

        if (game_phase == GAME_FINISHED) {
            //printf("------------------------CONGRATS--------------------------\n");
            //printf("You Successfully completed the game. Please Reset to Start a new game\n");
            game_phase = GAME_OFF;
            display1(line4);
            display2(line_clear);
            note_num = 0;
            song = TRI;
        }

    }
}

