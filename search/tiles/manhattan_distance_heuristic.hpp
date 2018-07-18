#ifndef MANHATTAN_DISTANCE_HEURISTIC_HPP
#define MANHATTAN_DISTANCE_HEURISTIC_HPP

#include "tiles.hpp"
#include "tile_dimensions.hpp"
#include <array>


namespace Tiles {

    // calculate row index from index in grid of given width
    int getRowIdx(int index, int height);

    // calculate row index from index in grid of given height
    int getColIdx(int index, int width);

    // calculate Manhattan distance between two indexes in a grid of dimensions
    // width and height
    int manhattanDistance(char index1, char index2, int width, int height);

    class ManhattanDistanceHeuristic {
    private:
        // 2-D array for calculating each tile's manhattan distance
        std::array< std::array<int, N_TILES>, N_TILES> table;
    public:
        ManhattanDistanceHeuristic();
        
        int getH(Board const & board);
    };
}

#endif
