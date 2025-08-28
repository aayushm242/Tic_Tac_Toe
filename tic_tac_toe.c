#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define N 3

/* --------- Game State --------- */
typedef struct {
    char board[N][N];   // 'X', 'O', or ' '
    char current;       // 'X' or 'O'
    int  moves;         // 0..9
} Game;

/* --------- Helpers --------- */
static void reset_board(Game *g) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            g->board[r][c] = ' ';
    g->current = 'X';
    g->moves = 0;
}

static void print_instructions(void) {
    puts("=========== TIC-TAC-TOE ===========");
    puts("Play by entering either:");
    puts("  • A single number 1-9 (as below), or");
    puts("  • Row and column as: 1 3  (row=1, col=3)\n");
    puts("Cell numbers:");
    puts("  1 | 2 | 3");
    puts(" ---+---+---");
    puts("  4 | 5 | 6");
    puts(" ---+---+---");
    puts("  7 | 8 | 9\n");
}

static void print_board(const Game *g) {
    printf("\n");
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            printf(" %c ", g->board[r][c]);
            if (c < N - 1) printf("|");
        }
        if (r < N - 1) printf("\n---+---+---\n");
    }
    printf("\n\n");
}

/* Returns:
   'X' if X has won,
   'O' if O has won,
   'D' if draw,
   ' ' (space) if game continues.
*/
static char check_status(const Game *g) {
    const char (*b)[N] = g->board;
    // Rows & Cols
    for (int i = 0; i < N; ++i) {
        if (b[i][0] != ' ' && b[i][0] == b[i][1] && b[i][1] == b[i][2]) return b[i][0];
        if (b[0][i] != ' ' && b[0][i] == b[1][i] && b[1][i] == b[2][i]) return b[0][i];
    }
    // Diagonals
    if (b[1][1] != ' ' && ((b[0][0] == b[1][1] && b[1][1] == b[2][2]) ||
                           (b[0][2] == b[1][1] && b[1][1] == b[2][0]))) {
        return b[1][1];
    }
    // Draw?
    if (g->moves >= 9) return 'D';
    return ' ';
}

static int place_if_empty(Game *g, int row, int col) {
    if (row < 0 || row >= N || col < 0 || col >= N) return 0;
    if (g->board[row][col] != ' ') return 0;
    g->board[row][col] = g->current;
    g->moves++;
    return 1;
}

/* Parse input line.
   Accepts "1-9" or "row col".
   Returns 1 on success and sets (row,col), 0 on invalid input.
*/
static int parse_move(char *line, int *row, int *col) {
    while (*line && isspace((unsigned char)*line)) line++;

    int n = -1, r = -1, c = -1;
    if (sscanf(line, "%d", &n) == 1 && (n >= 1 && n <= 9)) {
        n--;       // 0..8
        r = n / 3; // 0..2
        c = n % 3; // 0..2
        *row = r; *col = c;
        return 1;
    }

    int rr, cc;
    if (sscanf(line, "%d %d", &rr, &cc) == 2) {
        rr--; cc--;
        if (rr >= 0 && rr < 3 && cc >= 0 && cc < 3) {
            *row = rr; *col = cc;
            return 1;
        }
    }
    return 0;
}

static void swap_player(Game *g) {
    g->current = (g->current == 'X') ? 'O' : 'X';
}

/* --------- Main Game Loop --------- */
int main(void) {
    Game g;
    char line[128];

    print_instructions();

    for (;;) {
        reset_board(&g);

        while (1) {
            print_board(&g);
            printf("Player %c, enter your move: ", g.current);

            if (!fgets(line, sizeof(line), stdin)) {
                puts("\nInput closed. Exiting.");
                return 0;
            }

            int row, col;
            if (!parse_move(line, &row, &col)) {
                puts("Invalid input. Use 1-9 or 'row col' (e.g., 2 3). Try again.");
                continue;
            }

            if (!place_if_empty(&g, row, col)) {
                puts("That cell is not available. Choose another.");
                continue;
            }

            char status = check_status(&g);
            if (status == 'X' || status == 'O') {
                print_board(&g);
                printf("Player %c wins!\n", status);
                break;
            } else if (status == 'D') {
                print_board(&g);
                puts("It's a draw!");
                break;
            }

            swap_player(&g);
        }

        printf("\nPlay again? (y/n): ");
        if (!fgets(line, sizeof(line), stdin)) {
            puts("\nInput closed. Exiting.");
            return 0;
        }
        if (line[0] != 'y' && line[0] != 'Y') {
            puts("Thanks for playing!");
            break;
        }
        puts("");
    }

    return 0;
}
