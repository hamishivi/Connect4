#include <iostream>
#include <stdlib.h>
#include <limits>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define WIDTH 7
#define HEIGHT 6

uint64_t board_y = 0;
uint64_t board_r = 0;
bool is_red;
bool use_alpha_beta = true;
int col_search_order[] = {3, 2, 4, 1, 5, 0, 6};

// basic concept: shift by one, and to clear board,
// shift by two and then & to remove all but a possible 4-match
// only two shifts, two &s per direction!
bool is_win(uint64_t board)
{
    // vertical
    uint64_t y = board & (board >> 1);
    if (y & (y >> 2))
        return true;
    // horizontal
    y = board & (board >> 8);
    if (y & (y >> 2 * 8))
        return true;
    // / diagonal
    y = board & (board >> 9);
    if (y & (y >> 2 * 9))
        return true;
    // \ diagonal
    y = board & (board >> 7);
    if (y & y >> 2 * 7)
        return true;
    return false;
}

int utility(uint64_t board_r, uint64_t board_y)
{
    if (is_win(board_r))
        return 1000000;
    if (is_win(board_y))
        return -1000000;
    return 0;
}

uint64_t flip_bit(uint64_t board, int n_bit)
{
    board |= 1ULL << n_bit;
    return board;
}

int get_bit(uint64_t board, int i, int j)
{
    return !!((1ULL << ((i * 8) + j)) & board);
}

// Counts set bits by constantly clearing lowest bit
// From https://graphics.stanford.edu/~seander/bithacks.html
int number_of_set_bits(uint64_t board)
{
    unsigned int cnt;
    for (cnt = 0; board; cnt++)
    {
        board &= board - 1; // clear the least significant bit set
    }
    return cnt;
}

// counts num in a row that can be converted to 4-a-row
// we want the most convertible possibilities at once, so we don't double count
// empty spaces
int num_in_a_row(uint64_t board, uint64_t opp_board)
{
    unsigned int num_of_rows = 0;
    // Horizontal
    int hoz_cache[HEIGHT][WIDTH] = {0};
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (hoz_cache[i][j])
                continue;
            unsigned int cnt = 0;
            int adder = 0;
            while (get_bit(board, i, j + adder) && j + adder < WIDTH)
            {
                hoz_cache[i][j + adder] = 1;
                cnt++;
                adder++;
            }
            // only care about 2-3 in a rows.
            if (cnt < 1)
                continue;
            int mod = cnt;
            while (!get_bit(opp_board, i, j + adder) && j + adder < WIDTH)
            {
                if (hoz_cache[i][j + adder])
                    break; // this is a square another 4-in-a-row uses
                hoz_cache[i][j + adder] = 1;
                cnt++;
                adder++;
            }
            // if cnt is larger than 4 we have a poss 4-in-a-row
            // weight 1, 2, 3, more heavier
            if (cnt >= 4)
                num_of_rows += mod * 2;
        }
    }
    // Vertical
    int ver_cache[HEIGHT][WIDTH] = {0};
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (ver_cache[i][j])
                continue;
            unsigned int cnt = 0;
            int adder = 0;
            while (get_bit(board, i + adder, j) && i + adder < HEIGHT)
            {
                ver_cache[i + adder][j] = 1;
                cnt++;
                adder++;
            }
            if (cnt < 1)
                continue;
            int mod = cnt;
            while (!get_bit(opp_board, i + adder, j) && i + adder < HEIGHT)
            {
                if (ver_cache[i + adder][j])
                    break; // this is a square another 4-in-a-row uses
                ver_cache[i + adder][j] = 1;
                cnt++;
                adder++;
            }
            if (cnt >= 4)
                num_of_rows += 2 * mod;
        }
    }
    int cache[HEIGHT][WIDTH] = {0};
    // / diagonal -- treated like an array
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            // this was part of another in-a-row
            if (cache[i][j])
                continue;
            unsigned int cnt = 0;
            // this keeps track as we trace diagnal
            int adder = 0;
            while (get_bit(board, i + adder, j + adder) && j + adder < WIDTH && i + adder < HEIGHT)
            {
                cache[i + adder][j + adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt < 1)
                continue;
            int mod = cnt;
            while (!get_bit(opp_board, i + adder, j + adder) && j + adder < WIDTH && i + adder < HEIGHT)
            {
                if (cache[i + adder][j + adder])
                    break; // this is a square another 4-in-a-row uses
                cache[i + adder][j + adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt >= 4)
                num_of_rows += mod * 2;
        }
    }
    int cache2[HEIGHT][WIDTH] = {0};
    // \ diagonal -- treated like an array
    for (int i = HEIGHT - 1; i >= 0; i--)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            // this was part of another in-a-row
            if (cache2[i][j])
                continue;
            unsigned int cnt = 0;
            // this keeps track as we trace diagnal
            int adder = 0;
            while (get_bit(board, i - adder, j + adder) && j + adder < WIDTH && i - adder >= 0)
            {
                cache2[i - adder][j + adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt < 1)
                continue;
            int mod = cnt;
            while (!get_bit(opp_board, i - adder, j + adder) && j + adder < WIDTH && i - adder >= 0)
            {
                if (cache2[i - adder][j + adder])
                    break; // this is a square another 4-in-a-row uses
                cache2[i - adder][j + adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt >= 4)
                num_of_rows += 2 * mod;
        }
    }
    return num_of_rows;
}

