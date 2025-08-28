#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define N 3

/* ===================== Game State ===================== */
typedef struct {
    char board[N][N];   // 'X', 'O', or ' '
    char current;       // whose turn: 'X' or 'O'
    int  moves;         // 0..9
} Game;

/* players */
typedef struct {
    int vs_bot;         // 0 = human vs human, 1 = human vs bot
    char human_symbol;  // 'X' or 'O'
    char bot_symbol;    // 'X' or 'O' (if vs_bot = 1)
    int bot_hard;       // 0 = easy(random), 1 = hard(minimax)
} Mode;

/* ===================== Helpers ===================== */
static void reset_board(Game *g) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            g->board[r][c] = ' ';
    g->current = 'X';
    g->moves = 0;
}

static void print_instructions(void) {
    puts("=========== TIC-TAC-TOE ===========");
    puts("You can enter your move as either:");
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

/* Returns: 'X' winner, 'O' winner, 'D' draw, or ' ' (continue) */
static char check_status(const Game *g) {
    const char (*b)[N] = g->board;
    for (int i = 0; i < N; ++i) {
        if (b[i][0] != ' ' && b[i][0] == b[i][1] && b[i][1] == b[i][2]) return b[i][0];
        if (b[0][i] != ' ' && b[0][i] == b[1][i] && b[1][i] == b[2][i]) return b[0][i];
    }
    if (b[1][1] != ' ' && ((b[0][0] == b[1][1] && b[1][1] == b[2][2]) ||
                           (b[0][2] == b[1][1] && b[1][1] == b[2][0])))
        return b[1][1];
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

/* Parse "1..9" OR "row col" → 0-based row/col. Returns 1 on success. */
static int parse_move(char *line, int *row, int *col) {
    while (*line && isspace((unsigned char)*line)) line++;
    int n;
    if (sscanf(line, "%d", &n) == 1 && n >= 1 && n <= 9) {
        n--; *row = n / 3; *col = n % 3; return 1;
    }
    int r, c;
    if (sscanf(line, "%d %d", &r, &c) == 2) {
        r--; c--;
        if (r >= 0 && r < 3 && c >= 0 && c < 3) { *row = r; *col = c; return 1; }
    }
    return 0;
}

static void swap_player(Game *g) {
    g->current = (g->current == 'X') ? 'O' : 'X';
}

/* ===================== Bot (AI) ===================== */

/* Minimax with alpha-beta pruning.
   Returns a score from the bot's perspective:
   +10 - depth : bot win (prefer quicker)
   depth - 10  : human win (prefer slower losses)
   0           : draw
*/
static int minimax(Game *g, char bot, char human, int depth, int alpha, int beta, int maximizing) {
    char status = check_status(g);
    if (status == bot)   return 10 - depth;
    if (status == human) return depth - 10;
    if (status == 'D')   return 0;

    if (maximizing) {
        int best = -1000;
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (g->board[r][c] == ' ') {
                    g->board[r][c] = bot; g->moves++;
                    int val = minimax(g, bot, human, depth + 1, alpha, beta, 0);
                    g->board[r][c] = ' '; g->moves--;
                    if (val > best) best = val;
                    if (val > alpha) alpha = val;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (g->board[r][c] == ' ') {
                    g->board[r][c] = human; g->moves++;
                    int val = minimax(g, bot, human, depth + 1, alpha, beta, 1);
                    g->board[r][c] = ' '; g->moves--;
                    if (val < best) best = val;
                    if (val < beta)  beta = val;
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }
}

static void best_move_minimax(Game *g, char bot, char human, int *best_r, int *best_c) {
    int best_val = -1000;
    *best_r = -1; *best_c = -1;

    /* Small heuristic: if center is open, prefer it early. */
    if (g->board[1][1] == ' ') {
        *best_r = 1; *best_c = 1; /* still run search to confirm optimality */
    }

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (g->board[r][c] == ' ') {
                g->board[r][c] = bot; g->moves++;
                int move_val = minimax(g, bot, human, 0, -1000, 1000, 0);
                g->board[r][c] = ' '; g->moves--;
                if (move_val > best_val) {
                    best_val = move_val;
                    *best_r = r; *best_c = c;
                }
            }
        }
    }
}

/* Easy mode: random valid move */
static void random_move(Game *g, int *r, int *c) {
    int empties[9][2], k = 0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (g->board[i][j] == ' ') { empties[k][0] = i; empties[k][1] = j; k++; }
    if (k == 0) { *r = -1; *c = -1; return; }
    int idx = rand() % k;
    *r = empties[idx][0];
    *c = empties[idx][1];
}

/* Make bot move according to chosen difficulty */
static void bot_make_move(Game *g, const Mode *m) {
    int r=-1,c=-1;
    if (m->bot_hard) {
        best_move_minimax(g, m->bot_symbol, m->human_symbol, &r, &c);
    } else {
        random_move(g, &r, &c);
    }
    if (r != -1) {
        g->board[r][c] = m->bot_symbol;
        g->moves++;
        printf("Bot plays at %d %d\n", r+1, c+1);
    }
}

/* ===================== Menu / Setup ===================== */

static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, sz, stdin)) return 0;
    size_t n = strlen(buf);
    if (n && buf[n-1] == '\n') buf[n-1] = '\0';
    return 1;
}

