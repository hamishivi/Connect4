#include <iostream>
#include <stdlib.h> 
#include <limits>
#include <cstdint>
 
#define WIDTH 7
#define HEIGHT 6
 
std::uint64_t board_y = 0;
std::uint64_t board_r = 0;
bool is_red; 
bool use_alpha_beta = true;
int depth = 1000;
int nodes_visited;
 
// basic concept: shift by one, and to clear board,
// shift by two and then & to remove all but a possible 4-match
// only two shifts, two &s per direction!
// Inspired by http://eprints.qut.edu.au/85005/1/__staffhome.qut.edu.au_staffgroupm$_meaton_Desktop_bits-7.pdf
bool is_win(std::uint64_t board) {
    // horizontal
    std::uint64_t y = board & (board >> 1);
    if (y & (y >> 2)) return true;
    // horizontal
    y = board & (board >> 8);
    if (y & (y >> 2*8)) return true;
    // / diagonal
    y = board & (board >> 9);
    if (y & (y >> 2*9)) return true;
    // \ diagonal
    y = board & (board >> 7);
    if (y & y >> 2*7) return true;
    return false;
}
 
int utility(std::uint64_t board_r, std::uint64_t board_y) {
    if (is_win(board_r)) return 10000;
    if (is_win(board_y)) return -10000;
    return 0;
}
 
std::uint64_t flip_bit(std::uint64_t board, int n_bit) {
    board |= 1ULL << n_bit;
    return board;
}
 
int get_bit(std::uint64_t board, int i, int j) {
    return !!((1ULL << ((i*8)+j)) & board);
}
 
// From https://graphics.stanford.edu/~seander/bithacks.html
int number_of_set_bits(std::uint64_t board)
{
    unsigned int cnt;
    for (cnt = 0; board; cnt++)
    {
        board &= board - 1; // clear the least significant bit set
    }
    return cnt;
}
 
// counts number of rows that have 'count' bits set in a row
// this can be diag OR horizontal
int num_in_a_row(int count, std::uint64_t board) {
    unsigned int num_of_rows = 0;
    // if 4 look at larger vals too.
    if (count == 4) {
        num_of_rows += num_in_a_row(5, board);
        num_of_rows += num_in_a_row(6, board);
        // max in a row you can physically get is 7 horizontal.
        num_of_rows += num_in_a_row(7, board);
    }
    // Horizontal
     int hoz_cache[HEIGHT][WIDTH] = { 0 };
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (hoz_cache[i][j]) continue;
            unsigned int cnt = 0;
            int adder = 0;
            while (get_bit(board, i, j+adder) && j+adder < WIDTH) {
                hoz_cache[i][j+adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt == count) num_of_rows++;
        }
    }
    // Vertical
    int ver_cache[HEIGHT][WIDTH] = { 0 };
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (ver_cache[i][j]) continue;
            unsigned int cnt = 0;
            int adder = 0;
            while (get_bit(board, i+adder, j) && i+adder < HEIGHT) {
                ver_cache[i+adder][j] = 1;
                cnt++;
                adder++;
            }
            if (cnt == count) num_of_rows++;
        }
    }
    // 7 not possible in diags
    if (count > 6) return num_of_rows;
    int cache[HEIGHT][WIDTH] = { 0 };
    // / diagonal -- treated like an array
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // this was part of another in-a-row
            if (cache[i][j]) continue;
            unsigned int cnt = 0;
            // this keeps track as we trace diagnal
            int adder = 0;
            while (get_bit(board, i+adder, j+adder) && j+adder < WIDTH && i+adder < HEIGHT) {
                cache[i+adder][j+adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt == count) num_of_rows++;
        }
    }
    int cache2[HEIGHT][WIDTH] = { 0 };
    // \ diagonal -- treated like an array
    for (int i = HEIGHT-1; i >= 0; i--) {
        for (int j = 0; j < WIDTH; j++) {
            // this was part of another in-a-row
            if (cache2[i][j]) continue;
            unsigned int cnt = 0;
            // this keeps track as we trace diagnal
            int adder = 0;
            while (get_bit(board, i-adder, j+adder)  && j+adder < WIDTH && i-adder >= 0) {
                cache2[i-adder][j+adder] = 1;
                cnt++;
                adder++;
            }
            if (cnt == count) num_of_rows++;
        }
    }
    return num_of_rows;
}
 
bool can_place(std::uint64_t board_r, std::uint64_t board_y, int col) {
    std::uint64_t both_board = board_r | board_y;
    // #bit we want is 8*col+6
    // if the top bit is free, we can place.
    return !((1ULL << (8*5+col)) & both_board);
}
 
// determines what height the token gets placed at
// figures out height of column
int get_height(std::uint64_t board, int col) {
    unsigned int cnt = 0;
    // all the verti bits flipped
    std::int64_t y = (board >> col) & 1103823438081ULL;
    // clear all the set bits (=height next piece should be placed at)
    for (cnt = 0; y; cnt++)
    {
        y &= y - 1; // clear the least significant bit set
    }
    return cnt;
}
 
// placing token == flipping a bit!
// we want to return a temp board for the player for the DFS
std::uint64_t place(std::uint64_t board, int col, int height) {
    return flip_bit(board, (col*8)+height);
}
 
// score for a player's board
int score(std::uint64_t board) {
    return number_of_set_bits(board) +
          10 * num_in_a_row(2, board) +
          100 * num_in_a_row(3, board) +
          1000 * num_in_a_row(4, board);
}
 