bool can_place(uint64_t board_r, uint64_t board_y, int col)
{
    uint64_t both_board = board_r | board_y;
    // #bit we want is 8*col+6
    // if the top bit is free, we can place.
    return !((1ULL << (8 * 5 + col)) & both_board);
}

// determines what height the token gets placed at
// figures out height of column
int get_height(uint64_t board, int col)
{
    unsigned int cnt = 0;
    // all the verti bits flipped
    uint64_t y = (board >> col) & 1103823438081ULL;
    // clear all the set bits (=height next piece should be placed at)
    for (cnt = 0; y; cnt++)
    {
        y &= y - 1; // clear the least significant bit set
    }
    return cnt;
}

// placing token == flipping a bit!
// we want to return a temp board for the player for the DFS
uint64_t place(uint64_t board, int col, int height)
{
    return flip_bit(board, (col * 8) + height);
}

// score for a player's board
// better idea: weight possible 4-in-a-rows more heavily
int score(uint64_t board, uint64_t opp_board)
{
    return num_in_a_row(board, opp_board);
}

int evaluation(uint64_t board_r, uint64_t board_y)
{
    int val = score(board_r, board_y) - score(board_y, board_r);
    return val;
}

// definitions here since these functions recursively call each other
int max_value_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta);
int min_value_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta);

bool isWin = false;

int max_value_init_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta)
{
    if (is_win(board_r) || is_win(board_y))
        return utility(board_r, board_y);
    int v = INT32_MIN;
    int col = 0;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothing (FOR THE TOURNAMENT)
    for (int j = 0; j < WIDTH; j++)
    {
        int i = col_search_order[j];
        if (can_place(board_r, board_y, i))
        {
            int height = get_height(board_r | board_y, i);
            uint64_t new_board = flip_bit(board_r, (height * 8) + i);
            int min_val = min_value_alpha(new_board, board_y, depth + 1, max_depth, alpha, beta);
            if (min_val > v)
            {
                v = min_val;
                col = i;
            }
            // alpha beta: parent will be min, if v is larger than bound, we will never use rest of branches.
            if (v > alpha)
                alpha = v;
            if (beta <= alpha)
                break;
        }
    }
    // doing nothing move
    if (beta > alpha)
    {
        int min_val = min_value_alpha(board_r, board_y, depth + 1, max_depth, alpha, beta);
        if (min_val > v)
        {
            v = min_val;
            col = -1;
        }
    }
    // set is win to true if we have found a move we can win from
    if (v == 100000)
        isWin = true;
    return col;
}

int min_value_init_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta)
{
    if (is_win(board_r) || is_win(board_y))
        return utility(board_r, board_y);
    int v = INT32_MAX;
    int col = 0;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothin g(FOR THE TOURNAMENT)
    for (int j = 0; j < WIDTH; j++)
    {
        int i = col_search_order[j];
        if (can_place(board_r, board_y, i))
        {
            int height = get_height(board_r | board_y, i);
            uint64_t new_board = place(board_y, height, i);
            int max_val = max_value_alpha(board_r, new_board, depth + 1, max_depth, alpha, beta);
            if (max_val < v)
            {
                v = max_val;
                col = i;
            }
            if (v < beta)
                beta = v;
            if (beta <= alpha)
                break;
        }
    }
    // doing nothing move
    if (beta > alpha)
    {
        int max_val = max_value_alpha(board_r, board_y, depth + 1, max_depth, alpha, beta);
        if (max_val < v)
        {
            v = max_val;
            col = -1;
        }
    }
    // set is win to true if we have found a move we can win from
    if (v == -100000)
        isWin = true;
    return col;
}

