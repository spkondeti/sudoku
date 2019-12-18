#include <stdio.h>
#include <stdlib.h>
void generate_game();
int gridCheck(int row, int col, int num, int mode);
int rowCheck(int row, int col, int num, int mode);
int colCheck(int row, int col, int num, int mode);
int auto_generate(int i, int j);
void printSudoku();
int level;
int display[9][9];
int given_display[9][9];
void generate_game(long long seed) {
    int num;
    srand(seed);
    for (int x = 0; x < 9; x += 3) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                num = (rand() % 9) + 1;
                while (!gridCheck(x, x, num, 0)) {
                    num = (rand() % 9) + 1;
                }
                display[x + i][x + j] = num;
            }
        }
    }
    auto_generate(0, 3);

    for (int i = 0; i< 9; i++){
        for (int j = 0; j < 9; j++){
            given_display[i][j] = 1;
        }
    }
    int blocks_to_remove;
    if (level == 1) {
        blocks_to_remove = 1;
    } else if (level == 2) {
        blocks_to_remove = 4;
    } else if (level == 3) {
        blocks_to_remove = 15;
    } else if (level == 4) {
        blocks_to_remove = 20;
    } else if (level == 5) {
        blocks_to_remove = 25;
    }
    int r, c, counter1;
    counter1 = 0;
    while (counter1 < blocks_to_remove) {
        r = rand() % 9;
        c = rand() % 9;
        if (display[r][c] != 0) {
            display[r][c] = 0;
            given_display[r][c] = 0;
            counter1++;
        }
    }
    return;
}

int gridCheck(int row, int col, int num, int mode) {
    int row_cat = 0;
    int col_cat = 0;
    if (row >= 0 && row < 3 && col >= 0 && col < 3) {
        row_cat = 1;
        col_cat = 1;
    } else if (row >= 0 && row < 3 && col >= 3 && col < 6) {
        row_cat = 1;
        col_cat = 2;
    } else if (row >= 0 && row < 3 && col >= 6 && col < 9) {
        row_cat = 1;
        col_cat = 3;
    } else if (row >= 3 && row < 6 && col >= 0 && col < 3) {
        row_cat = 2;
        col_cat = 1;
    } else if (row >= 3 && row < 6 && col >= 3 && col < 6) {
        row_cat = 2;
        col_cat = 2;
    } else if (row >= 3 && row < 6 && col >= 6 && col < 9) {
        row_cat = 2;
        col_cat = 3;
    } else if (row >= 6 && row < 9 && col >= 0 && col < 3) {
        row_cat = 3;
        col_cat = 1;
    } else if (row >= 6 && row < 9 && col >= 3 && col < 6) {
        row_cat = 3;
        col_cat = 2;
    } else if (row >= 6 && row < 9 && col >= 6 && col < 9) {
        row_cat = 3;
        col_cat = 3;
    }

    for (int i = 3 * (row_cat - 1); i < (3 * (row_cat - 1) + 3); i++) {
        for (int j = 3 * (col_cat - 1); j < (3 * (col_cat - 1) + 3); j++) {
            if (display[i][j] == num) {
                if (mode) {
                    printf("Error: There is a match in the same sub-grid\n");
                    // SAD
                }
                return 0;
            }
        }
    }
    return 1;
}

int rowCheck(int row, int col, int num, int mode) {
    for (int j = 0; j < 9; j++)
        if (display[row][j] == num) {
            if (mode) {
                printf("Error: There is a match in the same row\n");
                // SAD
            }
            return 0;
        }
    return 1;
}

int colCheck(int row, int col, int num, int mode) {
    for (int j = 0; j < 9; j++)
        if (display[j][col] == num) {
            if (mode) {
                printf("Error: There is a match in the same column\n");
                // SAD
            }
            return 0;
        }
    return 1;
}

int auto_generate(int i, int j) {
    if (j >= 9 && i < 8) {
        i = i + 1;
        j = 0;
    }
    if (i >= 9 && j >= 9)
        return 1;

    if (i < 3) {
        if (j < 3)
            j = 3;
    } else if (i < 6) {
        if (j == (int) (i / 3) * 3)
            j = j + 3;
    } else if (j == 6) {
        i = i + 1;
        j = 0;
        if (i >= 9)
            return 1;
    }

    for (int num = 1; num <= 9; num++) {
        if (rowCheck(i, j, num, 0) && colCheck(i, j, num, 0)
                && gridCheck(i - i % 3, j - j % 3, num, 0)) {
            display[i][j] = num;
            if (auto_generate(i, j + 1))
                return 1;
            display[i][j] = 0;
        }
    }
    return 0;
}

void printSudoku() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++)
            printf("%d.", display[i][j]);
        printf("\n");
    }
}


