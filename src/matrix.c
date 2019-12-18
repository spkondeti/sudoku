/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
int row_counter;
int col_counter;
uint8_t pixels[64][64];
void setup_gpio() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER |= 0x01555555;
    GPIOC->MODER &= 0xfd555555;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x00000001;
    GPIOA->MODER &= 0xfffffffd;
    GPIOC->OSPEEDR |= 0x003fffff;
    //GPIOA->OSPEEDR |= 0x00000003;
}
void setup_timer15() {
    /* Student code goes here */
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->CR1 &= ~TIM_CR1_DIR; // clear it to count up
    TIM15->ARR |= 1;
    TIM15->PSC |= 1;
    TIM15->CCR3 = 0;
    TIM15->CCMR2 &= ~TIM_CCMR2_OC3M_2; // Turn off bit 2.
    TIM15->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0; // Enable output for channel 3 active-high output
    TIM15->CCER |= TIM_CCER_CC3E;
    TIM15->DIER |= TIM_DIER_UIE;
    row_counter = 0;
    TIM15->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = (1 << TIM15_IRQn);
}
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
void test_fun(int row_counter) {
    for (int col_counter = 0; col_counter < 64; col_counter++) {
        if (pixels[row_counter][col_counter]) {
            GPIOC->ODR |= 0x0001; //rgb0 100
            GPIOC->ODR &= 0xfff9; //rgb0 100
        } else {
            GPIOC->ODR &= 0xfff8; //rgb0 100
        }
        if (pixels[row_counter + 32][col_counter]) {
            GPIOC->ODR |= 0x0008; //rgb0 100
            GPIOC->ODR &= 0xffcf; //rgb0 100
        }
        else{
            GPIOC->ODR &= 0xffc7; //rgb0 100
        }
        GPIOC->ODR |= 0x0800; //clk high
        GPIOC->ODR &= ~0x0800; //clk low
    }
    GPIOA->ODR |= 0x0001; //oe high
    GPIOC->ODR |= 0x1000; //lat high
    GPIOC->ODR &= 0xefff; //lat low
    if (row_counter & 0x10) {
        GPIOC->ODR |= 0x0400;
    } else {
        GPIOC->ODR &= ~0x0400;
    }
    if (row_counter & 0x8) {
        GPIOC->ODR |= 0x0200;
    } else {
        GPIOC->ODR &= ~0x0200;
    }
    if (row_counter & 0x4) {
        GPIOC->ODR |= 0x0100;
    } else {
        GPIOC->ODR &= ~0x0100;
    }
    if (row_counter & 0x2) {
        GPIOC->ODR |= 0x0080;
    } else {
        GPIOC->ODR &= ~0x0080;
    }
    if (row_counter & 0x1) {
        GPIOC->ODR |= 0x0040;
    } else {
        GPIOC->ODR &= ~0x0040;
    }
    GPIOA->ODR &= ~0x0001; //oe low
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
    //At the end of this function pixels has all 1's at the vertical and horizontal line positions of the matrix
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
    if (num == 0) {
        return;
    }
    clean_block(row, col);
    row = row * 7 + 1;
    col = col * 7 + 1;
    if (num == 1) {
        pixels[row][col + 2] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 1][col + 2] = 1;
        pixels[row + 2][col] = 1;
        pixels[row + 2][col + 2] = 1;
        pixels[row + 3][col + 2] = 1;
        pixels[row + 4][col + 2] = 1;
        pixels[row + 5][col] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;
        pixels[row + 5][col + 5] = 1;
    } else if (num == 2) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 2][col + 2] = 1;
        pixels[row + 2][col + 3] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 1] = 1;
        pixels[row + 4][col + 1] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;
    } else if (num == 3) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 2][col + 2] = 1;
        pixels[row + 2][col + 3] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;
    } else if (num == 4) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 2][col + 2] = 1;
        pixels[row + 2][col + 3] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 4] = 1;
    } else if (num == 5) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 3][col + 1] = 1;
        pixels[row + 3][col + 2] = 1;
        pixels[row + 3][col + 3] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;
    } else if (num == 6) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 3][col + 1] = 1;
        pixels[row + 3][col + 2] = 1;
        pixels[row + 3][col + 3] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 1] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;

    } else if (num == 7) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 4] = 1;
    } else if (num == 8) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 1] = 1;
        pixels[row + 3][col + 2] = 1;
        pixels[row + 3][col + 3] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 1] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 1] = 1;
        pixels[row + 5][col + 2] = 1;
        pixels[row + 5][col + 3] = 1;
        pixels[row + 5][col + 4] = 1;
    } else if (num == 9) {
        pixels[row][col + 1] = 1;
        pixels[row][col + 2] = 1;
        pixels[row][col + 3] = 1;
        pixels[row][col + 4] = 1;
        pixels[row + 1][col + 1] = 1;
        pixels[row + 1][col + 4] = 1;
        pixels[row + 2][col + 1] = 1;
        pixels[row + 2][col + 4] = 1;
        pixels[row + 3][col + 1] = 1;
        pixels[row + 3][col + 2] = 1;
        pixels[row + 3][col + 3] = 1;
        pixels[row + 3][col + 4] = 1;
        pixels[row + 4][col + 4] = 1;
        pixels[row + 5][col + 4] = 1;
    }
}

void update_display(){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            set_number(i,j,display[i][j]);
        }
    }
}

/*int main(void) {
    setup_border();
    setup_gpio();
    int x = 0;
    while (1) {
        if (x > 31) {
            x = 0;
        }
        set_number(2,0,8);
        test_fun(x);
        x++;
    }
    //setup_timer15();
}*/

