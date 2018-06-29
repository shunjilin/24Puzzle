#include "tiles.hpp"
#include <array>

namespace Tiles {

    // fixed goal state (Richard Korf format)
    auto Board::goal_tiles = std::array<char, N_TILES>
        ({0 , 1 , 2 , 3 , 4 ,
          5 , 6 , 7 , 8 , 9 ,
          10, 11, 12, 13, 14,
          15, 16, 17, 18, 19,
          20, 21, 22, 23, 24});

    Board::Board(const std::array<char, N_TILES>& tiles) :
        tiles(tiles) {}

    // TODO: pretty print
    std::ostream &operator<<(std::ostream& os, const Board& board) {
        for (auto tile : board.tiles) {
            os << std::to_string(tile) << " ";
        }
        return os;
    }

    // free functions to manipulate the board

    // get index of current blank tile
    char getBlankIdx(const Board& board) {
        auto blank_iter = std::find(board.tiles.begin(), board.tiles.end(), 0);
        return std::distance(board.tiles.begin(), blank_iter);
    }

    // get possible new blank indexes
    std::array<char, static_cast<int>(DIRECTION::COUNT)>
    getNewBlankIdxs(const Board& board) {
        auto blank_idx = board.cur_blank_idx == -1 ?
            getBlankIdx(board) : board.cur_blank_idx;

        // -1 for invalid move
        std::array<char, DIRECTION::COUNT>
            new_blank_idxs = {-1, -1, -1, -1};

        if (blank_idx >= WIDTH) {
            new_blank_idxs[DIRECTION::UP] = blank_idx - WIDTH;
        }
        if (blank_idx < (WIDTH * (HEIGHT - 1))) {
            new_blank_idxs[DIRECTION::DOWN] = blank_idx + WIDTH;
        }
        if ((blank_idx % WIDTH) != 0) {
            new_blank_idxs[DIRECTION::LEFT] = blank_idx - 1;
        }
        if ((blank_idx % WIDTH) != (WIDTH - 1)) {
            new_blank_idxs[DIRECTION::RIGHT] = blank_idx + 1;
        }
        
        return new_blank_idxs;
    }

    // swap blank tile with new blank tile to get new board configuration
    Board getNewBoard(const Board& board, char new_blank_idx) {
        auto cur_blank_idx = board.cur_blank_idx == -1 ?
            getBlankIdx(board) : board.cur_blank_idx;
        auto new_board = board;
        // move blank tile
        auto tile_to_swap = board.tiles[new_blank_idx];
        new_board.tiles[cur_blank_idx] = tile_to_swap;
        new_board.tiles[new_blank_idx] = 0;
        // cache blank idxs
        new_board.prev_blank_idx = cur_blank_idx;
        new_board.cur_blank_idx = new_blank_idx;
        return new_board;
    }
}