int max_value_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta)
{
    // check if we have it cached
    if (is_win(board_r) || is_win(board_y))
        return utility(board_r, board_y);
    if (depth >= max_depth)
        return evaluation(board_r, board_y);
    int v = INT32_MIN;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothing (FOR THE TOURNAMENT)
    for (int j = 0; j < WIDTH; j++)
    {
        int i = col_search_order[j];
        if (can_place(board_r, board_y, i))
        {
            int height = get_height(board_r | board_y, i);
            uint64_t new_board = place(board_r, height, i);
            int min_val = min_value_alpha(new_board, board_y, depth + 1, max_depth, alpha, beta);
            v = (min_val > v) ? min_val : v;
            if (v > alpha)
                alpha = v;
            if (beta <= alpha)
                break;
        }
    }
    // doing nothing move
    if (beta > alpha)
    {
        int min_val = min_value_alpha(board_r, board_y, depth + 1, max_depth, alpha, beta);
        if (min_val > v)
        {
            v = min_val;
        }
    }
    return v;
}

int min_value_alpha(uint64_t board_r, uint64_t board_y, int depth, int max_depth, int alpha, int beta)
{
    if (is_win(board_r) || is_win(board_y))
        return utility(board_r, board_y);
    if (depth >= max_depth)
        return evaluation(board_r, board_y);
    int v = INT32_MAX;
    int bound = INT32_MAX;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothing (FOR THE TOURNAMENT)
    for (int j = 0; j < WIDTH; j++)
    {
        int i = col_search_order[j];
        if (can_place(board_r, board_y, i))
        {
            int height = get_height(board_r | board_y, i);
            uint64_t new_board = place(board_y, height, i);
            int max_val = max_value_alpha(board_r, new_board, depth + 1, max_depth, alpha, beta);
            v = (max_val < v) ? max_val : v;
            if (v < beta)
                beta = v;
            if (beta <= alpha && use_alpha_beta)
                break;
        }
    }
    // doing nothing move
    if (beta > alpha)
    {
        int max_val = max_value_alpha(board_r, board_y, depth + 1, max_depth, alpha, beta);
        if (max_val < v)
        {
            v = max_val;
        }
    }
    return v;
}

int minimax_decision(uint64_t board_r, uint64_t board_y, bool is_red, int max_depth)
{
    int col;
    if (is_red)
    {
        // R = MAX
        col = max_value_init_alpha(board_r, board_y, 0, max_depth, INT32_MIN, INT32_MAX);
    }
    else
    {
        // Y = MIN
        col = min_value_init_alpha(board_r, board_y, 0, max_depth, INT32_MIN, INT32_MAX);
    }
    return col;
}

// when in doubt, print 3.
int decision = 3;

void print_best(int sig)
{
    if (decision != -1)
        std::cout << decision << std::endl
                  << std::flush;
    exit(0);
}

void runGame(uint64_t board_r, uint64_t board_y, bool is_red)
{
    int max_depth = 20;
    isWin = 0;
    for (int i = 1; i <= max_depth; i++)
    {
        decision = minimax_decision(board_r, board_y, is_red, i);
        if (isWin)
            break;
    }
    print_best(0);
}

int main(int argc, char **argv)
{
    int h = 0;
    int w = 0;
    for (int i = 0; i < HEIGHT * WIDTH + 5; i++)
    {
        switch (argv[1][i])
        {
        case '.':
            w++;
            break;
        case 'y':
            // wonky position for our bitboard
            board_y = flip_bit(board_y, (h * 8) + w);
            w++;
            break;
        case 'r':
            board_r = flip_bit(board_r, (h * 8) + w);
            w++;
            break;
        case ',':
            h++;
            w = 0;
            break;
        default:
            std::cout << argv[1][i] << "Something has gone terribly wrong..." << std::endl;
        }
    }
    bool is_red = (argv[2][0] == 'r');
    // The input is now mine!! MWUHAHAHAHA
    // run the AI
    pid_t pid = fork();
    // child process
    if (pid == 0)
    {
        signal(SIGTERM, (void (*)(int))print_best);
        runGame(board_r, board_y, is_red);
    }
    else
    {
        usleep(950000);
        int status;
        if (waitpid(pid, &status, WNOHANG) != pid)
            kill(pid, SIGTERM);
    }
    return 0;
}