int evaluation(std::uint64_t board_r, std::uint64_t board_y) {
    int val = score(board_r) - score(board_y);
    return val;
}
 
// definitions here since these functions recursively call each other
int max_value_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta);
int min_value_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta);
 
int max_value_init_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta) {
    nodes_visited++;
    if (is_win(board_r) || is_win(board_y)) return utility(board_r, board_y);
    if (depth == max_depth) return evaluation(board_r, board_y);
    int v = INT32_MIN;
    int col = 0;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothing (FOR THE TOURNAMENT)
    for (int i = 0; i < WIDTH; i++) {
        if (can_place(board_r, board_y, i)) {
            int height = get_height(board_r | board_y, i);
            std::uint64_t new_board = flip_bit(board_r, (height*8)+i);
            int min_val = min_value_alpha(new_board, board_y, depth+1, max_depth, alpha, beta, use_alpha_beta);
            if (min_val > v) {
                v = min_val;
                col = i;
            }
            // alpha beta: parent will be min, if v is larger than bound, we will never use rest of branches.
            if (v > alpha) alpha = v;
            if (beta <= alpha && use_alpha_beta) break;
        }
    }
    return col;
}
 
int min_value_init_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta) {
    nodes_visited++;
    if (is_win(board_r) || is_win(board_y)) return utility(board_r, board_y);
    if (depth == max_depth) return evaluation(board_r, board_y);
    int v = INT32_MAX;
    int col = 0;
    // successors = placing a token into a col, if we can
    // one successor should be placing nothin g(FOR THE TOURNAMENT)
    for (int i = 0; i < WIDTH; i++) {
        if (can_place(board_r, board_y, i)) {
            int height = get_height(board_r | board_y, i);
            std::uint64_t new_board = place(board_y, height, i);
            int max_val = max_value_alpha(board_r, new_board, depth+1, max_depth, alpha, beta, use_alpha_beta);
            if (max_val < v) {
                v = max_val;
                col = i;
            }
            if (v < beta) beta = v;
            if (beta <= alpha && use_alpha_beta) break;
        }
    }
    return col;
}
 
int max_value_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta) {
    nodes_visited++;
   
    if (is_win(board_r) || is_win(board_y)) return utility(board_r, board_y);
    if (depth == max_depth) return evaluation(board_r, board_y);
    int v = INT32_MIN;
    // successors = placing a token into a col, if we can
    for (int i = 0; i < WIDTH; i++) {
        if (can_place(board_r, board_y, i)) {
            int height = get_height(board_r | board_y, i);
            std::uint64_t new_board = place(board_r, height,  i);
            int min_val = min_value_alpha(new_board, board_y, depth+1, max_depth, alpha, beta, use_alpha_beta);
            v = (min_val > v) ? min_val : v;
            if (v > alpha) alpha = v;
            if (beta <= alpha && use_alpha_beta) break;
        }
    }
    return v;
}
 
int min_value_alpha(std::uint64_t board_r, std::uint64_t board_y, int depth, int max_depth, int alpha, int beta, bool use_alpha_beta) {
    nodes_visited++;
    if (is_win(board_r) || is_win(board_y)) return utility(board_r, board_y);
    if (depth == max_depth) return evaluation(board_r, board_y);
    int v = INT32_MAX;
    int bound = INT32_MAX;
    // successors = placing a token into a col, if we can
    for (int i = 0; i < WIDTH; i++) {
        if (can_place(board_r, board_y, i)) {
            int height = get_height(board_r | board_y, i);
            std::uint64_t new_board = place(board_y, height, i);
            int max_val = max_value_alpha(board_r, new_board, depth+1, max_depth, alpha, beta, use_alpha_beta);
            v = (max_val < v) ? max_val : v;
            if (v < beta) beta = v;
            if (beta <= alpha && use_alpha_beta) break;
        }
    }
    return v;
}
 
int minimax_decision(std::uint64_t board_r, std::uint64_t board_y, bool is_red, int max_depth, bool use_alpha_beta) {
    int col;
    if (is_red) {
        // R = MAX
        col = max_value_init_alpha(board_r, board_y, 0, max_depth, INT32_MIN, INT32_MAX, use_alpha_beta);
    } else {
        // Y = MIN
        col = min_value_init_alpha(board_r, board_y, 0, max_depth, INT32_MIN, INT32_MAX, use_alpha_beta);
    }
    return col;
}
 
int main(int argc, char **argv) {
    int h = 0;
    int w = 0;
    for (int i = 0; i < HEIGHT*WIDTH+5; i++) {
        switch(argv[1][i]) {
                case '.':
                    w++;
                    break;
                case 'y':
                    // wonky position for our bitboard
                    board_y = flip_bit(board_y, (h*8)+w);
                    w++;
                    break;
                case 'r':
                    board_r = flip_bit(board_r, (h*8)+w);
                    w++;
                    break;
                case ',':
                    h++;
                    w = 0;
                    break;
                default:
                    break;
            }
    }
    is_red = (argv[2][0] == 'r');
    use_alpha_beta = (argv[3][0] == 'A');
    depth = atoi(argv[4]);
    // run the AI
    int decision = 0;
    nodes_visited = 0;
    decision = minimax_decision(board_r, board_y, is_red, depth, use_alpha_beta);
    std::cout << decision << std::endl;
    std::cout << nodes_visited << std::endl;
    
    return 0;
}
 