static void configure_mode(Mode *m) {
    char line[64];
    m->vs_bot = 1;
    m->human_symbol = 'X';
    m->bot_symbol = 'O';
    m->bot_hard = 1;

    puts("Play Mode:");
    puts("  1) Human vs Bot");
    puts("  2) Human vs Human");
    printf("Choose (1/2) [default 1]: ");
    if (read_line(line, sizeof(line)) && line[0] == '2') m->vs_bot = 0;

    if (m->vs_bot) {
        printf("Choose your symbol X or O [default X]: ");
        if (read_line(line, sizeof(line))) {
            if (line[0] == 'x' || line[0] == 'X') m->human_symbol = 'X';
            else if (line[0] == 'o' || line[0] == 'O') m->human_symbol = 'O';
        }
        m->bot_symbol = (m->human_symbol == 'X') ? 'O' : 'X';

        puts("Bot difficulty:");
        puts("  1) Easy (random)");
        puts("  2) Hard (unbeatable)");
        printf("Choose (1/2) [default 2]: ");
        if (read_line(line, sizeof(line)) && line[0] == '1') m->bot_hard = 0;
    }

    printf("\nSummary: ");
    if (m->vs_bot) {
        printf("Human(%c) vs Bot(%c), Difficulty: %s\n\n",
               m->human_symbol, m->bot_symbol, m->bot_hard ? "Hard" : "Easy");
    } else {
        printf("Human vs Human\n\n");
    }
}

/* ===================== Main ===================== */
int main(void) {
    srand((unsigned)time(NULL));

    Game g;
    Mode m;
    char line[128];

    print_instructions();

    for (;;) {
        configure_mode(&m);
        reset_board(&g);

        /* If bot is 'X', it moves first automatically */
        if (m.vs_bot && m.bot_symbol == 'X') {
            bot_make_move(&g, &m);
            g.current = 'O';
        } else {
            g.current = 'X';
        }

        /* --------- Turn Loop --------- */
        while (1) {
            print_board(&g);

            char status = check_status(&g);
            if (status == 'X' || status == 'O') {
                printf("Player %c wins!\n", status);
                break;
            } else if (status == 'D') {
                puts("It's a draw!");
                break;
            }

            if (!m.vs_bot || g.current == m.human_symbol) {
                /* Human turn */
                printf("Player %c, enter your move: ", g.current);
                if (!fgets(line, sizeof(line), stdin)) { puts("\nInput closed. Exiting."); return 0; }
                int row, col;
                if (!parse_move(line, &row, &col)) {
                    puts("Invalid input. Use 1-9 or 'row col' (e.g., 2 3). Try again.");
                    continue;
                }
                if (!place_if_empty(&g, row, col)) {
                    puts("That cell is not available. Choose another.");
                    continue;
                }
            } else {
                /* Bot turn */
                bot_make_move(&g, &m);
            }

            swap_player(&g);
        }

        /* Replay? */
        printf("\nPlay again? (y/n): ");
        if (!fgets(line, sizeof(line), stdin)) { puts("\nInput closed. Exiting."); return 0; }
        if (line[0] != 'y' && line[0] != 'Y') {
            puts("Thanks for playing!");
            break;
        }
        puts("");
    }

    return 0;
}
